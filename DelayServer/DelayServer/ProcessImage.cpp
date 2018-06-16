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

int g_nRisingLever	= 0;
int g_nOffLever		= 0;
sem	g_sem;
int g_nStop			= 0;

locker g_stopLock;

#define MAX_HOST_DATA 4096

extern pthread_mutex_t              g_ImageDataMutex;
extern pthread_cond_t               g_ImageDataCond;



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
	//pthread_create (&controllerThreadID, NULL, ProcessControllerThread, NULL);
	pthread_create (&boxNumberThreadID, NULL, ProcessBoxNumberThread, NULL);
	return;
}

void *ProcessImage::ProcessControllerThread(void* pParam)
{
	//ProcessImage *pThis = (ProcessImage*)pParam;
	//pThis->ProcessControllerProc();
	return 0; 
}

void *ProcessImage::ProcessBoxNumberThread(void* pParam)
{
	ProcessImage *pThis = (ProcessImage*)pParam;
	pThis->ProcessBoxNumberProc();
	return 0;
}

void ProcessImage::ProcessBoxNumberProc()
{
		int nResult = 0;
		// test code
		while (!GlobalStopFlag)
		{

			nResult = g_sem.wait();
			g_stopLock.lock();
			g_nStop = 0;
			g_stopLock.unlock();

			int nCount = 6;
			int nSpan  = CSimpleConfig::m_nDelayTime / 6;
			if (CSimpleConfig::m_nDelayTime > 0)
			{
				while (nCount)
				{
					SysSleep(nSpan);
					nCount--;

					g_stopLock.lock();
					if (g_nStop == 1)
					{
						g_nStop = 0;
						g_stopLock.unlock();
						break;
					}
					g_stopLock.unlock();
				}
			}


			//if (CSimpleConfig::m_nDelayTime > 0)
			//	SysSleep(CSimpleConfig::m_nDelayTime);

			if (nCount <= 0)
			{
				syslog(LOG_DEBUG, "===============EC_FINISHED_DELAY=============\n");

				JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
				for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
				{
					if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "2") == 0)
					{
						strcpy(pUserData->sequence_no, "2");
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

			//syslog(LOG_DEBUG, "============================\n");
			//int ch = getchar();
			//char buffer[2] = {0};
			//buffer[0] = (char)ch;
			//if (ch == 'r')
			//{
			//	pthread_mutex_lock (&g_ImageDataMutex);      
			//	g_nRisingLever	= 1;
			//	pthread_mutex_unlock (&g_ImageDataMutex);
			//}
			//else if (ch == 'o')
			//{
			//	pthread_mutex_lock (&g_ImageDataMutex); 
			//	g_nOffLever		= 1;
			//	pthread_mutex_unlock (&g_ImageDataMutex);
			//}
		
			//syslog(LOG_DEBUG, "============================\n");
			//SysSleep(3000);
		}
		return;
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
		if (g_nRisingLever == 1 || g_nOffLever == 1)
		{

		}
		else
		{
			SysSleep(200);
			continue;
		}
		int nNewSocket = CreateSocket();
		if (nNewSocket == -1)
		{
			//	不能建立套接字，直接返回
			syslog(LOG_DEBUG, "Can't create socket\n");
			sleep(5);
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
			sleep(5);
			continue;
			//EndClient(pParam);
		}
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
		//vector<BYTE> dataVect;
		//rtu.WriteRegisterRequest(dataVect);
		//memcpy(pBuffer, &dataVect[0], dataVect.size());
		//nLen = dataVect.size();
		//nNewLen = SocketWrite(nNewSocket,pBuffer,nLen,30);
		//printf("write nlen=%d\n", nNewLen);
		//printf("%s Send Data:", GetCurTime().c_str());
		//for (int i = 0; i < nNewLen; ++i)
		//{
		//	printf("%02X ", dataVect[i]);
		//}
		//printf("\n");
		//SetSocketNotBlock(nNewSocket);
		//while(!GlobalStopFlag && !nSocketErrorFlag)

		{
			if (g_nRisingLever > 0 || g_nOffLever > 0)
			{
				char szSendData[50 + 1] = {0};
				vector<BYTE> readDataVect;
				rtu.WriteRegisterRequest(readDataVect, 0x01);
				memcpy(szSendData, &readDataVect[0], readDataVect.size());
				int nLen2		= readDataVect.size();
				int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
				syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
				char szTmpData[5 + 1] = {0};
				string strLogData = "Send Data:";
				for (int i = 0; i < nNewLen2; ++i)
				{
					memset(szTmpData, 0x00, 5);
					sprintf(szTmpData, "%02X ", (BYTE)szSendData[i]);
					strLogData += szTmpData;
				}
				syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
				if (nNewLen2 < 0)	//	断开
				{
					CloseSocket(nNewSocket);
					readDataVect.clear();
					continue;
				}
				else
				{
					;
				}			
			}

			//// g_nOffLever
			//if (g_nOffLever > 0)
			//{
			//	char szSendData[50 + 1] = {0};
			//	vector<BYTE> readDataVect;
			//	rtu.WriteRegisterRequest(readDataVect, 0x01);
			//	memcpy(szSendData, &readDataVect[0], readDataVect.size());
			//	int nLen2		= readDataVect.size();
			//	int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
			//	printf("write nlen=%d\n", nNewLen2);
			//	printf("%s Send Data:", GetCurTime().c_str());
			//	for (int i = 0; i < nNewLen2; ++i)
			//	{
			//		printf("%02X ", (BYTE)szSendData[i]);
			//	}
			//	printf("\n");
			//	if(nNewLen<0)	//	断开
			//	{
			//		CloseSocket(nNewSocket);
			//		readDataVect.clear();
			//		break;
			//	}
			//	else
			//	{
			//		;
			//	}

			//}

			char szTmpData[5 + 1] = {0};
			string strLogData = "";
			int nRecvedFlag = 0;
			while (1)
			{
				if (nRecvedFlag == 1)
					break;

				syslog(LOG_DEBUG, "==============================================1\n");

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
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x65)
						{
							//read(1 + 2);
							int nStep = 5;
							//memcpy(pBuffer, pNewBuffer, nStep);						
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
							//int nNewLen2 = SocketWrite(nNewSocket, pBuffer, nStep, 30);
							//printf("write nlen=%d\n", nNewLen2);
							//printf("%s Send Data:", GetCurTime().c_str());
							//for (int i = 0; i < nNewLen2; ++i)
							//{
							//	printf("%02X ", (BYTE)pBuffer[i]);
							//}
							//printf("\n");
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


							//char szSendData[50 + 1] = {0};
							//vector<BYTE> readDataVect;
							//rtu.WriteRegisterRequest(readDataVect, 0x00);
							//memcpy(szSendData, &readDataVect[0], readDataVect.size());
							//int nLen2		= readDataVect.size();
							//int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
							//printf("write nlen=%d\n", nNewLen2);
							//printf("%s Send Data:", GetCurTime().c_str());
							//for (int i = 0; i < nNewLen2; ++i)
							//{
							//	printf("%02X ", (BYTE)szSendData[i]);
							//}
							//printf("\n");
							//if(nNewLen<0)	//	断开
							//{
							//	CloseSocket(nNewSocket);
							//	readDataVect.clear();
							//	break;
							//}
							//else
							//{
							//	;
							//}	

						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x10)
						{
							char szRecvData[50 + 1] = {0};
							int nStep = 8;
							memcpy(szRecvData, pNewBuffer, nStep);	
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
							//read(4 + 2);
							//bool bRet = false;
							//bRet = rtu.WriteRegisterResponse(dataVect);
							//if (bRet)
							//{
							//	;
							//}
							if (szRecvData[2] == 0x03 && (BYTE)szRecvData[3] == 0xFE)
							{
								if (g_nRisingLever > 0)
								{
									BYTE bM	= 0x09;
									BYTE CH	= 0x01;
									for (int i = 0; i < 4; ++i)
									{
										if (i < CSimpleConfig::m_typeVect.size() && CSimpleConfig::RELAY_ENABLE[i] == '1' && CSimpleConfig::m_typeVect[i] == "UP")
										{
											CH		= (BYTE)CSimpleConfig::RELAY_BIT[i];
											break;
										}
									}
									
									char szSendData[50 + 1] = {0};
									vector<BYTE> readDataVect;
									rtu.WriteCoilRequestRising(readDataVect, bM, CH);
									memcpy(szSendData, &readDataVect[0], readDataVect.size());
									int nLen2		= readDataVect.size();
									int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
									syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
									strLogData = "Send Data:";
									for (int i = 0; i < nNewLen2; ++i)
									{
										memset(szTmpData, 0x00, 5);
										sprintf(szTmpData, "%02X ", (BYTE)szSendData[i]);
										strLogData += szTmpData;
									}
									syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
									if (nNewLen2 < 0)	//	断开
									{
										CloseSocket(nNewSocket);
										readDataVect.clear();
										pthread_mutex_lock (&g_ImageDataMutex); 
										g_nRisingLever	= 0;
										pthread_mutex_unlock (&g_ImageDataMutex);
										nLenLeft	= 0;
										nRecvedFlag	= 1;
										break;
									}
									else
									{
										pthread_mutex_lock (&g_ImageDataMutex); 
										g_nRisingLever	= 0;
										pthread_mutex_unlock (&g_ImageDataMutex);
									}
									nLenLeft	= 0;
									nRecvedFlag	= 1;
								}

								// g_nOffLever
								if (g_nOffLever > 0)
								{

									BYTE bM	= 0x09;
									BYTE CH	= 0x01;
									for (int i = 0; i < 4; ++i)
									{
										if (i < CSimpleConfig::m_typeVect.size() && CSimpleConfig::RELAY_ENABLE[i] == '1' && CSimpleConfig::m_typeVect[i] == "DOWN")
										{
											CH		= (BYTE)CSimpleConfig::RELAY_BIT[i];
											break;
										}
									}

									char szSendData[50 + 1] = {0};
									vector<BYTE> readDataVect;
									rtu.WriteCoilRequestOff(readDataVect, bM, CH);
									memcpy(szSendData, &readDataVect[0], readDataVect.size());
									int nLen2		= readDataVect.size();
									int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
									syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
									strLogData = "Send Data:";
									for (int i = 0; i < nNewLen2; ++i)
									{
										memset(szTmpData, 0x00, 5);
										sprintf(szTmpData, "%02X ", (BYTE)szSendData[i]);
										strLogData += szTmpData;
									}
									syslog(LOG_DEBUG, "%s\n", strLogData.c_str());

									if (nNewLen2 < 0)	//	断开
									{
										CloseSocket(nNewSocket);
										readDataVect.clear();
										pthread_mutex_lock (&g_ImageDataMutex); 
										g_nOffLever	= 0;
										pthread_mutex_unlock (&g_ImageDataMutex);
										nLenLeft	= 0;
										nRecvedFlag	= 1;
										break;
									}
									else
									{
										pthread_mutex_lock (&g_ImageDataMutex); 
										g_nOffLever	= 0;
										pthread_mutex_unlock (&g_ImageDataMutex);

									}	
									nLenLeft	= 0;
									nRecvedFlag	= 1;
								}
								break;
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
							//vector<BYTE> readDataVect((BYTE*)pNewBuffer, (BYTE*)pNewBuffer + nStep);
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);

							//rtu.ReadRegisterResponse(readDataVect, Length);
						}
						else
						{
							syslog(LOG_DEBUG, "%s head[0] head[1]: 0x%02X, 0x%02X\n", GetCurTime().c_str(), (BYTE)pNewBuffer[0], (BYTE)pNewBuffer[1]);
							nLenLeft	= 0;
						}
						//pthread_cond_signal (&g_ImageDataCond);
						// write register
					}
				}
				if(nLen<0)
				{
					//	读断开
					CloseSocket(nNewSocket);
					break;
				}
			}

			int nRecvedFlag2 = 0;
			while (1)
			{
				if (nRecvedFlag2 == 1)
					break;

				syslog(LOG_DEBUG, "==============================================2\n");

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
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x65)
						{
							//read(1 + 2);
							int nStep = 5;
							//memcpy(pBuffer, pNewBuffer, nStep);						
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
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


							SysSleep(500); //sleep(4);		// 2015-3-27
							char szSendData[50 + 1] = {0};
							vector<BYTE> readDataVect;
							rtu.WriteRegisterRequest(readDataVect, 0x00);
							memcpy(szSendData, &readDataVect[0], readDataVect.size());
							int nLen2		= readDataVect.size();
							int nNewLen2	= SocketWrite(nNewSocket,szSendData,nLen2,30);
							syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
							strLogData = "Send Data:";
							for (int i = 0; i < nNewLen2; ++i)
							{
								memset(szTmpData, 0x00, 5);
								sprintf(szTmpData, "%02X ", (BYTE)szSendData[i]);
								strLogData += szTmpData;
							}
							syslog(LOG_DEBUG, "%s\n", strLogData.c_str());

							if(nNewLen2 < 0)	//	断开
							{
								CloseSocket(nNewSocket);
								readDataVect.clear();
								break;
							}
							else
							{
								;
							}	

							//while (1)
							//{
							//	nLen=SocketRead(nNewSocket, pNewBuffer, nMaxLen);	

							//	printf("recv  nLen=%d\n", nLen);
							//	if(nLen>0)
							//	{
							//		printf("%s Recv Data:", GetCurTime().c_str());
							//		for (size_t i = 0; i < nLen; ++i)
							//		{
							//			printf("%02X ", (BYTE)pNewBuffer[i]);
							//		}
							//		printf("\n");
							//		if (pNewBuffer[1] == 0x10)
							//			break;
							//	}
							//	
							//}


							nLenLeft	= 0;
							nRecvedFlag2 = 1;
							break;

						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x10)
						{
							//char szRecvData[50 + 1] = {0};
							int nStep = 8;
							//memcpy(szRecvData, pNewBuffer, nStep);	
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
							//read(4 + 2);
						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x03)
						{
							//Length = read(1);
							//read(Length + 2);
							int nStep = 0;
							BYTE Length = (BYTE)pNewBuffer[2];
							nStep = Length + 1 + 4;
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						}
						else
						{
							syslog(LOG_DEBUG, "%s head[0] head[1]: 0x%02X, 0x%02X\n", GetCurTime().c_str(), (BYTE)pNewBuffer[0], (BYTE)pNewBuffer[1]);
							nLenLeft	= 0;
						}
					}
				}
				if(nLen<0)
				{
					//	读断开
					CloseSocket(nNewSocket);
					break;
				}

			}


			// recv reset
			int nRecvedFlag3 = 0;
			while (1)
			{
				if (nRecvedFlag3 == 1)
					break;

				syslog(LOG_DEBUG, "==============================================3\n");

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
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x65)
						{
							//read(1 + 2);
							int nStep = 5;
							//memcpy(pBuffer, pNewBuffer, nStep);						
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
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
							//char szRecvData[50 + 1] = {0};
							int nStep = 8;
							//memcpy(szRecvData, pNewBuffer, nStep);	
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
							//read(4 + 2);
							nLenLeft	= 0;
							nRecvedFlag3 = 1;
							break;							
						}
						else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x03)
						{
							//Length = read(1);
							//read(Length + 2);
							int nStep = 0;
							BYTE Length = (BYTE)pNewBuffer[2];
							nStep = Length + 1 + 4;
							nLenLeft -= nStep;
							if (nLenLeft > 0)
								memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
						}
						else
						{
							syslog(LOG_DEBUG, "%s head[0] head[1]: 0x%02X, 0x%02X\n", GetCurTime().c_str(), (BYTE)pNewBuffer[0], (BYTE)pNewBuffer[1]);
							nLenLeft	= 0;
						}
					}
				}
				if(nLen<0)
				{
					//	读断开
					CloseSocket(nNewSocket);
					break;
				}

			}

			if((nSocketErrorFlag==0)&&(nLoopTotal>0))
			{
				SysSleep(50);
				if(nLoopTotal>=nLoopMax)
				{
					nLoopTotal=0;
				}
			}

		}	
		CloseSocket(nNewSocket);
	}
	SysSleep(50);
	pthread_exit(NULL);
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

