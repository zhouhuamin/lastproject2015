/* 
* File:   ProcessImage.cpp
* Author: root
* 
* Created on 2015年1月28日, 上午10:45
*/
#include <syslog.h>
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <vector>
#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "ProcessImage.h"
#include "JieziBoxStruct.h"
#include "ace/Log_Priority.h"
#include "ace/Log_Msg.h"
#include "BoxNumberCheckAlgo.h"
#include "SysMessage.h"
#include "MSGHandleCenter.h"
#include "SimpleConfig.h"
#include "MyLog.h"
#include "ace/SOCK_Connector.h"
#include "ace/Signal.h"
#include "jzLocker.h"

using namespace boost::uuids;
using namespace std;
using namespace boost;
int GlobalStopFlag = 0;
int	g_nHoppingCount = 0;
vector<pair<BYTE, structUploadData> > g_statusVect;
std::vector<BYTE>			g_loseSeqNoVect;
std::vector<char> g_status;
sem	g_sem;

#define MAX_HOST_DATA 4096

extern pthread_mutex_t              g_ImageDataMutex;
extern pthread_cond_t               g_ImageDataCond;

extern struct	struDeviceAndServiceStatus g_ObjectStatus;
extern pthread_mutex_t					 g_StatusMutex;

using namespace std;
void ReceiveReaderData(unsigned char* read_data, void* user_data) 
{
	return;
}
void PublishEventData(unsigned char* read_data, void* user_data) 
{
	JZ_PUBLISH_EVENT_STRUCT *pUserData = (JZ_PUBLISH_EVENT_STRUCT *)user_data;
	if (pUserData == NULL)
		return;
	NET_PACKET_MSG* pMsg_ = NULL;
	CMSG_Handle_Center::get_message(pMsg_);
	if (!pMsg_) 
	{
		return;
	}
	memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
	pMsg_->msg_head.msg_type 		= SYS_MSG_PUBLISH_EVENT;
	pMsg_->msg_head.packet_len 		= sizeof (T_SysEventData);
	T_SysEventData* pReadData 		= (T_SysEventData*) pMsg_->msg_body;
	strcpy(pReadData->event_id,  pUserData->event_id);
	strcpy(pReadData->device_tag, pUserData->device_tag);
	pReadData->is_data_full			= 0; // 1;
	// char szXMLGatherInfo[10 * 1024] = {0};
	//sprintf(szXMLGatherInfo,);  
	// printf("%s\n", szXMLGatherInfo);
	// strcpy(pReadData->xml_data, szXMLGatherInfo);
	pReadData->xml_data_len = 0; // strlen(szXMLGatherInfo) + 1;
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData); //  + pReadData->xml_data_len;
	syslog(LOG_DEBUG, "pack len :%d\n", pMsg_->msg_head.packet_len);

	MSG_HANDLE_CENTER::instance()->put(pMsg_);
	return;
}

ProcessImage::ProcessImage()
{
	syslog(LOG_DEBUG, "Enter ProcessImage Constructor!\n");
}

ProcessImage::ProcessImage(const ProcessImage& orig)
{
}

ProcessImage::~ProcessImage()
{
}

void ProcessImage::Init()
{
	//SetReadDataCallback((_READ_DATA_CALLBACK_) ReceiveReaderData, this);
	return;
}

void ProcessImage::Run()
{
	pthread_create (&controllerThreadID, NULL, ProcessControllerThread, NULL);
	//pthread_create (&statusTableThreadID, NULL, ProcessStatusTableThread, NULL);
	pthread_create (&boxNumberThreadID, NULL, ProcessDetectorThread, NULL);   
	//pthread_create (&statusTableThreadIDV2, NULL, ProcessStatusTableThreadV2, NULL);
	pthread_create (&statusTableThreadIDV2, NULL, ProcessStatusTableThreadV3, NULL);
	return;
}

void *ProcessImage::ProcessControllerThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessControllerProc();
	return 0; 
}

void *ProcessImage::ProcessStatusTableThread(void *pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessStatusTableProc();
	return 0;
}

void *ProcessImage::ProcessDetectorThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessDetectorProc();
};

void *ProcessImage::ProcessStatusTableThreadV2(void *pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessStatusTableProcV2();
	return 0;
}

void *ProcessImage::ProcessStatusTableThreadV3(void *pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessStatusTableProcV3();
	return 0;
}

void ProcessImage::ProcessControllerProc()
{
	//printf("Enter ProcessControllerProc\n");
	//	客户端处理线程,把接收的数据发送给目标机器,并把目标机器返回的数据返回到客户端
	int nLen=0;
	int nLoopTotal=0;
	int nLoopMax=20*300;	//	300 秒判断选循环
#define	nMaxLen 256		// 0x1000
	char pBuffer[nMaxLen+1];
	char pNewBuffer[nMaxLen+1];
	memset(pBuffer,0,nMaxLen);
	memset(pNewBuffer,0,nMaxLen);
	int nSocketErrorFlag=0;
	int nNewLen=0;
	RTUProtocol rtu;
	while (!GlobalStopFlag)
	{
		int nNewSocket = CreateSocket();
		if (nNewSocket == -1)
		{
			//	不能建立套接字，直接返回
			syslog(LOG_DEBUG, "Can't create socket\n");
			sleep(10);
			nNewSocket = CreateSocket();
			if(nNewSocket == -1)
			{
				continue;
			}		
		}
		//SetSocketNotBlock(nNewSocket);
		//if (ConnectSocket(nNewSocket, GlobalRemoteHost, GlobalRemotePort) <= 0)
		if (ConnectSocket(nNewSocket, CSimpleConfig::m_strGatherControlIP.c_str(), CSimpleConfig::m_nGatherControlPort) <= 0)
		{
			pthread_mutex_lock(&g_StatusMutex);
			g_ObjectStatus.szLocalIP				= CSimpleConfig::m_strChannelControlIP;
			g_ObjectStatus.nLocalPort				= 0;
			g_ObjectStatus.szUpstreamIP				= CSimpleConfig::m_strChannelControlIP;
			g_ObjectStatus.nUpstreamPort			= CSimpleConfig::CHANNEL_CONTROLER_PORT;
			g_ObjectStatus.szUpstreamObjectCode		= "APP_CHANNEL_001";
			g_ObjectStatus.nServiceType				= 0;
			g_ObjectStatus.szObjectCode				= "DEV_DETECTOR_001";
			g_ObjectStatus.szObjectStatus			= "021";
			g_ObjectStatus.szConnectedObjectCode	= "COMM_CONTROL_001";

			g_ObjectStatus.szReportTime = GetCurTime();

			pthread_mutex_unlock(&g_StatusMutex);
			//	不能建立连接，直接返回
			//CloseSocket(nSocket);
			syslog(LOG_DEBUG, "Can't connect host\n");
			CloseSocket(nNewSocket);
			sleep(3);
			continue;
			//EndClient(pParam);
		}

		set_keep_live(nNewSocket, 3, 30);

		//ConnectSocket(nNewSocket, "192.168.1.188", 502);
		//struct timeval timeout ;
		//fd_set sets;
		//FD_ZERO(&sets);
		//FD_SET(nNewSocket, &sets);
		//timeout.tv_sec = 15; //连接超时15秒
		//timeout.tv_usec =0;
		//int result = select(0, 0, &sets, 0, &timeout);
		//if (result <=0)
		//{
		//	CloseSocket(nNewSocket);
		//	printf("Enter select\n");
		//	sleep(10);
		//	continue;
		//}
		vector<BYTE> dataVect;
		rtu.WriteRegisterRequest(dataVect);
		memcpy(pBuffer, &dataVect[0], dataVect.size());
		nLen = dataVect.size();
		nNewLen = SocketWrite(nNewSocket,pBuffer,nLen,30);
		syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen);
		//printf("%s Send Data:", GetCurTime().c_str());
		char szTmpData[5 + 1] = {0};
		string strLogData = "Send Data:";
		for (int i = 0; i < nNewLen; ++i)
		{
			memset(szTmpData, 0x00, 5);
			sprintf(szTmpData, "%02X ", dataVect[i]);
			strLogData += szTmpData;
		}
		syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
		//SetSocketNotBlock(nNewSocket);
		while(!GlobalStopFlag && !nSocketErrorFlag)
		{
			nLoopTotal++;
			nLen=SocketRead(nNewSocket, pNewBuffer, nMaxLen);	
			//	读取客户端数据
			syslog(LOG_DEBUG, "recv  nLen=%d\n", nLen);
			if(nLen>0)
			{
				strLogData = "Recv Data:";
				for (size_t i = 0; i < nLen; ++i)
				{
					memset(szTmpData, 0x00, 5);
					sprintf(szTmpData, "%02X ", (BYTE)pNewBuffer[i]);
					strLogData += szTmpData;
				}
				syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
				pNewBuffer[nLen]=0;
				nLoopTotal=0;
				int nProcessed = 0;
				int nLenLeft = nLen;
				while (nLenLeft > 0)
				{
					// 进行判断,触发处理流程
					if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x53)
					{
						int nStep = 10;
						vector<BYTE> upDataVect((BYTE*)pNewBuffer, (BYTE*)pNewBuffer + nStep);
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						
						std::pair<BYTE, structUploadData> pairData;
						//read(8);
						bool bRet = rtu.ReadUploadData(upDataVect, pairData);
						//printf("ReadUploadData:ret-%d\n", bRet);
						// save local
						pthread_mutex_lock (&g_ImageDataMutex); 
						bool bFlag = false;
						for (size_t i = 0; i < g_statusVect.size(); ++i)
						{
							if (g_statusVect[i].first == pairData.first)
							{
								bFlag = true;
								break;
							}
						}
						if (!bFlag)
							g_statusVect.push_back(pairData);
						pthread_mutex_unlock (&g_ImageDataMutex);
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x65)
					{
						//read(1 + 2);
						int nStep = 5;
						memcpy(pBuffer, pNewBuffer, nStep);						
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						int nNewLen2 = SocketWrite(nNewSocket, pBuffer, nStep, 30);
						syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
						strLogData = "Send Data:";
						for (int i = 0; i < nNewLen2; ++i)
						{
							memset(szTmpData, 0x00, 5);
							sprintf(szTmpData, "%02X ", (BYTE)pBuffer[i]);
							strLogData += szTmpData;
						}
						syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x02)
					{
						//read(1 + 2);
						int nStep = 6;
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x01)
					{
						//read(1 + 2);
						int nStep = 6;
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x0F)
					{
						//read(1 + 2);
						int nStep = 8;
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x10)
					{
						int nStep = 8;
						nLenLeft -= nStep;
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						//read(4 + 2);
						bool bRet = false;
						bRet = rtu.WriteRegisterResponse(dataVect);
						if (bRet)
						{
							pthread_mutex_lock (&g_ImageDataMutex);   
							g_statusVect.clear();
							pthread_mutex_unlock (&g_ImageDataMutex);
						}
					}
					else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x03)
					{
						//Length = read(1);
						//read(Length + 2);
						int nStep = 0;
						BYTE Length = (BYTE)pNewBuffer[2];
						nStep = Length + 1 + 4;
						nLenLeft -= nStep;
						vector<BYTE> readDataVect((BYTE*)pNewBuffer, (BYTE*)pNewBuffer + nStep);
						if (nLenLeft > 0)
							memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						
						rtu.ReadRegisterResponse(readDataVect, Length);
					}
					else
					{
						syslog(LOG_DEBUG, "%s head[0] head[1]: 0x%02X, 0x%02X\n", GetCurTime().c_str(), (BYTE)pNewBuffer[0], (BYTE)pNewBuffer[1]);
						nLenLeft	= 0;
					}
					//pthread_cond_signal (&g_ImageDataCond);
					// read history
					//if (g_loseSeqNoVect.size() > 0)
					//{
					//	vector<BYTE> readDataVect;
					//	rtu.ReadRegisterRequest(readDataVect, g_loseSeqNoVect[0]);
					//	memcpy(nNewSocket, &readDataVect[0], readDataVect.size());
					//	nLen	= readDataVect.size();
					//	nNewLen=SocketWrite(nNewSocket,pNewBuffer,nLen,30);
					//	if(nNewLen<0)	//	断开
					//	{
					//		CloseSocket(nNewSocket);
					//		g_loseSeqNoVect.clear();
					//		break;
					//	}
					//	g_loseSeqNoVect.erase(g_loseSeqNoVect.begin());
					//	
					//}
				}
			}
			if(nLen<0)
			{
				//	读断开
				CloseSocket(nNewSocket);
				break;
			}
			//nNewLen=SocketRead(nNewSocket,pNewBuffer,nMaxLen);	
			////	读取返回数据
			//if(nNewLen>0)
			//{
			//	pNewBuffer[nNewLen]=0;
			//	//			WriteBaseLog(pNewBuffer);
			//	nLoopTotal=0;
			//	nLen=SocketWrite(nNewSocket,pNewBuffer,nNewLen,30);
			//	if(nLen<0)	//	断开
			//		break;
			//}
			//if(nNewLen<0)
			//{
			//	//	读断开
			//	break;
			//}
			if((nSocketErrorFlag==0)&&(nLoopTotal>0))
			{
				SysSleep(50);
				if(nLoopTotal>=nLoopMax)
				{
					nLoopTotal=0;
				}
			}
		}		
	}
	SysSleep(50);
	pthread_exit(NULL);
	return;    
}

void ProcessImage::ProcessStatusTableProc()
{
	int nStatusSize = 0;
	bool bStart = false;
	bool bEnd	= false;
	long lLastTime	= getCurrentTime();
	long lNowTime	= lLastTime;

	while (!GlobalStopFlag)
	{
		lNowTime	= getCurrentTime();
		if (g_statusVect.size() > 0)
		{
			std::vector<std::string>	g_statusTableVect;
			pthread_mutex_lock (&g_ImageDataMutex);
			std::pair<BYTE, structUploadData> &pairData = g_statusVect.front();
			pthread_mutex_unlock (&g_ImageDataMutex);
			BYTE ch			= pairData.second.Di_Last;
			string str		= Byte2String(ch);
			string inStr	= str;
			g_statusTableVect.push_back(inStr);
			//printf("inStr :%s\n", inStr.c_str());

			ch		= pairData.second.Di_Now;
			str		= Byte2String(ch);
			inStr	= str;

			pthread_mutex_lock (&g_ImageDataMutex);
			g_statusTableVect.push_back(inStr);
			pthread_mutex_unlock (&g_ImageDataMutex);
			//printf("inStr :%s\n", inStr.c_str());
			
			pthread_mutex_lock (&g_ImageDataMutex); 
			g_statusVect.erase(g_statusVect.begin());
			pthread_mutex_unlock (&g_ImageDataMutex);

			nStatusSize = g_statusTableVect.size();
			
			if (nStatusSize > 1)
			{
				// trigger
				syslog(LOG_DEBUG, "======================================================================================!\n");
				
				string str1 = g_statusTableVect[0];
				string str2 = g_statusTableVect[1];

				int nIndex = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (CSimpleConfig::DI_ENABLE[i] == '1')
					{
						char szTmp = CSimpleConfig::DI_BIT[i];
						int nTmp = (int)(szTmp - '0');
						nIndex = 8 - nTmp;
						break;
					}
				}
				
				//if (str1.size() > 2 && str2.size() > 2)
				if (str1.size() > nIndex && str2.size() > nIndex)
				{
					//if (!bStart && str1[2] == '0' && str2[2] == '1')
					//if (!bStart && str1[nIndex] == '0' && str2[nIndex] == '1')
					if (!bStart && str1[nIndex] == '1' && str2[nIndex] == '0')
					{
						syslog(LOG_DEBUG, "==========================Car Enter Channel\n");
						bStart	= true;
						lLastTime = lNowTime;
						pthread_mutex_lock (&g_ImageDataMutex); 						
						g_statusVect.clear();
						g_statusTableVect.clear();
						pthread_cond_signal (&g_ImageDataCond);
						pthread_mutex_unlock (&g_ImageDataMutex);
					}
					//else if (bStart && !bEnd && str1[2] == '1' && str2[2] == '0')
					//else if (bStart && !bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					else if (bStart && !bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					{
						lLastTime	= lNowTime;
						bEnd		= true;
					}
					//else if (bStart && bEnd && str1[2] == '1' && str2[2] == '0')
					//else if (bStart && bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					else if (bStart && bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					{
						lLastTime	= lNowTime;
					}
					//else if (bStart && bEnd && str1[2] == '0' && str2[2] == '1')
					//else if (bStart && bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					else if (bStart && bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					{
						bEnd = false;
						lLastTime	= lNowTime;
					}
				}
				else
				{
					//g_statusTableVect.erase(g_statusVect.begin(), g_statusVect.begin() + 2);
					g_statusTableVect.clear();
				}
			}
		}

		if (bStart && bEnd) // && lNowTime - lLastTime > 2)
		{
			bStart	= false;
			bEnd	= false;
			lLastTime	= lNowTime;

			// car leave
			syslog(LOG_DEBUG, "==========================Car Leave Detector!\n");

			// 
			JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
			for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
			{
				if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "14") == 0)
				{
					strcpy(pUserData->sequence_no, "14");
					pUserData->sequence_no[2]	= '\0';
					strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
					pUserData->device_tag[31]	= '\0';
					strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
					pUserData->event_id[31]	= '\0';
					break;
				}
			}
			PublishEventData(NULL, (void*)pUserData);
			delete pUserData;
			pUserData	= NULL;
		}

		SysSleep(200);
	}
	return;
}

void ProcessImage::ProcessStatusTableProcV2()
{
	int nStatusSize = 0;
	bool bStart = false;
	bool bEnd	= false;
	long lLastTime	= getCurrentTime();
	long lNowTime	= lLastTime;

	while (!GlobalStopFlag)
	{
		lNowTime	= getCurrentTime();

		if (bStart && !bEnd && lNowTime - lLastTime < 5)
		{
			pthread_mutex_lock (&g_ImageDataMutex); 						
			g_statusVect.clear();
			pthread_mutex_unlock (&g_ImageDataMutex);
		
			SysSleep(200);
			continue;
		}


		if (g_statusVect.size() > 0)
		{
			std::vector<std::string>	g_statusTableVect;
			pthread_mutex_lock (&g_ImageDataMutex);
			std::pair<BYTE, structUploadData> &pairData = g_statusVect.front();
			pthread_mutex_unlock (&g_ImageDataMutex);
			BYTE ch			= pairData.second.Di_Last;
			string str		= Byte2String(ch);
			string inStr	= str;
			g_statusTableVect.push_back(inStr);
			//printf("inStr :%s\n", inStr.c_str());

			ch		= pairData.second.Di_Now;
			str		= Byte2String(ch);
			inStr	= str;

			pthread_mutex_lock (&g_ImageDataMutex);
			g_statusTableVect.push_back(inStr);
			pthread_mutex_unlock (&g_ImageDataMutex);
			//printf("inStr :%s\n", inStr.c_str());

			pthread_mutex_lock (&g_ImageDataMutex); 
			g_statusVect.erase(g_statusVect.begin());
			pthread_mutex_unlock (&g_ImageDataMutex);

			nStatusSize = g_statusTableVect.size();

			if (nStatusSize > 1)
			{
				// trigger
				syslog(LOG_DEBUG, "======================================================================================!\n");

				string str1 = g_statusTableVect[0];
				string str2 = g_statusTableVect[1];

				int nIndex = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (CSimpleConfig::DI_ENABLE[i] == '1')
					{
						char szTmp = CSimpleConfig::DI_BIT[i];
						int nTmp = (int)(szTmp - '0');
						nIndex = 8 - nTmp;
						break;
					}
				}

				//if (str1.size() > 2 && str2.size() > 2)
				if (str1.size() > nIndex && str2.size() > nIndex)
				{
					//if (!bStart && str1[2] == '0' && str2[2] == '1')
					//if (!bStart && str1[nIndex] == '0' && str2[nIndex] == '1')
					if (!bStart && str1[nIndex] == '1' && str2[nIndex] == '0')
					{
						syslog(LOG_DEBUG, "==========================Car Enter Channel\n");
						bStart	= true;
						lLastTime = lNowTime;
						pthread_mutex_lock (&g_ImageDataMutex); 						
						g_statusVect.clear();
						g_statusTableVect.clear();
						pthread_cond_signal (&g_ImageDataCond);
						pthread_mutex_unlock (&g_ImageDataMutex);
					}
					//else if (bStart && !bEnd && str1[2] == '1' && str2[2] == '0')
					//else if (bStart && !bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					else if (bStart && !bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					{
						lLastTime	= lNowTime;
						bEnd		= true;
					}
					//else if (bStart && bEnd && str1[2] == '1' && str2[2] == '0')
					//else if (bStart && bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					else if (bStart && bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					{
						lLastTime	= lNowTime;
					}
					//else if (bStart && bEnd && str1[2] == '0' && str2[2] == '1')
					//else if (bStart && bEnd && str1[nIndex] == '0' && str2[nIndex] == '1')
					else if (bStart && bEnd && str1[nIndex] == '1' && str2[nIndex] == '0')
					{
						bEnd = false;
						lLastTime	= lNowTime;
					}
				}
				else
				{
					//g_statusTableVect.erase(g_statusVect.begin(), g_statusVect.begin() + 2);
					g_statusTableVect.clear();
				}
			}
		}

		if (bStart && bEnd) // && lNowTime - lLastTime > 2)
		{
			bStart	= false;
			bEnd	= false;
			lLastTime	= lNowTime;

			// car leave
			syslog(LOG_DEBUG, "==========================Car Leave Detector!\n");

			// 
			JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
			for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
			{
				if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "14") == 0)
				{
					strcpy(pUserData->sequence_no, "14");
					pUserData->sequence_no[2]	= '\0';
					strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
					pUserData->device_tag[31]	= '\0';
					strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
					pUserData->event_id[31]	= '\0';
					break;
				}
			}
			PublishEventData(NULL, (void*)pUserData);
			delete pUserData;
			pUserData	= NULL;
		}

		SysSleep(200);
	}
	return;
}

void ProcessImage::ProcessStatusTableProcV3()
{
	int nStatusSize = 0;
	long lLastTime	= getCurrentTime();
	long lNowTime	= lLastTime;
        static int nStarted = 0;

	while (!GlobalStopFlag)
	{
		lNowTime	= getCurrentTime();

		if (g_statusVect.size() > 0)
		{
			std::vector<std::string>	g_statusTableVect;
			pthread_mutex_lock (&g_ImageDataMutex);
			std::pair<BYTE, structUploadData> &pairData = g_statusVect.front();
			pthread_mutex_unlock (&g_ImageDataMutex);
			BYTE ch			= pairData.second.Di_Last;
			string str		= Byte2String(ch);
			string inStr	= str;
			g_statusTableVect.push_back(inStr);
			//printf("inStr :%s\n", inStr.c_str());

			ch		= pairData.second.Di_Now;
			str		= Byte2String(ch);
			inStr	= str;

			pthread_mutex_lock (&g_ImageDataMutex);
			g_statusTableVect.push_back(inStr);
			pthread_mutex_unlock (&g_ImageDataMutex);
			//printf("inStr :%s\n", inStr.c_str());

			pthread_mutex_lock (&g_ImageDataMutex); 
			g_statusVect.erase(g_statusVect.begin());
			pthread_mutex_unlock (&g_ImageDataMutex);

			nStatusSize = g_statusTableVect.size();

			if (nStatusSize > 1)
			{
				// trigger
				syslog(LOG_DEBUG, "======================================================================================!\n");

				string str1 = g_statusTableVect[0];
				string str2 = g_statusTableVect[1];

				int nIndex = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (CSimpleConfig::DI_ENABLE[i] == '1')
					{
						char szTmp = CSimpleConfig::DI_BIT[i];
						int nTmp = (int)(szTmp - '0');
						nIndex = 8 - nTmp;
						break;
					}
				}

				//if (str1.size() > 2 && str2.size() > 2)
				if (str1.size() > nIndex && str2.size() > nIndex)
				{
					char ch1	=	str1[nIndex];
					char ch2	=	str2[nIndex];

					if (ch1 == '1' && ch2 == '0')
					{
						pthread_mutex_lock(&g_ImageDataMutex);
						g_status.push_back(ch1);
						g_status.push_back(ch2);
                                                nStarted = 1;
						pthread_mutex_unlock(&g_ImageDataMutex);
						g_sem.post();		
					}
					else if (ch1 == '0' && ch2 == '1')
					{
                                            if (nStarted == 1)
                                            {
						pthread_mutex_lock(&g_ImageDataMutex);
                                                nStarted = 0;
						g_status.push_back(ch1);
						g_status.push_back(ch2);
						pthread_mutex_unlock(&g_ImageDataMutex);	
						g_sem.post();
                                            }
					}
					else if (ch1 == '0' && ch2 == '0')
					{
						pthread_mutex_lock(&g_ImageDataMutex);
						g_status.push_back('1');
						g_status.push_back('0');
                                                nStarted = 1;
						pthread_mutex_unlock(&g_ImageDataMutex);
						g_sem.post();	
					}
					else if (ch1 == '1' && ch2 == '1')
					{
						;
					}
					pthread_mutex_lock (&g_ImageDataMutex); 
					g_statusTableVect.clear();
					pthread_mutex_unlock (&g_ImageDataMutex);
				}
			}
		}

		SysSleep(200);
	}
	return;
}


void ProcessImage::ProcessDetectorProc()
{	
	bool bStarted = false;
	bool bEnded	= false;
	int nResult = 0;

	while (1)
	{
		nResult = g_sem.wait_timed(3);
		if (nResult == 0)
		{	
			pthread_mutex_lock(&g_ImageDataMutex);
			int nSize = g_status.size();
			for (int i = 0; i < nSize; ++i)
			{
				if (!bStarted)
				{
					if (g_status[i] == '1' && i + 1 < nSize && g_status[i + 1] == '0')
					{
                                       		//pthread_mutex_lock(&g_ImageDataMutex);
						
						//pthread_mutex_unlock(&g_ImageDataMutex);     
                                            
						syslog(LOG_DEBUG, "==========================Detector start!========================\n");
						bStarted	= true;
						JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
						for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
						{
							if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "13") == 0)
							{
								strcpy(pUserData->sequence_no, "13");
								pUserData->sequence_no[2]	= '\0';
								strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
								pUserData->device_tag[31]	= '\0';
								strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
								pUserData->event_id[31]	= '\0';
								break;
							}
						}
						PublishEventData(NULL, (void*)pUserData);
						delete pUserData;
						pUserData	= NULL;
						break;
					}				
				}
				else
				{
					break;
				}
			}

                            //g_status.clear();
                        
			pthread_mutex_unlock(&g_ImageDataMutex);
		}
		else
		{
			syslog(LOG_DEBUG, "timeout\n");

			pthread_mutex_lock(&g_ImageDataMutex);
			int nSize = g_status.size();
			for (int i = nSize - 1; i > 0; --i)
			{
				if (!bEnded)
				{
					if (g_status[i - 1] == '0' && g_status[i] == '1')
					{
						bEnded	= true;
						break;
					}
					else
					{
						break;	
					}
				}
				else
				{
					break;
				}
			}	
			pthread_mutex_unlock(&g_ImageDataMutex);			
		}

		if (bStarted && bEnded)
		{
			//syslog(LOG_DEBUG, "detector leave\n");
			//postmessage();

			bStarted	= false;
			bEnded	= false;
			pthread_mutex_lock(&g_ImageDataMutex);
			g_status.clear();
			pthread_mutex_unlock(&g_ImageDataMutex);

			// car leave
			syslog(LOG_DEBUG, "==========================Car Leave Detector!========================\n");

			// 
			JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
			for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
			{
				if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "14") == 0)
				{
					strcpy(pUserData->sequence_no, "14");
					pUserData->sequence_no[2]	= '\0';
					strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
					pUserData->device_tag[31]	= '\0';
					strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
					pUserData->event_id[31]	= '\0';
					break;
				}
			}
			PublishEventData(NULL, (void*)pUserData);
			delete pUserData;
			pUserData	= NULL;			
		}

		SysSleep(200);
	}
	return;
}

int ProcessImage::CreateSocket()
{
	int nSocket;
	nSocket=(int)socket(PF_INET,SOCK_STREAM,0);
	return nSocket;
}
int ProcessImage::ConnectSocket(int nSocket,const char * szHost,int nPort)
{
	hostent *pHost=NULL;
#if defined(_WIN32)||defined(_WIN64)
	pHost=gethostbyname(szHost);
	if(pHost==0)
	{
		return 0;
	}
#else
	hostent localHost;
	char pHostData[MAX_HOST_DATA];
	int h_errorno=0;
	//#ifdef	Linux
	int h_rc=gethostbyname_r(szHost,&localHost,pHostData,MAX_HOST_DATA,&pHost,&h_errorno);
	if((pHost==0)||(h_rc!=0))
	{
		return 0;
	}
	//#else
	//	//	we assume defined SunOS
	//	pHost=gethostbyname_r(szHost,&localHost,pHostData,MAX_HOST_DATA,&h_errorno);
	//	if((pHost==0))
	//	{
	//		return 0;
	//	}
	//#endif
#endif
	struct in_addr in;
	memcpy(&in.s_addr, pHost->h_addr_list[0],sizeof (in.s_addr));
	sockaddr_in name;
	memset(&name,0,sizeof(sockaddr_in));
	name.sin_family=AF_INET;
	name.sin_port=htons((unsigned short)nPort);
	name.sin_addr.s_addr=in.s_addr;
#if defined(_WIN32)||defined(_WIN64)
	int rc=connect((SOCKET)nSocket,(sockaddr *)&name,sizeof(sockaddr_in));
#else
	int rc=connect(nSocket,(sockaddr *)&name,sizeof(sockaddr_in));
#endif
	if(rc>=0)
		return 1;
	return 0;
}
int ProcessImage::CheckSocketValid(int nSocket)
{
	//	check socket valid
#if !defined(_WIN32)&&!defined(_WIN64)
	if(nSocket==-1)
		return 0;
	else
		return 1;
#else
	if(((SOCKET)nSocket)==INVALID_SOCKET)
		return 0;
	else
		return 1;
#endif
}
int ProcessImage::CloseSocket(int nSocket)
{
	int rc=0;
	if(!CheckSocketValid(nSocket))
	{
		return rc;
	}
#if	defined(_WIN32)||defined(_WIN64)
	shutdown((SOCKET)nSocket,SD_BOTH);
	closesocket((SOCKET)nSocket);
#else
	shutdown(nSocket,SHUT_RDWR);
	close(nSocket);
#endif
	rc=1;
	return rc;
}
void ProcessImage::SetSocketNotBlock(int nSocket)
{
	//	改变文件句柄为非阻塞模式
#if	defined(_WIN32)||defined(_WIN64)
	ULONG optval2=1;
	ioctlsocket((SOCKET)nSocket,FIONBIO,&optval2);
#else
	long fileattr;
	fileattr=fcntl(nSocket,F_GETFL);
	fcntl(nSocket,F_SETFL,fileattr|O_NDELAY);
#endif
}
void ProcessImage::SysSleep(long nTime) //	延时nTime毫秒，毫秒是千分之一秒
{
#if defined(_WIN32 )||defined(_WIN64)
	//	windows 代码
	MSG msg;
	while(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
	{
		if(GetMessage(&msg,NULL,0,0)!=-1)
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}
	Sleep(nTime);
#else
	//	unix/linux代码
	timespec localTimeSpec;
	timespec localLeftSpec;
	localTimeSpec.tv_sec=nTime/1000;
	localTimeSpec.tv_nsec=(nTime%1000)*1000000;
	nanosleep(&localTimeSpec,&localLeftSpec);
#endif
}
int ProcessImage::SocketWrite(int nSocket,char * pBuffer,int nLen,int nTimeout)
{
	int nOffset=0;
	int nWrite;
	int nLeft=nLen;
	int nLoop=0;
	int nTotal=0;
	int nNewTimeout=nTimeout*10;
	while((nLoop<=nNewTimeout)&&(nLeft>0))
	{
		nWrite=send(nSocket,pBuffer+nOffset,nLeft,0);
		if(nWrite==0)
		{
			return -1;
		}
#if defined(_WIN32)||defined(_WIN64)
		if(nWrite==SOCKET_ERROR)
		{
			if(WSAGetLastError()!=WSAEWOULDBLOCK)
			{	
				return -1;
			}
		}
#else
		if(nWrite==-1)
		{
			if(errno!=EAGAIN)
			{
				return -1;
			}
		}
#endif
		if(nWrite<0)
		{
			return nWrite;
		}	
		nOffset+=nWrite;
		nLeft-=nWrite;
		nTotal+=nWrite;
		if(nLeft>0)
		{
			//	延时100ms
			SysSleep(100);
		}
		nLoop++;
	}
	return nTotal;
}
int  ProcessImage::SocketRead(int nSocket,void * pBuffer,int nLen)
{
	if(nSocket==-1)
		return -1;
	int len=0;
#if	defined(_WIN32)||defined(_WIN64)
	len=recv((SOCKET) nSocket,(char *)pBuffer,nLen,0);
#else
	len=recv(nSocket,(char *)pBuffer,nLen,0);
#endif
	if(len==0)
	{
		return -1;
	}
	if(len==-1)
	{
#if	defined(_WIN32)||defined(_WIn64)
		int localError=WSAGetLastError();
		if(localError==WSAEWOULDBLOCK)
			return 0;
		return -1;
#else
		if(errno==0)
			return -1;
		if(errno==EAGAIN)
			return 0;
#endif		
		return len;
	}
	if(len>0)
		return len;
	else
		return -1;
}
size_t ProcessImage::ReadAll(FILE *fd, void *buff, size_t len)
{
	size_t n	= 0;
	size_t sum	= 0;
	do 
	{
		n = fread((char*)buff + sum, 1, len - sum, fd);
		sum += n;
	} while (sum < len && n != 0);
	if (n == 0 && ferror(fd))
		return 0;
	if (n == 0)
		return 1;
	return 1;
}
size_t ProcessImage::WriteAll(FILE *fd, void *buff, size_t len)
{
	size_t n	= 0;
	size_t sum	= 0;
	do 
	{
		n = fwrite((char*)buff + sum, 1, len - sum, fd);
		sum += n;
	} while (sum < len && n != 0);
	if (n == 0 && ferror(fd))
		return 0;
	if (n == 0)
		return 1;
	return 1;
}

std::string ProcessImage::Byte2String(BYTE ch)
{
	BYTE a = ch;
	char buf[128] = {0};
	char tmp[10] = {0};
	char sav[10] = {0};
	//for (int i = 0; i < sizeof(a) / sizeof(BYTE); i++)
	for (int i = 0; i < 1; i++)
	{
		int j = 0;
		// itoa((int)a[i], sav, 2);
		my_itoa((int)a, sav, 2);
		for (int w = 0; w < 8 - strlen(sav); w++)
			tmp[w] = '0';
		for (int w = 8 - strlen(sav); w < 8; w++)
			tmp[w] = sav[j++];
		sprintf(buf, "%8s", tmp);
		//sprintf(buf, "%s%8s", buf, tmp);
		string strRet = buf;
		return strRet;
	}
	return "";
}
int ProcessImage::my_itoa(int val, char* buf, int radix)
{
	//const int radix = 2;
	char* p;
	int a;        //every digit
	int len;
	char* b;    //start of the digit char
	char temp;
	p = buf;
	if (val < 0)
	{
		*p++ = '-';
		val = 0 - val;
	}
	b = p;
	do
	{
		a = val % radix;
		val /= radix;
		*p++ = a + '0';
	} while (val > 0);
	len = (int)(p - buf);
	*p-- = 0;
	//swap
	do
	{
		temp = *p;
		*p = *b;
		*b = temp;
		--p;
		++b;
	} while (b < p);
	return len;
}
string ProcessImage::GetCurTime(void)
{
	time_t t		= time(0);
	tm *ld			= NULL;
	char tmp[64]	= "";
	ld				= localtime(&t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ld);
	return string(tmp);
}
long ProcessImage::getCurrentTime()    
{    
	struct timeval tv;    
	gettimeofday(&tv,NULL);    
	//return tv.tv_sec * 1000 + tv.tv_usec / 1000; 
	return tv.tv_sec;    
}  

int ProcessImage::set_keep_live(int nSocket, int keep_alive_times, int keep_alive_interval)
{

#ifdef WIN32                                //WIndows下
	TCP_KEEPALIVE inKeepAlive = {0}; //输入参数
	unsigned long ulInLen = sizeof (TCP_KEEPALIVE);

	TCP_KEEPALIVE outKeepAlive = {0}; //输出参数
	unsigned long ulOutLen = sizeof (TCP_KEEPALIVE);

	unsigned long ulBytesReturn = 0;

	//设置socket的keep alive为5秒，并且发送次数为3次
	inKeepAlive.on_off = keep_alive_times;
	inKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
	inKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


	outKeepAlive.on_off = keep_alive_times;
	outKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
	outKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


	if (WSAIoctl((unsigned int) nSocket, SIO_KEEPALIVE_VALS,
		(LPVOID) & inKeepAlive, ulInLen,
		(LPVOID) & outKeepAlive, ulOutLen,
		&ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
	{
		//ACE_DEBUG((LM_INFO,
		//	ACE_TEXT("(%P|%t) WSAIoctl failed. error code(%d)!\n"), WSAGetLastError()));
	}

#else                                        //linux下
	int keepAlive = 1; //设定KeepAlive
	int keepIdle = keep_alive_interval; //开始首次KeepAlive探测前的TCP空闭时间
	int keepInterval = keep_alive_interval; //两次KeepAlive探测间的时间间隔
	int keepCount = keep_alive_times; //判定断开前的KeepAlive探测次数
	if (setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*) & keepAlive, sizeof (keepAlive)) == -1)
	{
		syslog(LOG_DEBUG, "setsockopt SO_KEEPALIVE error!\n");
	}
	if (setsockopt(nSocket, SOL_TCP, TCP_KEEPIDLE, (const char *) & keepIdle, sizeof (keepIdle)) == -1)
	{
		syslog(LOG_DEBUG, "setsockopt TCP_KEEPIDLE error!\n");
	}
	if (setsockopt(nSocket, SOL_TCP, TCP_KEEPINTVL, (const char *) & keepInterval, sizeof (keepInterval)) == -1)
	{
		syslog(LOG_DEBUG, "setsockopt TCP_KEEPINTVL error!\n");
	}
	if (setsockopt(nSocket, SOL_TCP, TCP_KEEPCNT, (const char *) & keepCount, sizeof (keepCount)) == -1)
	{
		syslog(LOG_DEBUG, "setsockopt TCP_KEEPCNT error!\n");
	}

#endif

	return 0;

}
