#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "server_acceptor.h"
#include "MyLog.h"
#include "MSG_Center.h"
#include "Cmd_Acceptor.h"
#include <syslog.h>
#include <zmq.h>

#include "DeviceStatus.h"

DeviceStatus g_ObjectStatus;

static ACE_THR_FUNC_RETURN pthread_worker_task(void *arg)
{
	char szPublisherIp[64] = {0};
	signal(SIGPIPE,SIG_IGN);

	void * pCtx = NULL;
	void * pSock = NULL;
	//使用tcp协议进行通信，需要连接的目标机器IP地址�?92.168.1.2
	//通信使用的网络端�?�?766
	snprintf(szPublisherIp, sizeof(szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
	szPublisherIp[63] = '\0';
	const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

	//创建context
	if((pCtx = zmq_ctx_new()) == NULL)
	{
		return 0;
	}
	//创建socket
	if((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
	{
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	int iSndTimeout = 5000;// millsecond
	//设置接收超时
	if(zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof(iSndTimeout)) < 0)
	{
		zmq_close(pSock);
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	//连接目标IP192.168.1.2，端�?766
	if(zmq_connect(pSock, pAddr) < 0)
	{
		zmq_close(pSock);
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	//循环发送消�?   
	while(1)
	{
		static int i = 0;
		char szMsg[1024] = {0};
		g_ObjectStatus.BuildStatusString(szMsg, i++);

		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
		//printf("Enter to send...\n");
		//syslog(LOG_DEBUG, "Enter to send...\n");

		if (szMsg[0] != '\0')
		{
			if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
			{
				//fprintf(stderr, "send message faild\n");
				syslog(LOG_ERR, "send message : [%s] faild\n", szMsg);
				sleep (10);
				continue;
			}
			syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
			// getchar();
		}
		sleep (10);
	}

	return 0;
}

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor *) arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

int main(int argc, char *argv[])
{

    openlog("ChannelControlServer", LOG_PID, LOG_LOCAL6);
    /* Ignore signals generated when a connection is broken unexpectedly. */

    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);

    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);

    CMyLog::Init();
    CSimpleConfig cg;
    cg.get_config();


    //////////////////////////////////////////////////////////////////////////////////////////////
    //do timeout event

    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);


    CMyLog::m_pLog->_XGSysLog("starting up channel control server......\n");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    select(0, 0, NULL, NULL, &tv);

    MSG_CENTER::instance()->open();

    CCmd_Acceptor::InitHandler();

    CCmd_Acceptor peer_acceptor;
    if (peer_acceptor.open(ACE_INET_Addr(CSimpleConfig::LOCAL_LISTEN_PORT), reactor, 1) == -1)
    {
        CMyLog::m_pLog->_XGSysLog("open listen port %d fail...\n",CSimpleConfig::LOCAL_LISTEN_PORT);
		{		
			struDeviceAndServiceStatus tmpStatus;
			tmpStatus.szLocalIP					= "";
			tmpStatus.nLocalPort				= 0;
			tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
			tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
			tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
			tmpStatus.nServiceType				= 1;
			tmpStatus.szObjectCode				= "APP_CHANNEL_001";
			tmpStatus.szObjectStatus			= "031";
			tmpStatus.szConnectedObjectCode		= "";
			char szNowTime[32]					= {0};
			g_ObjectStatus.GetCurTime(szNowTime);
			szNowTime[31]						= '\0';
			tmpStatus.szReportTime				= szNowTime;
			g_ObjectStatus.SetDeviceStatus(tmpStatus);
		}
    }
    else
    {
        CMyLog::m_pLog->_XGSysLog("open listen port %d ok...\n", CSimpleConfig::LOCAL_LISTEN_PORT);

		{		
			struDeviceAndServiceStatus tmpStatus;
			tmpStatus.szLocalIP					= "";
			tmpStatus.nLocalPort				= 0;
			tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
			tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
			tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
			tmpStatus.nServiceType				= 1;
			tmpStatus.szObjectCode				= "APP_CHANNEL_001";
			tmpStatus.szObjectStatus			= "030";
			tmpStatus.szConnectedObjectCode		= "";
			char szNowTime[32]					= {0};
			g_ObjectStatus.GetCurTime(szNowTime);
			szNowTime[31]						= '\0';
			tmpStatus.szReportTime				= szNowTime;
			g_ObjectStatus.SetDeviceStatus(tmpStatus);
		}
    }

    ACE_Thread_Manager::instance()->spawn(event_loop,			reactor);
	ACE_Thread_Manager::instance()->spawn(pthread_worker_task,	reactor);
    MSG_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();

    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));

    return 0;
}
