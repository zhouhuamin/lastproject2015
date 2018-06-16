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
using namespace std;
pthread_mutex_t g_ImageDataMutex;
pthread_cond_t	g_ImageDataCond;

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
	openlog("DelayServer", LOG_PID, LOG_LOCAL5);
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
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up controlcenter daemon\n"));
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));
	syslog(LOG_DEBUG, "starting up DelayServer daemon\n");
	syslog(LOG_DEBUG, "starting up reactor event loop ...\n");
    MSG_HANDLE_CENTER::instance()->open();
    /*
     * read config from config file,and open port ,listen
     */
    ACE_Reactor* reactorptr = ACE_Reactor::instance();
      
    processImage.Run();
    
    
    MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));
	syslog(LOG_DEBUG, "shutting down DelayServer daemon\n");
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
#define SOFTWARE_VERSION  "version 2.8"         //����汾��
int main(int argc, char *argv[])
{
    int iRet;
    int iStatus;
    pid_t pid;
    //��ʾ�汾��
    if (argc == 2)
    {
        //����ǲ鿴�汾��
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-V") || !strcmp(argv[1], "-Version") || !strcmp(argv[1], "-version"))
        {
            printf("%s  %s\n", argv[0], SOFTWARE_VERSION);
            return 0;
        }
    }
    Daemon();
createchildprocess:
    //��ʼ�����ӽ���
    printf("begin to create	the child process of %s\n", argv[0]);
    int itest = 0;
    switch (fork())//switch(fork())
    {
        case -1 : //�����ӽ���ʧ��
           printf("cs�����ӽ���ʧ��\n");
            return -1;
        case 0://�ӽ���
             printf("cs�����ӽ��̳ɹ�\n");
            rsmain();
            return -1;
        default://������
            pid = wait(&iStatus);
            printf("�ӽ����˳���5�����������......\n");
            sleep(5);
            goto createchildprocess; //������������
            break;
    }
    return 0;
}
*/


