/* 
* File:   ProcessImage.h
* Author: root
*
* Created on 2015年1月28日, 上午10:45
*/
#ifndef PROCESSIMAGE_H
#define	PROCESSIMAGE_H
#include <pthread.h>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include "DeviceBase.h"
#include "JieziBoxStruct.h"
#include "RTUProtocol.h"
struct structBoxNumberRecogResult
{
	std::string strSeqNo;
	std::string strBoxNumber;
	std::string strBoxModel;
	std::string strBoxColor;
	std::string strArrangement;
	BoxNumberDirection  direct;
};
struct structBoxNumberRecogResultCorrected
{
	std::string strFrontBoxNumber;
	std::string strBackBoxNumber;
	std::string strFrontBoxModel;
	std::string strBackBoxModel;
	std::string strRecogResult;
	int nBoxType;                   // 1:长箱   2:双箱  3:单箱 4:短箱
	int nPicNumber;

	std::string CONTA_PIC_F;
	std::string CONTA_PIC_B;
	std::string CONTA_PIC_LF;
	std::string CONTA_PIC_RF;
	std::string CONTA_PIC_LB;
	std::string CONTA_PIC_RB;
};
class ProcessImage {
public:
	ProcessImage();
	ProcessImage(const ProcessImage& orig);
	virtual ~ProcessImage();


	void Init();
	void Run();
private:
	pthread_t controllerThreadID;
	pthread_t statusTableThreadID;
	pthread_t arrivedThreadID;
	pthread_t startThreadID;
	pthread_t stopThreadID;
	//pthread_t finishedThreadID;

private:
	static void *ProcessControllerThread(void *pParam);
	static void *ProcessStatusTableThread(void *pParam);
	static void *ProcessArrivedThread(void *pParam);
	static void *ProcessStartThread(void *pParam);
	static void *ProcessStopThread(void *pParam);
	//static void *ProcessFinishedThread(void *pParam);

	void ProcessControllerProc();
	void ProcessStatusTableProc();

	void ProcessArrivedProc();
	void ProcessStartProc();
	void ProcessStopProc();
	//void ProcessFinishedProc();


private:
	int		CreateSocket();
	int		ConnectSocket(int nSocket,const char * szHost,int nPort);
	int		CheckSocketValid(int nSocket);
	int		CloseSocket(int nSocket);
	void	SetSocketNotBlock(int nSocket);
	void	SysSleep(long nTime);
	int		SocketWrite(int nSocket,char * pBuffer,int nLen,int nTimeout);
	int		SocketRead(int nSocket,void * pBuffer,int nLen);   
	size_t	ReadAll(FILE *fd, void *buff, size_t len);
	size_t	WriteAll(FILE *fd, void *buff, size_t len);
	int		JudgeBoxType(std::vector<std::string> &statusVect, const std::vector<structBoxNumberRecogResult> &boxNumberSet, structBoxNumberRecogResultCorrected &resultCorrected);
	int		BoxNumberCorrection(const std::vector<structBoxNumberRecogResult> &boxNumberSet, int nPicNumber, int nBoxType, structBoxNumberRecogResultCorrected &resultCorrected);
	void	GetContaOwnerData(std::vector<structContaOwnerData> &contaOwnerVect);
	int set_keep_live(int nSocket, int keep_alive_times, int keep_alive_interval);


private:
	std::string		Byte2String(BYTE ch);
	int				my_itoa(int val, char* buf, int radix);
	std::string		GetCurTime(void);
	long			getCurrentTime();

private:
	//_READ_DATA_CALLBACK_ m_pReadDataCallback;

private:
	
	
};
#endif	/* PROCESSIMAGE_H */

