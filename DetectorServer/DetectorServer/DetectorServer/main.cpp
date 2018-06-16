// main.cpp : Defines the entry point for the console application.
//
#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "MSGHandleCenter.h"
#include "MyLog.h"
#include "SysUtil.h"
#include "include/NetCamera_SnapServer.h"
#include "include/NetCamera.h"
#include "PicCapture.h"
#include "JieziBoxStruct.h"
#include "ProcessImage.h"
#include "RecoClientSDK.h"
#include <syslog.h>
#include <dlfcn.h>
#include <stack>
#include <vector>
#include <zmq.h>
using namespace std;
pthread_mutex_t g_ImageDataMutex;
pthread_cond_t	g_ImageDataCond;
struct struDeviceAndServiceStatus	g_ObjectStatus;
pthread_mutex_t						g_StatusMutex;

void GetCurTime(char *pTime)
{
	time_t t 		= time(0);
	struct tm *ld	= NULL;
	char tmp[32] 	= {0};
	ld					= localtime(&t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ld);
	memcpy(pTime, tmp, 64);
}

void BuildStatusString(struct struDeviceAndServiceStatus *pStatus, char *pMsg, int nCount)
{
	char szMsg[512] = {0};
	snprintf(szMsg, sizeof(szMsg), "NJJZTECH : |%s|%d|%s|%d|%s|%d|%s|%s|%s|%s|%d", \
		pStatus->szLocalIP.c_str(), \
		pStatus->nLocalPort, \
		pStatus->szUpstreamIP.c_str(), \
		pStatus->nUpstreamPort, \
		pStatus->szUpstreamObjectCode.c_str(), \
		pStatus->nServiceType, \
		pStatus->szObjectCode.c_str(), \
		pStatus->szObjectStatus.c_str(), \
		pStatus->szConnectedObjectCode.c_str(), \
		pStatus->szReportTime.c_str(), \
		nCount);
	memcpy(pMsg, szMsg, sizeof(szMsg));
	szMsg[511] = '\0';
	return;
}

void *pthread_worker_task(void *arg)
{
	char szPublisherIp[64] = {0};
	signal(SIGPIPE,SIG_IGN);

	void * pCtx = NULL;
	void * pSock = NULL;
	//浣跨敤tcp鍗忚杩涜閫氫俊锛岄渶瑕佽繛鎺ョ殑鐩爣鏈哄櫒IP鍦板潃涓?92.168.1.2
	//閫氫俊浣跨敤鐨勭綉缁滅鍙?涓?766
	snprintf(szPublisherIp, sizeof(szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
	szPublisherIp[63] = '\0';
	const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

	//鍒涘缓context
	if((pCtx = zmq_ctx_new()) == NULL)
	{
		return 0;
	}
	//鍒涘缓socket
	if((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
	{
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	int iSndTimeout = 5000;// millsecond
	//璁剧疆鎺ユ敹瓒呮椂
	if(zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof(iSndTimeout)) < 0)
	{
		zmq_close(pSock);
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	//杩炴帴鐩爣IP192.168.1.2锛岀鍙?766
	if(zmq_connect(pSock, pAddr) < 0)
	{
		zmq_close(pSock);
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	//寰幆鍙戦�佹秷鎭?   
	while(1)
	{
		static int i = 0;
		char szMsg[1024] = {0};
		pthread_mutex_lock(&g_StatusMutex);
		BuildStatusString(&g_ObjectStatus, szMsg, i++);
		pthread_mutex_unlock(&g_StatusMutex);

		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
		//printf("Enter to send...\n");
		syslog(LOG_DEBUG, "Enter to send...\n");
		if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
		{
			//fprintf(stderr, "send message faild\n");
			syslog(LOG_ERR, "send message faild\n");
			continue;
		}
		syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
		// getchar();
		usleep (5000 * 1000);
	}

	return 0;
}

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor*)arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

// int rsmain()
int main(int argc, char* argv[])
{
	pthread_t tid_worker_task;
	openlog("DetectorServer", LOG_PID, LOG_LOCAL2);
    pthread_cond_init(&g_ImageDataCond, NULL);   
    pthread_mutex_init(&g_ImageDataMutex, NULL);
	CSimpleConfig cg;
	cg.get_config();

    ProcessImage processImage;
    processImage.Init();  
    
    /* Ignore signals generated when a connection is broken unexpectedly. */
    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);
//    ACE_UNUSED_ARG(argc);
 //   ACE_UNUSED_ARG(argv);
    //get config details
    CMyLog::Init();
    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);
    /*  start event and control thread   */
    ACE_Thread_Manager::instance()->spawn(event_loop, reactor);
    // ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up controlcenter daemon\n"));
    // ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));

	syslog(LOG_DEBUG, "starting up controlcenter daemon\n");
	syslog(LOG_DEBUG, "starting up reactor event loop ...\n");

	pthread_mutex_lock(&g_StatusMutex);
	g_ObjectStatus.szLocalIP				= CSimpleConfig::m_strChannelControlIP;
	g_ObjectStatus.nLocalPort				= 0;
	g_ObjectStatus.szUpstreamIP				= CSimpleConfig::m_strChannelControlIP;
	g_ObjectStatus.nUpstreamPort			= CSimpleConfig::CHANNEL_CONTROLER_PORT;
	g_ObjectStatus.szUpstreamObjectCode		= "APP_CHANNEL_001";
	g_ObjectStatus.nServiceType				= 0;
	g_ObjectStatus.szObjectCode				= "DEV_DETECTOR_001";
	g_ObjectStatus.szObjectStatus			= "030";
	g_ObjectStatus.szConnectedObjectCode	= "COMM_CONTROL_001";

	{
		char szNowTime[32] = {0};
		GetCurTime(szNowTime);
		szNowTime[31] = '\0';
		g_ObjectStatus.szReportTime = szNowTime;
	}
	pthread_mutex_unlock(&g_StatusMutex);


    MSG_HANDLE_CENTER::instance()->open();
    /*
     * read config from config file,and open port ,listen
     */
    ACE_Reactor* reactorptr = ACE_Reactor::instance();
    
    
    
    
    processImage.Run();
    
	pthread_create(&tid_worker_task, NULL, pthread_worker_task, NULL);
	syslog(LOG_DEBUG, "create the worker task pthread OK!\n");

    MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));
	syslog(LOG_DEBUG, "shutting down controlcenter daemon\n");

	g_ObjectStatus.szLocalIP				= CSimpleConfig::m_strChannelControlIP;
	g_ObjectStatus.nLocalPort				= 0;
	g_ObjectStatus.szUpstreamIP				= CSimpleConfig::m_strChannelControlIP;
	g_ObjectStatus.nUpstreamPort			= CSimpleConfig::CHANNEL_CONTROLER_PORT;
	g_ObjectStatus.szUpstreamObjectCode		= "APP_CHANNEL_001";
	g_ObjectStatus.nServiceType				= 0;
	g_ObjectStatus.szObjectCode				= "DEV_DETECTOR_001";
	g_ObjectStatus.szObjectStatus			= "031";
	g_ObjectStatus.szConnectedObjectCode	= "COMM_CONTROL_001";

	{
		char szNowTime[32] = {0};
		GetCurTime(szNowTime);
		szNowTime[31] = '\0';
		g_ObjectStatus.szReportTime = szNowTime;
	}
	usleep(50 * 1000);

    return 0;
}
/*
int Daemon()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        return -1;
    }
    else if (pid != 0)
    {
        exit(0);
    }
    setsid();
    return 0;
}
#define SOFTWARE_VERSION  "version 2.8"         //软件版本号
int main(int argc, char *argv[])
{
    int iRet;
    int iStatus;
    pid_t pid;
    //显示版本号
    if (argc == 2)
    {
        //如果是查看版本号
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-V") || !strcmp(argv[1], "-Version") || !strcmp(argv[1], "-version"))
        {
            printf("%s  %s\n", argv[0], SOFTWARE_VERSION);
            return 0;
        }
    }
    Daemon();
createchildprocess:
    //开始创建子进程
    printf("begin to create	the child process of %s\n", argv[0]);
    int itest = 0;
    switch (fork())//switch(fork())
    {
        case -1 : //创建子进程失败
           printf("cs创建子进程失败\n");
            return -1;
        case 0://子进程
             printf("cs创建子进程成功\n");
            rsmain();
            return -1;
        default://父进程
            pid = wait(&iStatus);
            printf("子进程退出，5秒后重新启动......\n");
            sleep(5);
            goto createchildprocess; //重新启动进程
            break;
    }
    return 0;
}
*/


