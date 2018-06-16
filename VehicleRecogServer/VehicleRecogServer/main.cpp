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

#include "jzLocker.h"
#include <syslog.h>
#include <dlfcn.h>
#include <stack>
#include <vector>
#include <new>
#include <cstdlib>


using namespace std;
std::stack<structImageData> g_ImageDataStack;
vector<structBoxNumberRecogResult> g_boxNumberSet;

vector<structVP_PASS_VEHICLE_INFO>		g_VehicleDataVect;
std::stack<structVP_PASS_VEHICLE_INFO>	g_VehicleDataStack;

pthread_mutex_t g_ImageDataMutex;
pthread_cond_t	g_ImageDataCond;

pthread_mutex_t g_VehicleDataVectMutex;
pthread_mutex_t g_VehicleDataStackMutex;

sem				g_VehicleSemArrived;
sem				g_VehicleSemStart;
sem				g_VehicleSemStop;
//sem				g_VehicleSemFinishded;

void recv_snap_pic(char* szServerIP, unsigned char* pPicBuffer, int nPicLen)
{
    syslog(LOG_DEBUG, "recv camera %s,pic len is %d\n",szServerIP,nPicLen);
	string strFrontIp	= CSimpleConfig::CameraFront;
	string strBackIp	= CSimpleConfig::CameraBack;
	string strLeft		= CSimpleConfig::CameraLeft;
	string strRight		= CSimpleConfig::CameraRight;
	if (pPicBuffer == NULL || nPicLen <= 0)
		return;
	structImageData imagedata;
	imagedata.strIp		= szServerIP;
	imagedata.nPicLen	= nPicLen;
	if (imagedata.strIp == strFrontIp)
		imagedata.direct = CFRONT;
	else if (imagedata.strIp == strBackIp)
		imagedata.direct = CBACK;
	else if (imagedata.strIp == strLeft)
		imagedata.direct	= CLEFT;
	else if (imagedata.strIp == strRight)
		imagedata.direct	= CRIGHT;

	try
	{
		imagedata.pPicBuffer	= new unsigned char[nPicLen + 1];
	}
	catch (const bad_alloc& e)
	{
		syslog(LOG_DEBUG, "image data new failed\n");
		return;
	}

	memcpy(imagedata.pPicBuffer, pPicBuffer, nPicLen);
	pthread_mutex_lock(&g_ImageDataMutex);
	g_ImageDataStack.push(imagedata);
	pthread_mutex_unlock(&g_ImageDataMutex);
    return;
}

void RecvRecoResult(char* szRecoSequence,JZ_RecoContaID* pRecoREsult)
{
	//if(pRecoREsult == NULL || pRecoREsult->nResult==-1)
	//{
	//	printf("reco fail......\n");
	//	return;
	//}

	if (szRecoSequence != NULL && pRecoREsult->ContaID[0] != '\0' && pRecoREsult->Type[0] != '\0')
		syslog(LOG_DEBUG, "reco seq %s---result conta id %s,%s\n",szRecoSequence,pRecoREsult->ContaID,pRecoREsult->Type);
	else if (szRecoSequence != NULL && pRecoREsult->ContaID[0] != '\0' && pRecoREsult->Type[0] == '\0')
		syslog(LOG_DEBUG, "reco seq %s---result conta id %s\n",szRecoSequence,pRecoREsult->ContaID);
	else if (szRecoSequence != NULL && pRecoREsult->ContaID[0] == '\0' && pRecoREsult->Type[0] == '\0')
		syslog(LOG_DEBUG, "reco seq %s---result conta id\n",szRecoSequence);
	else if (szRecoSequence == NULL)
		return;

	structBoxNumberRecogResult result;
	result.strSeqNo	= szRecoSequence;
	result.strBoxNumber	= pRecoREsult->ContaID;
	result.strBoxModel	= pRecoREsult->Type;
	// RED, BLUE, WHITE, GRAY, GREEN, OTHER
	if (pRecoREsult->color	== RED)
		result.strBoxColor	=	"RED";
	if (pRecoREsult->color	== BLUE)
		result.strBoxColor	=	"BLUE";
	if (pRecoREsult->color	== WHITE)
		result.strBoxColor	=	"WHITE";
	if (pRecoREsult->color	== GRAY)
		result.strBoxColor	=	"GRAY";
	if (pRecoREsult->color	== GREEN)
		result.strBoxColor	=	"GREEN";
	if (pRecoREsult->color	== OTHER)
		result.strBoxColor	=	"OTHER";
	if (pRecoREsult->ali.Atype == H_Align)
		result.strArrangement = "H";
	if (pRecoREsult->ali.Atype == T_Align)
		result.strArrangement = "T";
	g_boxNumberSet.push_back(result);
}

void RECVPASS_VEHICLE_INFO(VP_PASS_VEHICLE_INFO& pVehicleInfo)
{
	syslog(LOG_DEBUG, "recv snap pic info...\n");
	
	//if(pVehicleInfo.pPicBuffer)
	//{
	//	char szFileName[256]={0};
	//	sprintf(szFileName,"/root/%s.jpg",pVehicleInfo.szPassTime);
	//	FILE* fPic=fopen(szFileName,"wb");
	//	fwrite(pVehicleInfo.pPicBuffer,1,pVehicleInfo.nPicLen,fPic);
	//	fclose(fPic);
	//}

	structVP_PASS_VEHICLE_INFO	vehicleData;
	if (pVehicleInfo.szHPHM[0] != '\0')
	{
		pVehicleInfo.szHPHM[15]	= '\0';
		vehicleData.strHPHM	= pVehicleInfo.szHPHM;
		syslog(LOG_DEBUG, "plate number is :%s\n", vehicleData.strHPHM.c_str());
		syslog(LOG_DEBUG, "\n");
	}

	if (pVehicleInfo.szPassTime[0] != '\0')
	{
		vehicleData.strPassTime	= pVehicleInfo.szPassTime;
	}

	if(pVehicleInfo.pPicBuffer != NULL && pVehicleInfo.nPicLen > 0)
	{
		try
		{
			vehicleData.nPicLen		= pVehicleInfo.nPicLen;
			vehicleData.pPicBuffer	= new unsigned char[pVehicleInfo.nPicLen + 1];
			memcpy(vehicleData.pPicBuffer, pVehicleInfo.pPicBuffer, pVehicleInfo.nPicLen);
		}
		catch (const bad_alloc& e)
		{
			syslog(LOG_DEBUG, "VehicleInfo data new failed\n");
			return;
		}
	}

	pthread_mutex_lock(&g_VehicleDataVectMutex);
	g_VehicleDataVect.push_back(vehicleData);
	pthread_mutex_unlock(&g_VehicleDataVectMutex);
	g_VehicleSemArrived.post();
	return;
}


static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor*)arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

//int rsmain()
int main(int argc, char* argv[])
{
	openlog("VehicleRecogServer", LOG_PID, LOG_LOCAL4);
    pthread_cond_init(&g_ImageDataCond, NULL);   
    pthread_mutex_init(&g_ImageDataMutex, NULL);
	CSimpleConfig cg;
	cg.get_config();
	//char *pRecoIP = CSimpleConfig::RecogServerIP;			// "192.168.1.154";
	//RecoInit(pRecoIP, CSimpleConfig::RecogServerPort);		// 19000);
	//SetRecoResultCallback(RecvRecoResult);
    ProcessImage processImage;
    processImage.Init();
 //   
 //   {
 //       void* device_capture = dlopen("/usr/lib/libJZPicCapture.so", RTLD_LAZY);
 //       if (!device_capture)
 //       {
 //           //printf("load library fail! dll name is  %s \n", "libJZPicCapture.so");
 //           //		lprintf(g_log, FATAL, "load library fail! dll name is  %s \n", "libJZPicCapture.so");
 //                       
	//		//ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t)load library fail! dll name is  %s \n"), "libJZPicCapture.so"));
	//		syslog(LOG_DEBUG, "load library fail! dll name is  %s \n", "libJZPicCapture.so");
 //           return -1;
 //       }
 //       else
 //       {
 //           //printf("load library succ! dll name is  %s \n", "libJZPicCapture.so");
	////		lprintf(g_log, INFO, "load library succ! dll name is  %s \n", "libJZPicCapture.so");
 //           //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)load library succ! dll name is  %s \n"), "libJZPicCapture.so"));  
	//		syslog(LOG_DEBUG, "load library succ! dll name is  %s \n", "libJZPicCapture.so");
 //       }
 //       // reset errors
 //       dlerror();
 //       // load the symbols
 //       create_t_p* create_capture	= (create_t_p*) dlsym(device_capture, "create");
 //       const char* dlsym_error		= dlerror();
 //       if (dlsym_error)
 //       {
 //           //printf("Cannot load symbol create:  %s \n", dlsym_error);
	////		lprintf(g_log, FATAL, "Cannot load symbol create:  %s \n", dlsym_error);
 //           //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)Cannot load symbol create:  %s \n"), dlsym_error));
	//		syslog(LOG_DEBUG, "Cannot load symbol create:  %s \n", dlsym_error);
 //           return 1;
 //       }
 //       if (create_capture)
 //       {
 //           //printf("get create function pointer %p succ !\n", create_capture);
 //           //		lprintf(g_log, INFO, "get create function pointer %p succ !\n", create_capture);
 //           //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)get create function pointer %p succ !\n"), create_capture));
	//		syslog(LOG_DEBUG, "get create function pointer %p succ !\n", create_capture);
 //       }
 //       // create an instance of the class
 //       CCapturePic* m_pCapture = create_capture();
 //       
 //       if(m_pCapture)
 //       {
 //           m_pCapture->SetSnapDataCallback(recv_snap_pic);
	//		//m_pCapture->SetSnapServer(CSimpleConfig::CameraFront, 2233, "123", "123", "127.0.0.1", 9999);
	//		//m_pCapture->SetSnapServer(CSimpleConfig::CameraLeft, 2233, "123", "123", "127.0.0.1", 9999);
	//		//m_pCapture->SetSnapServer(CSimpleConfig::CameraRight, 2233, "123", "123", "127.0.0.1", 9999);
	//		//m_pCapture->SetSnapServer(CSimpleConfig::CameraBack, 2233, "123", "123", "127.0.0.1", 9999);
 //                       
	//		m_pCapture->SetSnapServer(CSimpleConfig::CameraFront, CSimpleConfig::CAMERA_FRONT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort); // "192.168.1.151", 9999);
	//		m_pCapture->SetSnapServer(CSimpleConfig::CameraLeft,  CSimpleConfig::CAMERA_LEFT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
	//		m_pCapture->SetSnapServer(CSimpleConfig::CameraRight, CSimpleConfig::CAMERA_RIGHT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
	//		m_pCapture->SetSnapServer(CSimpleConfig::CameraBack,  CSimpleConfig::CAMERA_BACK_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);                       
 //                       
 //                       
 //           m_pCapture->SetListenPort(CSimpleConfig::SnapshotServerPort);
 //         
 //       }
 //       
	//	//printf("========================================\n");
	//	//lprintf(g_log, INFO, "========================================\n");
 //       
 //       // ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)========================================\n")));
	//	 syslog(LOG_DEBUG, "========================================\n");
 //   }
    
	CDeviceBase* m_pHikIPC=NULL;
	{

		void* proxy_so = dlopen("/usr/lib/libVP_HikIPC.so", RTLD_LAZY);
		if (!proxy_so)
		{

			printf("********load library fail! dll name is  %s ,error %s %d \n", "libVP_HikIPC.so", dlerror(), errno);
			return 1;
		}
		else
		{
			printf("load library succ! dll name is  %s \n", "libVP_HikIPC.so");
		}



		// reset errors
		dlerror();

		// load the symbols
		create_t* create = (create_t*) dlsym(proxy_so, "create");
		const char* dlsym_error = dlerror();
		if (dlsym_error)
		{
			printf("Cannot load symbol create:  %s \n", dlsym_error);
			return 1;
		}

		if (create)
		{
			printf("get create function pointer %p succ !\n", create);
		}

		// create an instance of the class
		m_pHikIPC = (CDeviceBase*)create();

		if (m_pHikIPC)
		{
			VP_DEVICE_INFO device_info;
			memset(&device_info,0,sizeof(VP_DEVICE_INFO));

			strcpy(device_info.device_ip, CSimpleConfig::SnapshotServerIP); //"192.168.127.64");
			strcpy(device_info.device_pass,"12345");
			strcpy(device_info.device_user,"admin");
			device_info.command_port = CSimpleConfig::SnapshotServerPort;// 8000;


			m_pHikIPC->SetDeviceInfo(&device_info);
			m_pHikIPC->Login();

			m_pHikIPC->SetPassVehicleCB(RECVPASS_VEHICLE_INFO);


		}
	}    
    
    
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
	syslog(LOG_DEBUG, "starting up controlcenter daemon\n");
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));
	syslog(LOG_DEBUG, "starting up reactor event loop ...\n");
    MSG_HANDLE_CENTER::instance()->open();
    /*
     * read config from config file,and open port ,listen
     */
    ACE_Reactor* reactorptr = ACE_Reactor::instance();
    
    
    
    
    processImage.Run();
    
    
    MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();
    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));
	closelog();
    return 0;
}

//int Daemon()
//{
//    pid_t pid;
//    pid = fork();
//    if (pid < 0)
//    {
//        return -1;
//    }
//    else if (pid != 0)
//    {
//        exit(0);
//    }
//    setsid();
//    return 0;
//}
//#define SOFTWARE_VERSION  "version 1.0.0.0"         //软件版本号
//
//int main(int argc, char *argv[])
//{
//    int iRet;
//    int iStatus;
//    pid_t pid;
//    //显示版本号
//    if (argc == 2)
//    {
//        //如果是查看版本号
//        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-V") || !strcmp(argv[1], "-Version") || !strcmp(argv[1], "-version"))
//        {
//            printf("%s  %s\n", argv[0], SOFTWARE_VERSION);
//            return 0;
//        }
//    }
//
//    Daemon();
//
//createchildprocess:
//    //开始创建子进程
//    syslog(LOG_DEBUG, "begin to create	the child process of %s\n", argv[0]);
//
//    int itest = 0;
//    switch (fork())//switch(fork())
//    {
//        case -1 : //创建子进程失败
//			syslog(LOG_DEBUG, "cs创建子进程失败\n");
//            return -1;
//        case 0://子进程
//            syslog(LOG_DEBUG, "cs创建子进程成功\n");
//            rsmain();
//            return -1;
//        default://父进程
//            pid = wait(&iStatus);
//            syslog(LOG_DEBUG, "子进程退出，5秒后重新启动......\n");
//            sleep(5);
//            goto createchildprocess; //重新启动进程
//            break;
//    }
//    return 0;
//}



