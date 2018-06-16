/* 
* File:   ProcessImage.cpp
* Author: root
* 
* Created on 2015年1月28日, 上午10:45
*/
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
#include <syslog.h>
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
#include "RecoClientSDK.h"
#include "jzLocker.h"

using namespace boost::uuids;
using namespace std;
using namespace boost;
int GlobalStopFlag = 0;
int	g_nHoppingCount = 0;
int g_nShield		= 0;
vector<pair<BYTE, structUploadData> > g_statusVect;
std::vector<std::string>	g_statusTableVect;
std::vector<std::string>	g_statusTablePassVect;
std::vector<BYTE>			g_loseSeqNoVect;
vector<string>	g_DOStatusVect;
vector<char>	g_resDOStatusVect;
#define MAX_HOST_DATA 4096
extern std::stack<structImageData>				g_ImageDataStack;
extern vector<structBoxNumberRecogResult>		g_boxNumberSet;
extern vector<structVP_PASS_VEHICLE_INFO>		g_VehicleDataVect;
extern std::stack<structVP_PASS_VEHICLE_INFO>	g_VehicleDataStack;

extern pthread_mutex_t              g_ImageDataMutex;
extern pthread_cond_t               g_ImageDataCond;

extern pthread_mutex_t g_VehicleDataVectMutex;
extern pthread_mutex_t g_VehicleDataStackMutex;

extern sem				g_VehicleSemArrived;
extern sem				g_VehicleSemStart;
extern sem				g_VehicleSemStop;
//extern sem				g_VehicleSemFinishded;

using namespace std;
void ReceiveReaderData(unsigned char* read_data, void* user_data) 
{
	structBoxNumberRecogResultCorrected *pUserData = (structBoxNumberRecogResultCorrected *)user_data;
	if (pUserData == NULL)
		return;
	NET_PACKET_MSG* pMsg_ = NULL;
	CMSG_Handle_Center::get_message(pMsg_);
	if (!pMsg_) 
	{
		return;
	}
	memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
	pMsg_->msg_head.msg_type = SYS_MSG_PUBLISH_EVENT;
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData);
	T_SysEventData* pReadData = (T_SysEventData*) pMsg_->msg_body;



	//strcpy(pReadData->event_id, CSimpleConfig::m_eventVect[9].event_id); // SYS_EVENT_CONTA_RECOG_COMPLETE);
	//pReadData->event_id[31]	= '\0';
	for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
	{
		if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "10") == 0)
		{
			strcpy(pReadData->event_id, CSimpleConfig::m_eventVect[i].event_id);
			pReadData->event_id[31]	= '\0';
			break;
		}
	}


	strcpy(pReadData->device_tag, CSimpleConfig::DEVICE_TAG);
	// strcpy(pReadData->device_tag, "CONTA");
	pReadData->is_data_full	= 0; // 1;
	char szXMLGatherInfo[10 * 1024] = {0};
	string strContNum = "";
	if (pUserData->nBoxType	== 1 || pUserData->nBoxType == 3 || pUserData->nBoxType == 4)
		strContNum = "1";
	else if (pUserData->nBoxType == 2)
		strContNum = "2";
	if (pUserData->strFrontBoxNumber != "" && pUserData->strBackBoxNumber != "")
		strContNum = "2";
	string strContReco = pUserData->strRecogResult;

	if (strContReco.empty() || strContNum.empty() || strContReco == "0")
		pReadData->is_data_full	=  1;


	if (strContNum == "1")
	{
		sprintf(szXMLGatherInfo,
			"<CONTA>"
			"<CONTA_NUM>%s</CONTA_NUM>"
			"<CONTA_RECO>%s</CONTA_RECO>"
			"<CONTA_ID_F>%s</CONTA_ID_F>"
			"<CONTA_ID_B>%s</CONTA_ID_B>"
			"<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
			"<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
			"</CONTA>"
			"<CONTA_PIC>"
			"<CONTA_PIC_F>%s</CONTA_PIC_F>"
			"<CONTA_PIC_B>%s</CONTA_PIC_B>"
			"<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
			"<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
			"<CONTA_PIC_LF>%s</CONTA_PIC_LF>"			
			"<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
			"</CONTA_PIC>"
			, strContNum.c_str()
			, strContReco.c_str()
			, pUserData->strFrontBoxNumber.c_str()
			, pUserData->strBackBoxNumber.c_str()
			, pUserData->strFrontBoxModel.c_str()
			, pUserData->strBackBoxModel.c_str()
			, pUserData->CONTA_PIC_F.c_str()
			, pUserData->CONTA_PIC_B.c_str()
			, pUserData->CONTA_PIC_RF.c_str()
			, pUserData->CONTA_PIC_LB.c_str()
			, pUserData->CONTA_PIC_LF.c_str()
			, pUserData->CONTA_PIC_RB.c_str()
			);  
	}
	else
	{
		sprintf(szXMLGatherInfo,
			"<CONTA>"
			"<CONTA_NUM>%s</CONTA_NUM>"
			"<CONTA_RECO>%s</CONTA_RECO>"
			"<CONTA_ID_F>%s</CONTA_ID_F>"
			"<CONTA_ID_B>%s</CONTA_ID_B>"
			"<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
			"<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
			"</CONTA>"
			"<CONTA_PIC>"
			"<CONTA_PIC_F>%s</CONTA_PIC_F>"
			"<CONTA_PIC_B>%s</CONTA_PIC_B>"
			"<CONTA_PIC_LF>%s</CONTA_PIC_LF>"
			"<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
			"<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
			"<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
			"</CONTA_PIC>"
			, strContNum.c_str()
			, strContReco.c_str()
			, pUserData->strFrontBoxNumber.c_str()
			, pUserData->strBackBoxNumber.c_str()
			, pUserData->strFrontBoxModel.c_str()
			, pUserData->strBackBoxModel.c_str()
			, pUserData->CONTA_PIC_F.c_str()
			, pUserData->CONTA_PIC_B.c_str()
			, pUserData->CONTA_PIC_LF.c_str()
			, pUserData->CONTA_PIC_RF.c_str()
			, pUserData->CONTA_PIC_LB.c_str()
			, pUserData->CONTA_PIC_RB.c_str()
			);  
	}
	syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
	strcpy(pReadData->xml_data, szXMLGatherInfo);
	pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;
	syslog(LOG_DEBUG, "pack len:%d, bucaiflag=%d\n", pMsg_->msg_head.packet_len, pReadData->is_data_full);
	MSG_HANDLE_CENTER::instance()->put(pMsg_);
	return;
}

void PublishEventData(unsigned char* read_data, void* user_data) 
{
	JZ_PUBLISH_EVENT_STRUCT *pUserData = (JZ_PUBLISH_EVENT_STRUCT *)user_data;
	//memset(pUserData, 0x00, sizeof(JZ_PUBLISH_EVENT_STRUCT));
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
	char szXMLGatherInfo[1024] = {0};

	pUserData->PCAR_NO[16]	= '\0';
	std::string strCarNo = pUserData->PCAR_NO;


	sprintf(szXMLGatherInfo, "<OPTCAR><PCAR_NO>%s</PCAR_NO><PCAR_NO_PICNAME>%s</PCAR_NO_PICNAME><PCAR_PICNAME>%s</PCAR_PICNAME></OPTCAR>\n", strCarNo.c_str(), pUserData->PCAR_NO_PICNAME, pUserData->PCAR_PICNAME);
	syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
	strcpy(pReadData->xml_data, szXMLGatherInfo);
	pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;
	syslog(LOG_DEBUG, "pack len:%d\n", pMsg_->msg_head.packet_len);
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
	//pthread_create (&controllerThreadID, NULL, ProcessControllerThread, NULL);
	//pthread_create (&statusTableThreadID, NULL, ProcessStatusTableThread, NULL);
	pthread_create (&arrivedThreadID, NULL, ProcessArrivedThread, NULL);   
	pthread_create (&startThreadID, NULL, ProcessStartThread, NULL);   
	pthread_create (&stopThreadID, NULL, ProcessStopThread, NULL);   
	//pthread_create (&finishedThreadID, NULL, ProcessFinishedThread, NULL);   

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

void *ProcessImage::ProcessArrivedThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessArrivedProc();
}

void *ProcessImage::ProcessStartThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessStartProc();
}

void *ProcessImage::ProcessStopThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessStopProc();
}

//void *ProcessImage::ProcessFinishedThread(void* pParam)
//{
//	ProcessImage *pThis = (ProcessImage*)pParam;
//	pThis->ProcessFinishedProc();
//}


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
		//syslog(LOG_DEBUG, "%s Send Data:", GetCurTime().c_str());

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
				//syslog(LOG_DEBUG, "%s Recv Data:", GetCurTime().c_str());
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
						//syslog(LOG_DEBUG, "%s Send Data:", GetCurTime().c_str());
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
	bool bStart = false;
	bool bEnd	= false;
	long lLastTime	= getCurrentTime();
	long lNowTime	= lLastTime;
	int nStatusSize	= 0;
	pthread_mutex_lock (&g_ImageDataMutex); 
	nStatusSize = g_statusTableVect.size();
	pthread_mutex_unlock (&g_ImageDataMutex);

	string strDiEnable = CSimpleConfig::DI_ENABLE;
	//printf("nStatusSize:%d\n", nStatusSize);
	vector<BYTE> seqNoVect;
	while (!GlobalStopFlag)
	{
		lNowTime	= getCurrentTime();
		if (g_statusVect.size() > 0)
		{
			std::pair<BYTE, structUploadData> &pairData = g_statusVect.front();
			BYTE ch = pairData.second.Di_Last;
			string str = Byte2String(ch);
			// string inStr = str.substr(4);
			string inStr = "";
			for (int j = 0; j < 8; ++j)
			{
				if (strDiEnable[j] == '1')
					inStr += str[j];
			}

			pthread_mutex_lock (&g_ImageDataMutex);
			g_statusTableVect.push_back(inStr);
			pthread_mutex_unlock (&g_ImageDataMutex);

			//printf("inStr :%s\n", inStr.c_str());
			ch = pairData.second.Di_Now;
			str = Byte2String(ch);
			
			//inStr = str.substr(4);
			inStr	= "";
			for (int j = 0; j < 8; ++j)
			{
				if (strDiEnable[j] == '1')
					inStr += str[j];
			}

			pthread_mutex_lock (&g_ImageDataMutex);
			g_statusTableVect.push_back(inStr);
			pthread_mutex_unlock (&g_ImageDataMutex);

			//printf("inStr :%s\n", inStr.c_str());
			ch = pairData.second.Do_Now;
			str = Byte2String(ch);
			
			seqNoVect.push_back(pairData.first);
			pthread_mutex_lock (&g_ImageDataMutex); 
			g_DOStatusVect.push_back(str);
			g_statusVect.erase(g_statusVect.begin());
			pthread_mutex_unlock (&g_ImageDataMutex);
		}

		pthread_mutex_lock (&g_ImageDataMutex);
		nStatusSize = g_statusTableVect.size();
		pthread_mutex_unlock (&g_ImageDataMutex);

		//printf("=====================nStatusSize:%d===================\n", nStatusSize);
		//for (int i = 0; i < g_statusTableVect.size(); ++i)
		//{
		//	printf("%s \n", g_statusTableVect[i].c_str());
		//}
		//printf("=======================================================\n");
		if (nStatusSize > 1)
		{
			for (int i = 2; i < nStatusSize + 1; ++i)
			{
				if ( !bStart &&  g_statusTableVect[i - 2] == "1110" && g_statusTableVect[i - 1] == "1100")
				{
					// 集装箱到达事件
					syslog(LOG_DEBUG, "Box arrived event:%s\n", GetCurTime().c_str());
					JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
					memset(pUserData, 0x00, sizeof(JZ_PUBLISH_EVENT_STRUCT));
					for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
					{
						if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "1") == 0)
						{
							strcpy(pUserData->sequence_no, "0");
							pUserData->sequence_no[1]	= '\0';
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


					// start
					syslog(LOG_DEBUG, "===============================================================time sequence start!\n");
					bStart = true;
					g_nHoppingCount = 0;
					lLastTime	= lNowTime;
					BYTE ch = ' ';
					if (seqNoVect.size() > 0)
						ch = seqNoVect[seqNoVect.size() - 1];
					seqNoVect.clear();
					if (ch != ' ')
						seqNoVect.push_back(ch);
					continue;
				}
				//if (bStart && !bEnd &&  g_statusTableVect[i - 2].substr(1) == "001" && g_statusTableVect[i - 1].substr(1) == "011")
                                if (bStart && !bEnd && g_statusTableVect[i - 1] == "0011")
				{
					//BYTE chIncrement	= 0x01;
					//BYTE chStartSeqNo	= 0;
					//BYTE chEndSeqNo		= 0;
					//if (seqNoVect.size() > 0)
					//{
					//	chStartSeqNo = seqNoVect[0];
					//	chEndSeqNo   = seqNoVect[seqNoVect.size() - 1];
					//}
					//vector<BYTE> newSeqNoVect;
					//for (BYTE j = chStartSeqNo; j != chEndSeqNo + 1; ++j)
					//{
					//	newSeqNoVect.push_back(j);
					//}
					//vector<BYTE> loseSeqNoVect;
					//for (BYTE j = 0, k = 0; j < newSeqNoVect.size() && k < seqNoVect.size();)
					//{
					//	if (newSeqNoVect[j] == seqNoVect[k])
					//	{
					//		++j;
					//		++k;
					//		continue;
					//	}
					//	else
					//	{
					//		loseSeqNoVect.push_back(newSeqNoVect[j]);
					//		++j;
					//	}
					//}
					//bool bLose = false;
					//if (loseSeqNoVect.size() > 0)
					//{
					//	// lose seqno
					//	bLose	= true;
					//}
					//else
					//{
					//	bLose = false;
					//}
					//if (bLose)
					//{
					//	for (size_t i = 0; i < loseSeqNoVect.size(); ++i)
					//		g_loseSeqNoVect.push_back(loseSeqNoVect[i]);
					//	SysSleep(500);
					//	// 后面不再检测了
					//}
					// trigger
					// end

					lLastTime	= lNowTime;
					bStart	= false;
					bEnd = true;
					syslog(LOG_DEBUG, "===============================================================time sequence end!\n");
					int nHoppingCount = 0;
					for (size_t j = 0; j < g_DOStatusVect.size(); j += 1)
					{
						//vector<char> diVect1;
						//vector<char> diVect2;
						string str1 = g_DOStatusVect[j].substr(4);
						if (j + 1 < g_DOStatusVect.size())
						{
							string str2 = g_DOStatusVect[j + 1].substr(4);
							for (size_t k = 0, m = 0; k < str1.size() &&  m < str2.size(); ++k, ++m)
							{
								//diVect1[k] = str1[k];
								if (str1[k] == '0' && str2[m] == '1')
									++nHoppingCount;
							}
							//for (size_t k = 0; k < str2.size(); ++k)
							//	diVect2[k] = str2[k];
							//set_difference(diVect2.begin(), diVect2.end(), diVect1.begin(), diVect1.end(), back_inserter(g_resDOStatusVect));
						}
						else
						{
							break;
						}
					}
					pthread_mutex_lock (&g_ImageDataMutex); 
					copy(g_statusTableVect.begin(), g_statusTableVect.end(), back_inserter(g_statusTablePassVect));
					g_nHoppingCount = nHoppingCount;
					g_statusVect.clear();
					g_statusTableVect.clear();
					g_DOStatusVect.clear();
					pthread_cond_signal (&g_ImageDataCond);
					pthread_mutex_unlock (&g_ImageDataMutex);
					break;
				}
				else if (!bStart && bEnd &&  g_statusTableVect[i - 2] == "0111" && g_statusTableVect[i - 1] == "1111")
				{
					// 车辆过卡口
					syslog(LOG_DEBUG, "===============================================================car leave\n");
					bStart	= false;
					bEnd	= false;
					pthread_mutex_lock (&g_ImageDataMutex);  
					g_statusVect.clear();
					g_statusTableVect.clear();
					g_DOStatusVect.clear();
					pthread_mutex_unlock (&g_ImageDataMutex);
					break;
					//g_resDOStatusVect.clear();
				}
				else if (g_statusTableVect[i - 2] == "1110" && g_statusTableVect[i - 1] == "1111")
				{
//					// dao che
//					printf("===============================================================car astern\n");
//					lLastTime	= lNowTime;
//					bStart	= false;
//					bEnd	= false;
//					pthread_mutex_lock (&g_ImageDataMutex);  
//					g_statusVect.clear();
//					g_statusTableVect.clear();
//					g_DOStatusVect.clear();
//					pthread_mutex_unlock (&g_ImageDataMutex);
//					break;
//					//g_resDOStatusVect.clear();
				}
				else
				{
					continue;
				}
			}
		}
		if (bStart && lNowTime - lLastTime > 60)
		{
			syslog(LOG_DEBUG, "===============================================================Timing Sequence is timeout!\n");
			bStart	= false;
			bEnd	= true;
			int nHoppingCount = 0;
			lLastTime	= lNowTime;
			for (size_t j = 0; j < g_DOStatusVect.size(); j += 1)
			{
				//vector<char> diVect1;
				//vector<char> diVect2;
				string str1 = g_DOStatusVect[j];
                                string str11 = str1.substr(4);
				if (j + 1 < g_DOStatusVect.size())
				{
					string str2 = g_DOStatusVect[j + 1].substr(4);
                                        string str22 = str2.substr(4);
					for (size_t k = 0, m = 0; k < str11.size() &&  m < str22.size(); ++k, ++m)
					{
						//diVect1[k] = str1[k];
						if (str11[k] == '0' && str22[m] == '1')
							++nHoppingCount;
					}
					//for (size_t k = 0; k < str2.size(); ++k)
					//	diVect2[k] = str2[k];
					//set_difference(diVect2.begin(), diVect2.end(), diVect1.begin(), diVect1.end(), back_inserter(g_resDOStatusVect));
				}
				else
				{
					break;
				}
			}
			pthread_mutex_lock (&g_ImageDataMutex); 
			copy(g_statusTableVect.begin(), g_statusTableVect.end(), back_inserter(g_statusTablePassVect));
			g_nHoppingCount = nHoppingCount;
			g_statusVect.clear();
			g_statusTableVect.clear();
			g_DOStatusVect.clear();
			pthread_cond_signal (&g_ImageDataCond);
			pthread_mutex_unlock (&g_ImageDataMutex);
		}
		//sleep(1);
		SysSleep(100);
	}
	return;
}

void ProcessImage::ProcessArrivedProc()
{
//	// test code
//	while (!GlobalStopFlag)
//	{
//		printf("============================\n");
//		getchar();
//
//		//structBoxNumberRecogResultCorrected result;
//		//result.strFrontBoxNumber	= "GESU5520648";
//		//result.strBackBoxNumber		= "";
//		//result.strFrontBoxModel		= "45G1";
//		//result.strBackBoxModel		= "";
//		//result.strRecogResult		= "1";
//		//result.nBoxType				= 1;
//		//result.nPicNumber			= 4;
//		//result.CONTA_PIC_F			= "/dev/shm/2014-12-18_10-58-32_875_4773.jpg";
//		//result.CONTA_PIC_B			= "/dev/shm/2014-12-18_10-58-32_875_4774.jpg";
//		//result.CONTA_PIC_RF			= "/dev/shm/2014-12-18_10-58-32_875_4775.jpg";
//		//result.CONTA_PIC_LB			= "/dev/shm/2014-12-18_10-58-32_875_4776.jpg";
//
//		//ReceiveReaderData(NULL, &result);
//
//
//		char szFileName[256]={0};
//		char szSeq[8]={0};
//
//		for(int i=0;i<1;i++)
//		{
//			sprintf(szFileName,"/dev/shm/1.jpg");
//			sprintf(szSeq,"%s.jpg","1");
//
//			printf("ContaReco :%s, %s\n", szSeq, szFileName);
//			ContaReco(szSeq,szFileName);
//			sleep(2);;
//		}
//
//
//
//		getchar();
//		printf("============================\n");
//	}
//	return;
	
	// test code end
	while (!GlobalStopFlag)
	{
		g_VehicleSemArrived.wait();

		if (g_nShield == 0)
			syslog(LOG_DEBUG, "========================EG_ARRIVED_OPTCAR=================\n");

		pthread_mutex_lock (&g_VehicleDataVectMutex); 
		if (g_VehicleDataVect.empty())
		{
			pthread_mutex_unlock (&g_VehicleDataVectMutex);
			continue;
		}
		
		structVP_PASS_VEHICLE_INFO vehicleData = g_VehicleDataVect.front();
		g_VehicleDataVect.erase(g_VehicleDataVect.begin());
		pthread_mutex_unlock (&g_VehicleDataVectMutex);


		syslog(LOG_DEBUG, "EG_ARRIVED_OPTCAR:%s\n", vehicleData.strHPHM.c_str());

		//if (vehicleData.strHPHM != "")
		{
			syslog(LOG_DEBUG, "process vehicle data:%s, %s\n", vehicleData.strHPHM.c_str(), GetCurTime().c_str());

			if(vehicleData.pPicBuffer != NULL && vehicleData.nPicLen > 0)
			{
				char szFileName[256]={0};
				sprintf(szFileName, "/root/recvpic/%s_big.jpg", vehicleData.strPassTime.c_str());
				FILE* fPic = fopen(szFileName, "wb");
				fwrite(vehicleData.pPicBuffer, 1, vehicleData.nPicLen, fPic);
				fclose(fPic);
			}

			if(vehicleData.pPlateBuffer != NULL && vehicleData.nPlateLen > 0)
			{
				char szFileName[256]={0};
				sprintf(szFileName, "/root/recvpic/%s_small.jpg", vehicleData.strPassTime.c_str());
				FILE* fPic = fopen(szFileName, "wb");
				fwrite(vehicleData.pPlateBuffer, 1, vehicleData.nPlateLen, fPic);
				fclose(fPic);
			}
		}

		if (g_nShield == 0)
		{
			JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
			memset(pUserData, 0x00, sizeof(JZ_PUBLISH_EVENT_STRUCT));
			for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
			{
				if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "4") == 0)
				{
					strcpy(pUserData->sequence_no, "4");
					pUserData->sequence_no[2]	= '\0';
					strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
					pUserData->device_tag[31]	= '\0';
					strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
					pUserData->event_id[31]	= '\0';

					strcpy(pUserData->PCAR_NO, vehicleData.strHPHM.c_str());
					pUserData->PCAR_NO[16] = '\0';

					if(vehicleData.pPicBuffer != NULL && vehicleData.nPicLen > 0)
					{
						char szFileName[256]={0};
						sprintf(szFileName, "/root/recvpic/%s_big.jpg", vehicleData.strPassTime.c_str());
						strcpy(pUserData->PCAR_PICNAME, szFileName);
						pUserData->PCAR_PICNAME[255] = '\0';	
					}

					if(vehicleData.pPlateBuffer != NULL && vehicleData.nPlateLen > 0)
					{
						char szFileName[256]={0};
						sprintf(szFileName, "/root/recvpic/%s_small.jpg", vehicleData.strPassTime.c_str());
						strcpy(pUserData->PCAR_NO_PICNAME, szFileName);
						pUserData->PCAR_NO_PICNAME[255] = '\0';	
					}
					break;
				}
			}
			PublishEventData(NULL, (void*)pUserData);
			delete pUserData;
			pUserData	= NULL;
		}

	
		pthread_mutex_lock (&g_VehicleDataStackMutex); 
		g_VehicleDataStack.push(vehicleData);
		pthread_mutex_unlock (&g_VehicleDataStackMutex);

		//ReceiveReaderData(NULL, (void*)pResult);
		//if (vehicleData.pPicBuffer != NULL && vehicleData.nPicLen > 0)
		//{
		//	delete []vehicleData.pPicBuffer;
		//	vehicleData.nPicLen	= 0;
		//}

		//if (vehicleData.pPlateBuffer != NULL && vehicleData.nPlateLen > 0)
		//{
		//	delete []vehicleData.pPlateBuffer;
		//	vehicleData.nPlateLen	= 0;
		//}

	}    
	return;
}

void ProcessImage::ProcessStartProc()
{
	while (!GlobalStopFlag)
	{
		g_VehicleSemStart.wait();
		//syslog(LOG_DEBUG, "========================Recv EC_START_OPTCAR=================\n");
		structVP_PASS_VEHICLE_INFO vehicleData;

		int nHaveDataTimeout = 6 + 1;
		while (nHaveDataTimeout)
		{
			if (!g_VehicleDataStack.empty())
				break;
			else
			{
				--nHaveDataTimeout;
				SysSleep(1000);
			}
		}
		//if (nHaveDataTimeout == 0)
		//{

		//}

		pthread_mutex_lock (&g_VehicleDataStackMutex); 
		if (g_VehicleDataStack.empty())
		{
			pthread_mutex_unlock (&g_VehicleDataStackMutex);
			continue;
		}


		int i = g_VehicleDataStack.size();
		while (!g_VehicleDataStack.empty())
		{
			if (i > 1)
			{
				structVP_PASS_VEHICLE_INFO tmpVehicleData;
				tmpVehicleData = g_VehicleDataStack.top();
				g_VehicleDataStack.pop();

				if (tmpVehicleData.pPicBuffer != NULL && tmpVehicleData.nPicLen > 0)
				{
					delete []tmpVehicleData.pPicBuffer;
					tmpVehicleData.nPicLen	= 0;
				}

				if (tmpVehicleData.pPlateBuffer != NULL && tmpVehicleData.nPlateLen > 0)
				{
					delete []tmpVehicleData.pPlateBuffer;
					tmpVehicleData.nPlateLen	= 0;
				}
				--i;
				continue;
			}

			vehicleData = g_VehicleDataStack.top();
			g_VehicleDataStack.pop();
			--i;
		}			
		pthread_mutex_unlock (&g_VehicleDataStackMutex);

		JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
		memset(pUserData, 0x00, sizeof(JZ_PUBLISH_EVENT_STRUCT));
		for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
		{
			if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "3") == 0)
			{
				strcpy(pUserData->sequence_no, "3");
				pUserData->sequence_no[2]	= '\0';
				strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
				pUserData->device_tag[31]	= '\0';
				strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
				pUserData->event_id[31]	= '\0';

				strcpy(pUserData->PCAR_NO, vehicleData.strHPHM.c_str());
				pUserData->PCAR_NO[16] = '\0';

				if(vehicleData.pPicBuffer != NULL && vehicleData.nPicLen > 0)
				{
					char szFileName[256]={0};
					sprintf(szFileName, "/root/recvpic/%s_big.jpg", vehicleData.strPassTime.c_str());
					strcpy(pUserData->PCAR_PICNAME, szFileName);
					pUserData->PCAR_PICNAME[255] = '\0';	
				}

				if(vehicleData.pPlateBuffer != NULL && vehicleData.nPlateLen > 0)
				{
					char szFileName[256]={0};
					sprintf(szFileName, "/root/recvpic/%s_small.jpg", vehicleData.strPassTime.c_str());
					strcpy(pUserData->PCAR_NO_PICNAME, szFileName);
					pUserData->PCAR_NO_PICNAME[255] = '\0';	
				}
				
				break;
			}
		}
		PublishEventData(NULL, (void*)pUserData);
		syslog(LOG_DEBUG, "========================Send EG_FINISHED_OPTCAR=================\n");

		//pthread_mutex_lock (&g_VehicleDataStackMutex);
		//g_nShield	= 0;
		//pthread_mutex_unlock (&g_VehicleDataStackMutex);

		delete pUserData;
		pUserData	= NULL;

		if (vehicleData.pPicBuffer != NULL && vehicleData.nPicLen > 0)
		{
			delete []vehicleData.pPicBuffer;
			vehicleData.nPicLen	= 0;
		}

		if (vehicleData.pPlateBuffer != NULL && vehicleData.nPlateLen > 0)
		{
			delete []vehicleData.pPlateBuffer;
			vehicleData.nPlateLen	= 0;
		}

	}    
	return;
}

void ProcessImage::ProcessStopProc()
{
	while (!GlobalStopFlag)
	{
		g_VehicleSemStop.wait();
		syslog(LOG_DEBUG, "========================Recv EC_STOP_OPTCAR=================\n");
		structVP_PASS_VEHICLE_INFO vehicleData;

		pthread_mutex_lock (&g_VehicleDataStackMutex); 
		if (g_VehicleDataStack.empty())
		{
			pthread_mutex_unlock (&g_VehicleDataStackMutex);
			continue;
		}

		while (!g_VehicleDataStack.empty())
		{
			structVP_PASS_VEHICLE_INFO tmpVehicleData;
			tmpVehicleData = g_VehicleDataStack.top();
			g_VehicleDataStack.pop();

			if (tmpVehicleData.pPicBuffer != NULL && tmpVehicleData.nPicLen > 0)
			{
				delete []tmpVehicleData.pPicBuffer;
				tmpVehicleData.nPicLen	= 0;
			}

			if (tmpVehicleData.pPlateBuffer != NULL && tmpVehicleData.nPlateLen > 0)
			{
				delete []tmpVehicleData.pPlateBuffer;
				tmpVehicleData.nPlateLen	= 0;
			}
		}			
		pthread_mutex_unlock (&g_VehicleDataStackMutex);
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
int ProcessImage::JudgeBoxType(std::vector<std::string>& statusVect, const std::vector<structBoxNumberRecogResult>& boxNumberSet, structBoxNumberRecogResultCorrected &resultCorrected)
{
	if (statusVect.size() == 0)
	{
		return 0;
	}
	if (boxNumberSet.size() == 0)
	{      
		return 0;
	}
	syslog(LOG_DEBUG, "Enter JudgeBoxType\n");
	// 根据对射触发判断长短箱流程-----single double judge procedure
	int nBoxType = 0;   // 1:长箱   2:双箱  3:单箱 4:短箱
	for (size_t i = 0; i < statusVect.size() - 1; ++i)
	{
		// 010x-> 000x
		if (statusVect[i].substr(1) == "010" && statusVect[i+1].substr(1) == "000")
		{
			nBoxType = 2;
			break;
		}
		if (statusVect[i].substr(1) == "000" && statusVect[i+1].substr(1) == "010")
		{
			nBoxType = 2;
			break;
		}
		// 长箱的判断标准是四对对射全部被挡住，即状态为0000
		if (statusVect[i] == "1001" && statusVect[i+1] == "1011")
		{
			nBoxType = 4;
		}
	}

	syslog(LOG_DEBUG, "===================================nBoxType:%d\n", nBoxType);
	if (nBoxType == 2 || nBoxType == 4)
	{
		// 确定是长箱或双箱或短箱
		syslog(LOG_DEBUG, "===================================222nBoxType:%d\n", nBoxType);
	}
	else
	{
		bool bModel = false;
		for (int i = 0; i < boxNumberSet.size(); ++i)
		{
			//if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '2')
			if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxModel[0] == '2')
			{
				bModel      = true;
				nBoxType    = 2;
				break;
			}
			//if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '4')
			if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxModel[0] == '4')
			{
				bModel      = true;
				nBoxType    = 1;
				break;
			}            
		}
		syslog(LOG_DEBUG, "===================================333nBoxType:%d\n", nBoxType);
		if (bModel)
		{
			// 确定是双箱或长箱
		}
		else
		{
			string strFront         = "";
			string strFrontRight    = "";
			string strBack          = "";
			string strBackLeft      = "";
			string strFrontLeft     = "";
			string strBackRight     = "";
			string strFrontRightArrange  = "";
			string strBackLeftArrange   = "";
			string strFrontLeftColor = "";
			string strBackLeftColor  = "";
			for (int i = 0; i < boxNumberSet.size(); ++i)
			{
				if (boxNumberSet[i].direct == FRONT)
					strFront        = boxNumberSet[i].strBoxNumber;
				if (boxNumberSet[i].direct == FRONTRIGHT)
				{
					strFrontRight   = boxNumberSet[i].strBoxNumber;  
					strFrontRightArrange  =   boxNumberSet[i].strArrangement;
				}
				if (boxNumberSet[i].direct == BACK)
					strBack        = boxNumberSet[i].strBoxNumber;
				if (boxNumberSet[i].direct == BACKLEFT)
				{
					strBackLeft         =   boxNumberSet[i].strBoxNumber;   
					strBackLeftArrange  =   boxNumberSet[i].strArrangement;
					strBackLeftColor    =   boxNumberSet[i].strBoxColor;
				}
				if (boxNumberSet[i].direct == FRONTLEFT)
				{
					strFrontLeft   = boxNumberSet[i].strBoxNumber;
					strFrontLeftColor   =   boxNumberSet[i].strBoxColor;
				}
				if (boxNumberSet[i].direct == BACKRIGHT)
					strBackRight   = boxNumberSet[i].strBoxNumber;                
			}
			// 前箱号前右箱号后4位相同么 && 后箱号与后左箱号后4位相同么
			if (strFront.size() == 11 && strBack.size() == 11 && strFront.substr(7, 4) == strBack.substr(7, 4)  && strFrontRight.substr(7, 4) == strBackLeft.substr(7, 4) )
			{
				// 确定是长箱
				nBoxType = 1;
			}           
			else
			{
				//                int nRet = 0;
				//                char chVerifyCode   = ' ';
				//                string strBoxNumber = strFrontLeft;
				//                BoxNumberCheckAlgo check;
				//                nRet = check.GetBoxNumCheckbit(strBoxNumber, chVerifyCode);
				//                if (nRet == 0)
				//                {
				//                    chVerifyCode    = ' ';
				//                    strBoxNumber    = strBackRight;
				//                    nRet = check.GetBoxNumCheckbit(strBoxNumber, chVerifyCode);                    
				//                }
				// 比较前左、后右有一个识别出来了么
				if (strFrontLeft != "" || strBackRight != "")
					//if (nRet == 1)
				{
					// 确定是双箱
					nBoxType = 2;
				}
				else
				{
					// 比较前右、后左箱号 排列方式是否不同
					if (strFrontRightArrange != strBackLeftArrange)
					{
						// 确定是双箱
						nBoxType = 2;
					}
					else
					{
						// 比较前左，后左箱体颜色，不同且差距大么
						if (strFrontLeftColor != strBackLeftColor)
						{
							// 确定是双箱
							nBoxType = 2;
						}
						else
						{
							// 自认为是单箱
							nBoxType = 3;
						}
					}
				}
			}
		}
	}
	//structBoxNumberRecogResultCorrected resultCorrected;
	// 能确定 是 单箱 、双箱 么
	if (nBoxType == 2 || nBoxType == 3)
	{
		// 单箱
		if (nBoxType == 3)
		{
			// 挑选出4副图片识别结果
			int nPicNumber = 4;
			// 进入箱号校验修正流程
			BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);            
		}
		else if (nBoxType == 2)// 双箱
		{
			// 每个箱子至少三个识别结果
			int nPicNumber = 6;
			// 进入箱号校验修正流程
			BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
		}
	}
	else if (nBoxType == 4)
	{
		// 挑选出4副图片识别结果
		int nPicNumber = 4;
		// 进入箱号校验修正流程
		BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);  
	}
	else
	{
		// 按长箱处理
		nBoxType = 1;
		// 挑选出4副图片识别结果
		int nPicNumber = 4;
		// 进入箱号校验修正流程
		BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
	}
	return 0;
}

int ProcessImage::BoxNumberCorrection(const std::vector<structBoxNumberRecogResult>& boxNumberSet, int nPicNumber, int nBoxType, structBoxNumberRecogResultCorrected &resultCorrected)
{ 
	syslog(LOG_DEBUG, "\n%s Box Correction before - PicNumber:%d, BoxType:%d",GetCurTime().c_str(), nPicNumber, nBoxType);
	for (size_t i = 0; i < boxNumberSet.size(); ++i)
	{
		syslog(LOG_DEBUG, "%s,%s,%s,%s,%d\n", boxNumberSet[i].strBoxNumber.c_str(),boxNumberSet[i].strBoxModel.c_str(),boxNumberSet[i].strBoxColor.c_str(), \
			boxNumberSet[i].strArrangement.c_str(),(int)boxNumberSet[i].direct);
	}
	syslog(LOG_DEBUG, "\n");
	// 4副图片 前、前右，后、后左识别结果
	// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
	//structBoxNumberRecogResultCorrected resultCorrected;
	if (boxNumberSet.size() <= 0)
	{
		resultCorrected.strFrontBoxNumber   = "";
		resultCorrected.strFrontBoxModel    = "";
		resultCorrected.strBackBoxNumber    = "";
		resultCorrected.strBackBoxModel     = "";
		resultCorrected.nPicNumber          = 0;
		resultCorrected.nBoxType            = 0;
		resultCorrected.strRecogResult		= "0";
		syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
		syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
			resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
		return 0;
	}
	int nRet1 = 0;
	int nRet2 = 0;
	int nRet3 = 0;
	int nRet4 = 0;
	int nRet5 = 0;
	int nRet6 = 0;
	char chVerifyCode1   = ' ';
	char chVerifyCode2   = ' ';
	char chVerifyCode3   = ' ';
	char chVerifyCode4   = ' ';
	char chVerifyCode5   = ' ';
	char chVerifyCode6   = ' ';    
	string strBoxNumber = "";
	BoxNumberCheckAlgo check;
	string strFront         = "";
	string strFrontRight    = "";
	string strBack          = "";
	string strBackLeft      = "";
	string strFrontLeft     = "";
	string strBackRight     = "";
	string strFrontBoxModel = "";
	string strBackBoxModel  = "";
	for (int i = 0; i < boxNumberSet.size(); ++i)
	{
		if (boxNumberSet[i].direct == FRONT)
		{
			strFront        = boxNumberSet[i].strBoxNumber;
			if (boxNumberSet[i].strBoxModel != "")
				strFrontBoxModel = boxNumberSet[i].strBoxModel;
		}
		if (boxNumberSet[i].direct == FRONTRIGHT)
		{
			strFrontRight   = boxNumberSet[i].strBoxNumber; 
			if (boxNumberSet[i].strBoxModel != "")
				strFrontBoxModel = boxNumberSet[i].strBoxModel;            
		}
		if (boxNumberSet[i].direct == BACK)
		{
			strBack        = boxNumberSet[i].strBoxNumber;
			if (boxNumberSet[i].strBoxModel != "")
				strBackBoxModel = boxNumberSet[i].strBoxModel;   
		}
		if (boxNumberSet[i].direct == BACKLEFT)
		{
			strBackLeft         =   boxNumberSet[i].strBoxNumber;   
			if (boxNumberSet[i].strBoxModel != "")
				strBackBoxModel = boxNumberSet[i].strBoxModel;            
		}
		if (boxNumberSet[i].direct == FRONTLEFT)
		{
			strFrontLeft   = boxNumberSet[i].strBoxNumber;
			if (boxNumberSet[i].strBoxModel != "")
				strFrontBoxModel = boxNumberSet[i].strBoxModel;            
		}
		if (boxNumberSet[i].direct == BACKRIGHT)
		{
			strBackRight   = boxNumberSet[i].strBoxNumber; 
			if (boxNumberSet[i].strBoxModel != "")
				strBackBoxModel = boxNumberSet[i].strBoxModel;             
		}
	}        
	if (nPicNumber == 4 && boxNumberSet.size() >= 4)
	{
		nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
		nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
		nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
		nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);
		if (nRet1 == 1)
		{
			resultCorrected.strFrontBoxNumber   = strFront;
			resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			resultCorrected.strBackBoxNumber    = "";
			resultCorrected.strBackBoxModel     = "";
			resultCorrected.nPicNumber          = 4;
			resultCorrected.nBoxType            = nBoxType;   
			resultCorrected.strRecogResult		= "1";
			syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
			syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
				resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
			return 1;
		}
		if (nRet2 == 1)
		{
			resultCorrected.strFrontBoxNumber   = strFrontRight;
			resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			resultCorrected.strBackBoxNumber    = "";
			resultCorrected.strBackBoxModel     = "";
			resultCorrected.nPicNumber          = 4;
			resultCorrected.nBoxType            = nBoxType;   
			resultCorrected.strRecogResult		= "1";
			syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
			syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
				resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
			return 1;
		}
		if (nRet3 == 1)
		{
			resultCorrected.strFrontBoxNumber   = strBackLeft;
			resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			resultCorrected.strBackBoxNumber    = "";
			resultCorrected.strBackBoxModel     = "";
			resultCorrected.nPicNumber          = 4;
			resultCorrected.nBoxType            = nBoxType;   
			resultCorrected.strRecogResult		= "1";
			syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
			syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
				resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
			return 1;
		}    
		if (nRet4 == 1)
		{
			resultCorrected.strFrontBoxNumber   = strBack;
			resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			resultCorrected.strBackBoxNumber    = "";
			resultCorrected.strBackBoxModel     = "";
			resultCorrected.nPicNumber          = 4;
			resultCorrected.nBoxType            = nBoxType;   
			resultCorrected.strRecogResult		= "1";
			syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
			syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
				resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
			return 1;
		}        
	}
	if (nPicNumber == 6 && boxNumberSet.size() >= 6)
	{
		nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
		nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
		nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
		nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4); 
		nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
		nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);   
		if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
		{
			if (nRet1 == 1)
			{
				resultCorrected.strFrontBoxNumber   = strFront;
				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			}
			else if (nRet2 == 1)
			{
				resultCorrected.strFrontBoxNumber   = strFrontRight; 
				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			}
			else if (nRet5 == 1)
			{
				resultCorrected.strFrontBoxNumber   = strFrontLeft; 
				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
			}

			
			if (nRet3 == 1)
			{
				resultCorrected.strBackBoxNumber    = strBackLeft;
				resultCorrected.strBackBoxModel     = strBackBoxModel;
			}
			else if (nRet4 == 1)
			{
				resultCorrected.strBackBoxNumber    = strBack;
				resultCorrected.strBackBoxModel     = strBackBoxModel;
			}
			else if (nRet6 == 1)
			{
				resultCorrected.strBackBoxNumber    = strBackRight;
				resultCorrected.strBackBoxModel     = strBackBoxModel;
			}
			
			resultCorrected.nPicNumber          = 6;
			resultCorrected.nBoxType            = nBoxType;   
			resultCorrected.strRecogResult		= "1";
			syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
			syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
				resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
			return 1;
		}
	}    
	//    // 有校验正确的么
	//    if (nRet1 == 1 && nRet2 == 1 && nRet3 == 1 && nRet4 == 1)
	//    {
	//        // never go here
	//        ; // 获取正确的识别结果、箱型、长短箱标志输出
	//    }
	//    else
	//    {
	//                       
	//    }
	if (nPicNumber == 4)
	{
		// 确定箱主代码
		vector<structContaOwnerData> contaOwnerVect;
		GetContaOwnerData(contaOwnerVect);
		bool bFind = false;
		for (int i = 0; i < contaOwnerVect.size(); ++i)
		{
			if (strFront.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strFrontRight.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strBack.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strBackLeft.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}             
		}
		// call interface
		// bFind = xxxxx;
		// 不存在箱主代码么
		if (!bFind)
		{
			// 修改易错字母、并重新校验- front
			string	strFrontTmp = strFront;
			vector<int> posVect;
			for (int i = 0; i < strFrontTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect.push_back(nFrontPos);
					string strSub 	= strFrontTmp.substr(nFrontPos + 1);
					strFrontTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontTmp = strFront;
			for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
			{
				if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
					strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
			}
			// modify frontright
			string	strFrontRightTmp = strFrontRight;
			vector<int> posVect2;
			for (int i = 0; i < strFrontRightTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontRightTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect2.push_back(nFrontPos);
					string strSub 	= strFrontRightTmp.substr(nFrontPos + 1);
					strFrontRightTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontRightTmp = strFrontRight;
			for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
			{
				if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
					strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
			}
			// modify backLeft
			string	strBackLeftTmp = strBackLeft;
			vector<int> posVect3;
			for (int i = 0; i < strBackLeftTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackLeftTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect3.push_back(nFrontPos);
					string strSub 	= strBackLeftTmp.substr(nFrontPos + 1);
					strBackLeftTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackLeftTmp = strBackLeft;
			for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
			{
				if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
					strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
			} 
			// modify back
			string	strBackTmp = strBack;
			vector<int> posVect4;
			for (int i = 0; i < strBackTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect4.push_back(nFrontPos);
					string strSub 	= strBackTmp.substr(nFrontPos + 1);
					strBackTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackTmp = strBackLeft;
			for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
			{
				if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
					strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
			}             
			// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			if (nPicNumber == 4 && boxNumberSet.size() >= 4)
			{
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
				nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
				nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
				nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
				if (nRet1 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strFrontTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;   
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
				if (nRet2 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strFrontRightTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;  
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
				if (nRet3 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strBackLeftTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;   
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}    
				if (nRet4 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strBackTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;   
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}                 
			}
			//            if (nPicNumber == 6 && boxNumberSet.size() >= 6)
			//            {
			//                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
			//                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
			//                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
			//                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);     
			//            } 
			// 有校验正确的么
			if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
			{
				// never go here
				// 获取正确的识别结果、箱型、长短箱标志输出
			}            
			else
			{
				// 从箱号中挑选出一个正确的校验位,再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				// 1. supposing strFront right
				strFrontRightTmp	= strFrontRight;
				strFrontTmp 		= strFront;
				if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
					strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
				nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
				// 2. supposing strFrontRight right
				strFrontRightTmp	= strFrontRight;
				strFrontTmp 		= strFront;
				if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
					strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);	
				// 3. supposing strBackTmp right
				strBackLeftTmp		= strBackLeft;
				strBackTmp 			= strBack;
				if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
					strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
				nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
				// 4. supposing strBackLeftTmp right
				strBackLeftTmp		= strBackLeft;
				strBackTmp 			= strBack;
				if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
					strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];								
				nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4); 
				// 有校验正确的么
				if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
				{
					// 获取正确的识别结果、箱型、长短箱标志输出
					if (nRet1 == 1)
					{
						resultCorrected.strFrontBoxNumber   = strFrontTmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType;  
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
					if (nRet2 == 1)
					{
						resultCorrected.strFrontBoxNumber   = strFrontRightTmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType;  
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
					if (nRet3 == 1)
					{
						resultCorrected.strFrontBoxNumber   = strBackLeftTmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType; 
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}    
					if (nRet4 == 1)
					{
						resultCorrected.strFrontBoxNumber   = strBackTmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					} 
				}                 
				else
				{
					// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
					// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
					if (nPicNumber == 4)
					{
						string str1	= strFront;
						string str2	= strFrontRight;
						string str3	= strBack;
						string str4	= strBackLeft;
						string str1tmp	= "";
						int nMixSize = 0;
						if (str1.size() > 0)
							nMixSize	= str1.size();
						if (str2.size() > 0)
							nMixSize	= str2.size();
						if (str3.size() > 0)
							nMixSize	= str3.size();
						if (str4.size() > 0)
							nMixSize	= str4.size();
						//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
						for (size_t i = 0; i < nMixSize; ++i)
						{
							map<char,int> statisticsMap;
							map<char,int>::iterator mapIter;
							if (str1.size() > 0 && i < str1.size())
							{
								char ch1	= str1[i];
								mapIter = statisticsMap.find(ch1);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch1, nCount));
								}
							}
							if (str2.size() > 0 && i < str2.size())
							{
								char ch2	= str2[i];
								mapIter = statisticsMap.find(ch2);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch2, nCount));
								}
							}
							if (str3.size() > 0 && i < str3.size())
							{
								char ch3	= str3[i];
								mapIter = statisticsMap.find(ch3);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch3, nCount));
								}
							}
							if (str4.size() > 0 && i < str4.size())
							{
								char ch4	= str4[i];
								mapIter = statisticsMap.find(ch4);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch4, nCount));
								}
							}	
							int nMax = -1;		
							char tmpCh = ' ';
							for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
							{			
								//if ((*it).second > nMax)
								//{
								//	nMax	= (*it).second;
								//	tmpCh	= (*it).first;
								//}
								if ((*it).second > nMax)
								{
									nMax	= (*it).second;
									if (it->first != '?')
										tmpCh	= (*it).first;
									else
										tmpCh	= '?';
								}
							}
							str1tmp += tmpCh;		
						}                   
						nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
						// 有校验正确的么
						if (nRet1 == 1)
						{
							// 获取正确的识别结果、箱型、长短箱标志输出
							resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							resultCorrected.strBackBoxNumber    = "";
							resultCorrected.strBackBoxModel     = "";
							resultCorrected.nPicNumber          = 4;
							resultCorrected.nBoxType            = nBoxType;   
							resultCorrected.strRecogResult		= "1";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 1;
						}                     
						else
						{
							// 通过算法算出校验位
							nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志
							resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							resultCorrected.strBackBoxNumber    = "";
							resultCorrected.strBackBoxModel     = "";
							resultCorrected.nPicNumber          = 4;
							resultCorrected.nBoxType            = nBoxType; 
							resultCorrected.strRecogResult		= "0";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 0;
						}                        
					}
					else if (nPicNumber == 6)
					{
						// never go to here
						string str1	= strFront;
						string str2	= strFrontRight;
						string str5	= strFrontLeft;
						string str3	= strBack;
						string str4	= strBackLeft;						
						string str6 = strBackRight;
						string str1tmp	= "";
						string str2tmp	= "";
						for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
						{
							map<char,int> statisticsMap;
							char ch1	= str1[i];
							map<char,int>::iterator mapIter = statisticsMap.find(ch1);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch1, nCount));
							}
							char ch2	= str2[i];
							mapIter = statisticsMap.find(ch2);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch2, nCount));
							}	
							char ch3	= str5[i];
							mapIter = statisticsMap.find(ch3);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch3, nCount));
							}	
							int nMax = 0;		
							char tmpCh = ' ';
							for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
							{			
								if ((*it).second > nMax)
								{
									nMax	= (*it).second;
									tmpCh	= (*it).first;
								}
							}
							str1tmp += tmpCh;		
						}                    
						for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
						{
							map<char,int> statisticsMap;
							char ch1	= str3[i];
							map<char,int>::iterator mapIter = statisticsMap.find(ch1);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch1, nCount));
							}
							char ch2	= str4[i];
							mapIter = statisticsMap.find(ch2);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch2, nCount));
							}	
							char ch3	= str6[i];
							mapIter = statisticsMap.find(ch3);
							if (mapIter != statisticsMap.end())
							{
								++(mapIter->second);
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch3, nCount));
							}	
							int nMax = 0;		
							char tmpCh = ' ';
							for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
							{			
								if ((*it).second > nMax)
								{
									nMax	= (*it).second;
									tmpCh	= (*it).first;
								}
							}
							str2tmp += tmpCh;		
						}  						
						nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
						nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
						// 有校验正确的么
						if (nRet1 == 1 && nRet2 == 1)
						{
							// 获取正确的识别结果、箱型、长短箱标志输出
						}
						else if (nRet1 == 1 && nRet2 == 0)
						{
							// 通过算法算出校验位
							nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志						
						}
						else if (nRet1 == 0 && nRet2 == 1)
						{
							// 通过算法算出校验位
							nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志						
						}
						else
						{
							// 通过算法算出校验位
							nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志
						}                         
					}
				}
			}
		}
		else
		{/*
		 // 从箱号中挑选出一个正确的校验位,再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 通过算法算出校验位
		 // 产生不确定的箱号、箱型、长短箱标志
		 }
		 }*/
			// 从箱号中挑选出一个正确的校验位,再次进行校验
			// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			// 1. supposing strFront right
			string strFrontRightTmp	= strFrontRight;
			string strFrontTmp 	= strFront;
			if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
				strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
			nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
			// 2. supposing strFrontRight right
			strFrontRightTmp	= strFrontRight;
			strFrontTmp 		= strFront;
			if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
				strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
			nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);	
			// 3. supposing strBackTmp right
			string strBackLeftTmp		= strBackLeft;
			string strBackTmp 			= strBack;
			if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
				strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
			nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
			// 4. supposing strBackLeftTmp right
			strBackLeftTmp		= strBackLeft;
			strBackTmp 			= strBack;
			if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
				strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];								
			nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4); 
			// 有校验正确的么
			if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
			{
				// 获取正确的识别结果、箱型、长短箱标志输出
				if (nRet1 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strFrontTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;  
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
				if (nRet2 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strFrontRightTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;  
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
				if (nRet3 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strBackLeftTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType; 
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}    
				if (nRet4 == 1)
				{
					resultCorrected.strFrontBoxNumber   = strBackTmp;
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					resultCorrected.strBackBoxNumber    = "";
					resultCorrected.strBackBoxModel     = "";
					resultCorrected.nPicNumber          = 4;
					resultCorrected.nBoxType            = nBoxType;   
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				} 
			}                 
			else
			{
				// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				if (nPicNumber == 4)
				{
					string str1	= strFront;
					string str2	= strFrontRight;
					string str3	= strBack;
					string str4	= strBackLeft;
					string str1tmp	= "";
					int nMixSize = 0;
					if (str1.size() > 0)
						nMixSize	= str1.size();
					if (str2.size() > 0)
						nMixSize	= str2.size();
					if (str3.size() > 0)
						nMixSize	= str3.size();
					if (str4.size() > 0)
						nMixSize	= str4.size();
					//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
					for (size_t i = 0; i < nMixSize; ++i)
					{
						map<char,int> statisticsMap;
						map<char,int>::iterator mapIter;
						if (str1.size() > 0 && i < str1.size())
						{
							char ch1	= str1[i];
							mapIter = statisticsMap.find(ch1);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch1, nCount));
							}
						}
						if (str2.size() > 0 && i < str2.size())
						{
							char ch2	= str2[i];
							mapIter = statisticsMap.find(ch2);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch2, nCount));
							}
						}
						if (str3.size() > 0 && i < str3.size())
						{
							char ch3	= str3[i];
							mapIter = statisticsMap.find(ch3);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch3, nCount));
							}
						}
						if (str4.size() > 0 && i < str4.size())
						{
							char ch4	= str4[i];
							mapIter = statisticsMap.find(ch4);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch4, nCount));
							}
						}	
						int nMax = -1;		
						char tmpCh = ' ';
						for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
						{			
							if ((*it).second > nMax)
							{
								nMax	= (*it).second;
								if (it->first != '?')
									tmpCh	= (*it).first;
								else
									tmpCh	= '?';
							}
						}
						str1tmp += tmpCh;		
					}                    
					nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
					// 有校验正确的么
					if (nRet1 == 1)
					{
						// 获取正确的识别结果、箱型、长短箱标志输出
						resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}                     
					else
					{
						// 通过算法算出校验位
						nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
						if (str1tmp.size() == 11 && str1tmp[10] == '?')
							str1tmp[10]	= chVerifyCode1;
						int nTmpIndex = 0;
						for (int i = 4; i < str1tmp.size(); ++i)
						{
							if (str1tmp[i] == '?')
							{
								nTmpIndex = i;
								break;
							}
						}
						if (nTmpIndex != 0)
						{
							for (int i = 0; i < 10; ++i)
							{
								str1tmp[nTmpIndex]	= '0' + i;
								nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
								if (nRet1 == 1)
								{
									break;
								}
							}
						}
						nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
						if (str1tmp.size() == 11 && str1tmp[10] != chVerifyCode1)
						{
							str1tmp[10] = chVerifyCode1;
						}
						// 产生不确定的箱号、箱型、长短箱标志
						resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = "";
						resultCorrected.strBackBoxModel     = "";
						resultCorrected.nPicNumber          = 4;
						resultCorrected.nBoxType            = nBoxType; 
						resultCorrected.strRecogResult		= "0";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 0;
					}                        
				}
				else if (nPicNumber == 6)
				{
					// never go to here
					//string str1	= strFront;
					//string str2	= strFrontRight;
					//string str5	= strFrontLeft;
					//string str3	= strBack;
					//string str4	= strBackLeft;						
					//string str6 = strBackRight;
					//string str1tmp	= "";
					//string str2tmp	= "";
					//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
					//{
					//	map<char,int> statisticsMap;
					//	char ch1	= str1[i];
					//	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch1, nCount));
					//	}
					//	char ch2	= str2[i];
					//	mapIter = statisticsMap.find(ch2);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch2, nCount));
					//	}	
					//	char ch3	= str5[i];
					//	mapIter = statisticsMap.find(ch3);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch3, nCount));
					//	}	
					//	int nMax = 0;		
					//	char tmpCh = ' ';
					//	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
					//	{			
					//		if ((*it).second > nMax)
					//		{
					//			nMax	= (*it).second;
					//			tmpCh	= (*it).first;
					//		}
					//	}
					//	str1tmp += tmpCh;		
					//}                    
					//for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
					//{
					//	map<char,int> statisticsMap;
					//	char ch1	= str3[i];
					//	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch1, nCount));
					//	}
					//	char ch2	= str4[i];
					//	mapIter = statisticsMap.find(ch2);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch2, nCount));
					//	}	
					//	char ch3	= str6[i];
					//	mapIter = statisticsMap.find(ch3);
					//	if (mapIter != statisticsMap.end())
					//	{
					//		++(mapIter->second);
					//	}
					//	else
					//	{
					//		int nCount = 1;
					//		statisticsMap.insert(make_pair(ch3, nCount));
					//	}	
					//	int nMax = 0;		
					//	char tmpCh = ' ';
					//	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
					//	{			
					//		if ((*it).second > nMax)
					//		{
					//			nMax	= (*it).second;
					//			tmpCh	= (*it).first;
					//		}
					//	}
					//	str2tmp += tmpCh;		
					//}  						
					//nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
					//nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
					//// 有校验正确的么
					//if (nRet1 == 1 && nRet2 == 1)
					//{
					//	// 获取正确的识别结果、箱型、长短箱标志输出
					//}
					//else if (nRet1 == 1 && nRet2 == 0)
					//{
					//	// 通过算法算出校验位
					//	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
					//	// 产生不确定的箱号、箱型、长短箱标志						
					//}
					//else if (nRet1 == 0 && nRet2 == 1)
					//{
					//	// 通过算法算出校验位
					//	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
					//	// 产生不确定的箱号、箱型、长短箱标志						
					//}
					//else
					//{
					//	// 通过算法算出校验位
					//	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
					//	// 产生不确定的箱号、箱型、长短箱标志
					//}                         
				}
			}
		}          
	}
	else if (nPicNumber == 6)
	{
		// 确定箱主代码
		vector<structContaOwnerData> contaOwnerVect;
		GetContaOwnerData(contaOwnerVect);
		bool bFrontFind = false;
		bool bBackFind	= false;
		for (int i = 0; i < contaOwnerVect.size(); ++i)
		{
			if (strFront.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFrontFind = true;
				break;
			}
			if (strFrontRight.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFrontFind = true;
				break;
			}
		}
		for (int i = 0; i < contaOwnerVect.size(); ++i)
		{
			if (strBack.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bBackFind = true;
				break;
			}
			if (strBackLeft.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bBackFind = true;
				break;
			}
		}
		// call interface
		// bFind = xxxxx;
		// 不存在箱主代码么
		if (!bFrontFind || !bBackFind)
		{
			// 修改易错字母、并重新校验- front
			string	strFrontTmp = strFront;
			vector<int> posVect;
			for (int i = 0; i < strFrontTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect.push_back(nFrontPos);
					string strSub 	= strFrontTmp.substr(nFrontPos + 1);
					strFrontTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontTmp = strFront;
			for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
			{
				if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
				{
					if (strFrontRight[posVect[i]] != '?')
						strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
				}
			}
			strFrontTmp = strFront;
			for (int i = 0; i < posVect.size() && i < strFrontLeft.size(); ++i)
			{
				if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontLeft.size())
				{
					if (strFrontLeft[posVect[i]] != '?')
						strFrontTmp[posVect[i]] = strFrontLeft[posVect[i]];
				}
			}
			// modify frontright
			string	strFrontRightTmp = strFrontRight;
			vector<int> posVect2;
			for (int i = 0; i < strFrontRightTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontRightTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect2.push_back(nFrontPos);
					string strSub 	= strFrontRightTmp.substr(nFrontPos + 1);
					strFrontRightTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontRightTmp = strFrontRight;
			for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
			{
				if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
				{
					if (strFront[posVect[i]] != '?')
						strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
				}
			}
			strFrontRightTmp = strFrontRight;
			for (int i = 0; i < posVect2.size() && i < strFrontLeft.size(); ++i)
			{
				if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFrontLeft.size())
				{
					if (strFrontLeft[posVect[i]] != '?')
						strFrontRightTmp[posVect2[i]] = strFrontLeft[posVect2[i]];
				}
			}
			// modify frontleft
			string	strFrontLeftTmp = strFrontLeft;
			vector<int> posVect5;
			for (int i = 0; i < strFrontLeftTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontLeftTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect5.push_back(nFrontPos);
					string strSub 	= strFrontLeftTmp.substr(nFrontPos + 1);
					strFrontLeftTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontLeftTmp = strFrontLeft;
			for (int i = 0; i < posVect5.size() && i < strFront.size(); ++i)
			{
				if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFront.size())
				{
					if (strFront[posVect[i]] != '?')
						strFrontLeftTmp[posVect5[i]] = strFront[posVect5[i]];
				}
			}
			strFrontLeftTmp = strFrontLeft;
			for (int i = 0; i < posVect5.size() && i < strFrontRight.size(); ++i)
			{
				if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFrontRight.size())
				{
					if (strFrontRight[posVect[i]] != '?')
						strFrontLeftTmp[posVect5[i]] = strFrontRight[posVect5[i]];
				}
			}
			// modify backLeft
			string	strBackLeftTmp = strBackLeft;
			vector<int> posVect3;
			for (int i = 0; i < strBackLeftTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackLeftTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect3.push_back(nFrontPos);
					string strSub 	= strBackLeftTmp.substr(nFrontPos + 1);
					strBackLeftTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackLeftTmp = strBackLeft;
			for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
			{
				if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
				{
					if (strBack[posVect3[i]] != '?')
						strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
				}
			} 
			strBackLeftTmp = strBackLeft;
			for (int i = 0; i < posVect3.size() && i < strBackRight.size(); ++i)
			{
				if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBackRight.size())
				{
					if (strBackRight[posVect3[i]] != '?')
						strBackLeftTmp[posVect3[i]] = strBackRight[posVect3[i]];
				}
			} 
			// modify back
			string	strBackTmp = strBack;
			vector<int> posVect4;
			for (int i = 0; i < strBackTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect4.push_back(nFrontPos);
					string strSub 	= strBackTmp.substr(nFrontPos + 1);
					strBackTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackTmp = strBack;
			for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
			{
				if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
				{
					if (strBackLeft[posVect4[i]] != '?')
						strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
				}
			}    
			strBackTmp = strBack;
			for (int i = 0; i < posVect4.size() && i < strBackRight.size(); ++i)
			{
				if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackRight.size())
				{
					if (strBackRight[posVect4[i]] != '?')
						strBackTmp[posVect4[i]] = strBackRight[posVect4[i]];
				}
			}      
			// modify backRight
			string	strBackRightTmp = strBackRight;
			vector<int> posVect6;
			for (int i = 0; i < strBackRightTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackRightTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect6.push_back(nFrontPos);
					string strSub 	= strBackRightTmp.substr(nFrontPos + 1);
					strBackRightTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackRightTmp = strBackRight;
			for (int i = 0; i < posVect6.size() && i < strBack.size(); ++i)
			{
				if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBack.size())
				{
					if (strBack[posVect6[i]] != '?')
						strBackRightTmp[posVect6[i]] = strBack[posVect6[i]];
				}
			} 
			strBackRightTmp = strBackRight;
			for (int i = 0; i < posVect6.size() && i < strBackLeft.size(); ++i)
			{
				if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBackLeft.size())
				{
					if (strBackLeft[posVect6[i]] != '?')
						strBackRightTmp[posVect6[i]] = strBackLeft[posVect6[i]];
				}
			} 
			// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			if (nPicNumber == 6) //  && boxNumberSet.size() >= 6)	//2015-2-11
			{
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
				nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
				nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
				nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);  
				nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
				nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
			} 
			// 有校验正确的么
			if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
			{
				// 获取正确的识别结果、箱型、长短箱标志输出
				if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
				{
					if (nRet1 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontTmp;
					else if (nRet2 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontRightTmp; 
					else if (nRet5 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontLeftTmp; 
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					if (nRet3 == 1)
						resultCorrected.strBackBoxNumber    = strBackLeftTmp;
					else if (nRet4 == 1)
						resultCorrected.strBackBoxNumber    = strBackTmp;
					else if (nRet6 == 1)
						resultCorrected.strBackBoxNumber    = strBackRightTmp;
					resultCorrected.strBackBoxModel     = strBackBoxModel;
					resultCorrected.nPicNumber          = 6;
					resultCorrected.nBoxType            = nBoxType;   
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
			}            
			else
			{
				// 从箱号中挑选出一个正确的校验位,再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				// 1. supposing strFront right
				strFrontRightTmp	= strFrontRight;
				strFrontTmp 		= strFront;
				strFrontLeftTmp		= strFrontLeft;
				if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
					strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
				nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
				if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
					strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
				nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
				// 2. supposing strFrontRight right
				strFrontRightTmp	= strFrontRight;
				strFrontTmp 		= strFront;
				strFrontLeftTmp		= strFrontLeft;
				if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
					strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
				if (nRet5 == 0)
				{
					if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
						strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
					nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
				}
				// 3. supposing strBackTmp right
				strBackLeftTmp		= strBackLeft;
				strBackTmp 			= strBack;
				strBackRightTmp		= strBackRight;
				if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
					strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
				nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
				if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
					strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
				nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
				// 4. supposing strBackLeftTmp right
				strBackLeftTmp		= strBackLeft;
				strBackTmp 			= strBack;
				strBackRightTmp		= strBackRight;
				if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
					strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];								
				nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
				if (nRet6 == 0)
				{
					if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
						strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
					nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
				}
				// 4. supposing strBackRightTmp right
				strBackLeftTmp		= strBackLeft;
				strBackTmp 			= strBack;
				strBackRightTmp		= strBackRight;
				if (nRet3 == 0)
				{
					if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
						strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];								
					nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
				}
				if (nRet4 == 0)
				{
					if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
						strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
					nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
				}
				// 有校验正确的么
				if ((nRet1 == 1 || nRet2 == 1 || nRet5	== 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
				{
					// 获取正确的识别结果、箱型、长短箱标志输出
					if ((nRet1 == 1 || nRet2 == 1 || nRet5	== 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
					{
						if (nRet1 == 1)
							resultCorrected.strFrontBoxNumber   = strFrontTmp;
						else if (nRet2 == 1)
							resultCorrected.strFrontBoxNumber   = strFrontRightTmp; 
						else if (nRet5 == 1)
							resultCorrected.strFrontBoxNumber   = strFrontLeftTmp; 
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						if (nRet3 == 1)
							resultCorrected.strBackBoxNumber    = strBackLeftTmp;
						else if (nRet4 == 1)
							resultCorrected.strBackBoxNumber    = strBackTmp;
						else if (nRet6 == 1)
							resultCorrected.strBackBoxNumber    = strBackRightTmp;
						resultCorrected.strBackBoxModel     = strBackBoxModel;
						resultCorrected.nPicNumber          = 6;
						resultCorrected.nBoxType            = nBoxType;  
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
				}                 
				else
				{
					// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
					// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
					if (nPicNumber == 6)
					{
						string str1	= strFront;
						string str2	= strFrontRight;
						string str5	= strFrontLeft;
						string str3	= strBack;
						string str4	= strBackLeft;						
						string str6 = strBackRight;
						string str1tmp	= "";
						string str2tmp	= "";
						int nMixSize = 0;
						if (str1.size() > 0)
							nMixSize	= str1.size();
						if (str2.size() > 0)
							nMixSize	= str2.size();
						if (str5.size() > 0)
							nMixSize	= str5.size();
						//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
						for (size_t i = 0; i < nMixSize; ++i)
						{
							map<char,int> statisticsMap;
							map<char,int>::iterator mapIter;
							if (str1.size() > 0 && i < str1.size())
							{
								char ch1	= str1[i];
								mapIter = statisticsMap.find(ch1);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch1, nCount));
								}
							}
							if (str2.size() > 0 && i < str2.size())
							{
								char ch2	= str2[i];
								mapIter = statisticsMap.find(ch2);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch2, nCount));
								}
							}
							if (str5.size() > 0 && i < str5.size())
							{
								char ch5	= str5[i];
								mapIter = statisticsMap.find(ch5);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch5, nCount));
								}
							}	
							int nMax = -1;		
							char tmpCh = ' ';
							for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
							{			
								if ((*it).second > nMax)
								{
									nMax	= (*it).second;
									if (it->first != '?')
										tmpCh	= (*it).first;
									else
										tmpCh	= '?';
								}
							}
							str1tmp += tmpCh;		
						}    
						nMixSize = 0;
						if (str3.size() > 0)
							nMixSize	= str3.size();
						if (str4.size() > 0)
							nMixSize	= str4.size();
						if (str6.size() > 0)
							nMixSize	= str6.size();
						for (size_t i = 0; i < nMixSize; ++i)
						{
							map<char,int> statisticsMap;
							map<char,int>::iterator mapIter;
							if (str3.size() > 0 && i < str3.size())
							{
								char ch3	= str3[i];
								mapIter = statisticsMap.find(ch3);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch3, nCount));
								}
							}
							if (str4.size() > 0 && i < str4.size())
							{
								char ch4	= str4[i];
								mapIter = statisticsMap.find(ch4);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch4, nCount));
								}
							}
							if (str6.size() > 0 && i < str6.size())
							{
								char ch6	= str6[i];
								mapIter = statisticsMap.find(ch6);
								if (mapIter != statisticsMap.end() && mapIter->first != '?')
								{
									++(mapIter->second);
								}
								else if (mapIter != statisticsMap.end() && mapIter->first == '?')
								{
									mapIter->second = 0;
								}
								else
								{
									int nCount = 1;
									statisticsMap.insert(make_pair(ch6, nCount));
								}
							}	
							int nMax = -1;		
							char tmpCh = ' ';
							for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
							{			
								if ((*it).second > nMax)
								{
									nMax	= (*it).second;
									if (it->first != '?')
										tmpCh	= (*it).first;
									else
										tmpCh	= '?';
								}
							}
							str2tmp += tmpCh;		
						}  						
						nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
						nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
						// 有校验正确的么
						if (nRet1 == 1 && nRet2 == 1)
						{
							// 获取正确的识别结果、箱型、长短箱标志输出
							if (nRet1 == 1)
								resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							if (nRet2 == 1)
								resultCorrected.strBackBoxNumber    = str2tmp;
							resultCorrected.strBackBoxModel     = strBackBoxModel;
							resultCorrected.nPicNumber          = 6;
							resultCorrected.nBoxType            = nBoxType;   
							resultCorrected.strRecogResult		= "1";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 1;
						}
						else if (nRet1 == 1 && nRet2 == 0)
						{
							// 通过算法算出校验位
							//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志						
							if (nRet1 == 1)
								resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							resultCorrected.strBackBoxNumber    = str2tmp;
							resultCorrected.strBackBoxModel     = strBackBoxModel;
							resultCorrected.nPicNumber          = 6;
							resultCorrected.nBoxType            = nBoxType;   
							resultCorrected.strRecogResult		= "0";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 1;
						}
						else if (nRet1 == 0 && nRet2 == 1)
						{
							// 通过算法算出校验位
							//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志						
							resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							resultCorrected.strBackBoxNumber    = str2tmp;
							resultCorrected.strBackBoxModel     = strBackBoxModel;
							resultCorrected.nPicNumber          = 6;
							resultCorrected.nBoxType            = nBoxType;   
							resultCorrected.strRecogResult		= "0";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 1;
						}
						else
						{
							// 通过算法算出校验位
							//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
							// 产生不确定的箱号、箱型、长短箱标志
							resultCorrected.strFrontBoxNumber   = str1tmp;
							resultCorrected.strFrontBoxModel    = strFrontBoxModel;
							resultCorrected.strBackBoxNumber    = str2tmp;
							resultCorrected.strBackBoxModel     = strBackBoxModel;
							resultCorrected.nPicNumber          = 6;
							resultCorrected.nBoxType            = nBoxType;   
							resultCorrected.strRecogResult		= "0";
							syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
							syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
								resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
							return 1;
						}                         
					}
				}
			}
		}
		else
		{/*
		 // 从箱号中挑选出一个正确的校验位,再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 通过算法算出校验位
		 // 产生不确定的箱号、箱型、长短箱标志
		 }
		 }*/
			// 从箱号中挑选出一个正确的校验位,再次进行校验
			// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			string strFrontRightTmp	= "";
			string strFrontTmp		= "";
			string strFrontLeftTmp	= "";
			string strBackLeftTmp	= "";
			string strBackTmp		= "";
			string strBackRightTmp	= "";
			nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
			nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
			nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

			nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
			nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
			nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
			//// 1. supposing strFront right
			//strFrontRightTmp	= strFrontRight;
			//strFrontTmp 		= strFront;
			//strFrontLeftTmp		= strFrontLeft;
			//if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
			//	strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
			//nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
			//if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
			//	strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
			//nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

			//// 2. supposing strFrontRight right
			//strFrontRightTmp	= strFrontRight;
			//strFrontTmp 		= strFront;
			//strFrontLeftTmp		= strFrontLeft;
			//if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
			//	strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
			//nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
			//if (nRet5 == 0)
			//{
			//	if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
			//		strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
			//	nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
			//}
			//// 3. supposing strBackTmp right
			//strBackLeftTmp		= strBackLeft;
			//strBackTmp 			= strBack;
			//strBackRightTmp		= strBackRight;
			//if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
			//	strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
			//nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
			//if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
			//	strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
			//nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
			//// 4. supposing strBackLeftTmp right
			//strBackLeftTmp		= strBackLeft;
			//strBackTmp 			= strBack;
			//strBackRightTmp		= strBackRight;
			//if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
			//	strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];								
			//nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
			//if (nRet6 == 0)
			//{
			//	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
			//		strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
			//	nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
			//}
			//// 4. supposing strBackRightTmp right
			//strBackLeftTmp		= strBackLeft;
			//strBackTmp 			= strBack;
			//strBackRightTmp		= strBackRight;
			//if (nRet3 == 0)
			//{
			//	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
			//		strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];								
			//	nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
			//}
			//if (nRet4 == 0)
			//{
			//	if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
			//		strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
			//	nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
			//}
			// 有校验正确的么
			if ((nRet1 == 1 || nRet2 == 1 || nRet5	== 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
			{
				// 获取正确的识别结果、箱型、长短箱标志输出
				if ((nRet1 == 1 || nRet2 == 1 || nRet5	== 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
				{
					if (nRet1 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontTmp;
					else if (nRet2 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontRightTmp; 
					else if (nRet5 == 1)
						resultCorrected.strFrontBoxNumber   = strFrontLeftTmp; 
					resultCorrected.strFrontBoxModel    = strFrontBoxModel;
					if (nRet3 == 1)
						resultCorrected.strBackBoxNumber    = strBackLeftTmp;
					else if (nRet4 == 1)
						resultCorrected.strBackBoxNumber    = strBackTmp;
					else if (nRet6 == 1)
						resultCorrected.strBackBoxNumber    = strBackRightTmp;
					resultCorrected.strBackBoxModel     = strBackBoxModel;
					resultCorrected.nPicNumber          = 6;
					resultCorrected.nBoxType            = nBoxType;  
					resultCorrected.strRecogResult		= "1";
					syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
					syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
						resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
					return 1;
				}
			}                 
			else
			{
				// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				if (nPicNumber == 6)
				{
					string str1	= strFront;
					string str2	= strFrontRight;
					string str5	= strFrontLeft;
					string str3	= strBack;
					string str4	= strBackLeft;						
					string str6 = strBackRight;
					string str1tmp	= "";
					string str2tmp	= "";
					int nMixSize = 0;
					if (str1.size() > 0)
						nMixSize	= str1.size();
					if (str2.size() > 0)
						nMixSize	= str2.size();
					if (str5.size() > 0)
						nMixSize	= str5.size();
					//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
					for (size_t i = 0; i < nMixSize; ++i)
					{
						map<char,int> statisticsMap;
						map<char,int>::iterator mapIter;
						if (str1.size() > 0 && i < str1.size())
						{
							char ch1	= str1[i];
							mapIter = statisticsMap.find(ch1);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch1, nCount));
							}
						}
						if (str2.size() > 0 && i < str2.size())
						{
							char ch2	= str2[i];
							mapIter = statisticsMap.find(ch2);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch2, nCount));
							}
						}
						if (str5.size() > 0 && i < str5.size())
						{
							char ch5	= str5[i];
							mapIter = statisticsMap.find(ch5);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch5, nCount));
							}
						}	
						int nMax = -1;		
						char tmpCh = ' ';
						for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
						{			
							if ((*it).second > nMax)
							{
								nMax	= (*it).second;
								if (it->first != '?')
									tmpCh	= (*it).first;
								else
									tmpCh	= '?';
							}
						}
						str1tmp += tmpCh;		
					}    
					nMixSize = 0;
					if (str3.size() > 0)
						nMixSize	= str3.size();
					if (str4.size() > 0)
						nMixSize	= str4.size();
					if (str6.size() > 0)
						nMixSize	= str6.size();
					for (size_t i = 0; i < nMixSize; ++i)
					{
						map<char,int> statisticsMap;
						map<char,int>::iterator mapIter;
						if (str3.size() > 0 && i < str3.size())
						{
							char ch3	= str3[i];
							mapIter = statisticsMap.find(ch3);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch3, nCount));
							}
						}
						if (str4.size() > 0 && i < str4.size())
						{
							char ch4	= str4[i];
							mapIter = statisticsMap.find(ch4);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch4, nCount));
							}
						}
						if (str6.size() > 0 && i < str6.size())
						{
							char ch6	= str6[i];
							mapIter = statisticsMap.find(ch6);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch6, nCount));
							}
						}	
						int nMax = -1;		
						char tmpCh = ' ';
						for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
						{			
							if ((*it).second > nMax)
							{
								nMax	= (*it).second;
								if (it->first != '?')
									tmpCh	= (*it).first;
								else
									tmpCh	= '?';
							}
						}
						str2tmp += tmpCh;		
					}  						
					nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
					nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
					// 有校验正确的么
					if (nRet1 == 1 && nRet2 == 1)
					{
						// 获取正确的识别结果、箱型、长短箱标志输出
						if (nRet1 == 1)
							resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						if (nRet2 == 1)
							resultCorrected.strBackBoxNumber    = str2tmp;
						resultCorrected.strBackBoxModel     = strBackBoxModel;
						resultCorrected.nPicNumber          = 6;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "1";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
					else if (nRet1 == 1 && nRet2 == 0)
					{
						// 通过算法算出校验位
						//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
						// 产生不确定的箱号、箱型、长短箱标志						
						if (nRet1 == 1)
							resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = str2tmp;
						resultCorrected.strBackBoxModel     = strBackBoxModel;
						resultCorrected.nPicNumber          = 6;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "0";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
					else if (nRet1 == 0 && nRet2 == 1)
					{
						// 通过算法算出校验位
						//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
						// 产生不确定的箱号、箱型、长短箱标志						
						resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = str2tmp;
						resultCorrected.strBackBoxModel     = strBackBoxModel;
						resultCorrected.nPicNumber          = 6;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "0";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}
					else
					{
						// 通过算法算出校验位
						//nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
						// 产生不确定的箱号、箱型、长短箱标志
						resultCorrected.strFrontBoxNumber   = str1tmp;
						resultCorrected.strFrontBoxModel    = strFrontBoxModel;
						resultCorrected.strBackBoxNumber    = str2tmp;
						resultCorrected.strBackBoxModel     = strBackBoxModel;
						resultCorrected.nPicNumber          = 6;
						resultCorrected.nBoxType            = nBoxType;   
						resultCorrected.strRecogResult		= "0";
						syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
						syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
							resultCorrected.strBackBoxNumber.c_str(),resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType,__LINE__);
						return 1;
					}                         
				}
			}
		}          
	} 
	return 0;
}

void ProcessImage::GetContaOwnerData(std::vector<structContaOwnerData> &contaOwnerVect)
{
	int i = 0;
	char szLine[256] = {0};
	FILE *pFile = fopen("/etc/ContaOwnerData.txt", "r");
	if (pFile == NULL)
	{
		return ;
	}
	else
	{
		do 
		{
			memset(szLine, 0x00, sizeof(szLine));
			if (fgets(szLine, 255, pFile) == NULL)
			{
				break;
			}
			else
			{
				if (szLine[0] == '\n')
				{
					memset(szLine, 0x00, sizeof(szLine));
					continue;
				}
				char *p = NULL;
				char *pStart = szLine;
				if (pStart != NULL)
					p = strchr(pStart, ',');
				else
					break;
				if (p != NULL)
				{
					structContaOwnerData tmpData;
					*p = '\0';
					if (i == 0)
					{
						tmpData.nLineNo = atoi(p - 1);
					}
					else
					{
						tmpData.nLineNo = atoi(pStart);
					}
					pStart = p + 1;
					p = strchr(pStart, ',');
					if (p != NULL)
					{
						char *pTemp = NULL;
						pTemp = strchr(pStart, ',');
						if (pTemp != NULL)
						{
							*pTemp = '\0';
							tmpData.strCompanyCode = pStart;
						}
						*p = '\0';
						pStart = p + 1;
						p = strchr(pStart, '\n');
						if (p != NULL)
						{
							*p = '\0';
							tmpData.strCompanyName = pStart;
						}
					}
					contaOwnerVect.push_back(tmpData);
					//printf("%d, %s, %s\n", tmpData.nLineNo, tmpData.strCompanyCode.c_str(), tmpData.strCompanyName.c_str());
				}
				i++;
			}
		} while(1);
	}
	fclose(pFile);
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

