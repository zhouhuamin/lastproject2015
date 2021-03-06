// MSGHandleCenter.cpp: implementation of the CMSG_Handle_Center class.

//

//////////////////////////////////////////////////////////////////////

#include "MSGHandleCenter.h"
#include "SimpleConfig.h"
#include "MyLog.h"
#include "ace/SOCK_Connector.h"
#include "ace/Signal.h"
#include "SysMessage.h"
#include <vector>
#include "CascadeCmdHandler.h"
#include "SysUtil.h"
#include <dlfcn.h>
#include <syslog.h>
#include "jzLocker.h"

#define MAX_RECORD_NUMBER    8



ACE_Thread_Mutex CMSG_Handle_Center::msg_lock_;



std::vector<NET_PACKET_MSG*> CMSG_Handle_Center::msgVec_;

int CMSG_Handle_Center::m_nMsgBufferCount = 0;

char CMSG_Handle_Center::msg_buffer_[sizeof (NET_PACKET_MSG) * MAX_BUFFERD_MESSAGE_NUMBER] = {0};



extern "C" {

    typedef void*(*THREADFUNC)(void*);

}

extern sem	g_sem;
extern int g_nStop;
extern locker g_stopLock;


//void ReceiveReaderData(unsigned char* read_data, void* user_data) {

//    NVP_PACKET_MSG* pMsg_ = NULL;

//    CMSG_Handle_Center::get_message(pMsg_);

//    if (!pMsg_) {

//        return;

//    }

//

//    memset(pMsg_, 0, sizeof (NVP_PACKET_MSG));

//

//    pMsg_->msg_head.msg_type = SYS_MSG_PUBLISH_EVENT;

//

//    pMsg_->msg_head.packet_len = sizeof (T_SysEventData);

//

//    T_SysEventData* pReadData = (T_SysEventData*) pMsg_->msg_body;

//

//    strcpy(pReadData->event_id, SYS_EVENT_ICREADER_READ_COMPLETE);

//    strcpy(pReadData->device_tag, CSimpleConfig::DEVICE_TAG);

//

//

//    char szXMLGatherInfo[10 * 1024] = {0};

//    sprintf(szXMLGatherInfo,

//            "<CONTA>\n"

//            "<CONTA_NUM>2</CONTA_NUM>\n"

//            "<CONTA_RECO>1</CONTA_RECO>\n"

//            "<CONTA_ID_F>%s</CONTA_ID_F>\n"

//            "<CONTA_ID_B>%s</CONTA_ID_B>\n"

//            "<CONTA_MODEL_F>%s</CONTA_MODEL_F>\n"

//            "<CONTA_MODEL_B>%s</CONTA_MODEL_B>\n"

//            "</CONTA>\n"

//            , "HLXU4074106"

//            , "FCIU9176921"

//            , "20"

//            , "20"

//            );  

////    sprintf(szXMLGatherInfo,

////            "<IC>\n"

////            "<DR_IC_NO>%s</DR_IC_NO>\n"

////            "<IC_DR_CUSTOMS_NO></IC_DR_CUSTOMS_NO>\n"

////            "<IC_CO_CUSTOMS_NO></IC_CO_CUSTOMS_NO>\n"

////            "<IC_BILL_NO></IC_BILL_NO>\n"

////            "<IC_GROSS_WT></IC_GROSS_WT>\n"

////            "<IC_VE_CUSTOMS_NO></IC_VE_CUSTOMS_NO>\n"

////            "<IC_VE_NAME></IC_VE_NAME>\n"

////            "<IC_CONTA_ID></IC_CONTA_ID>\n"

////            "<IC_ESEAL_ID></IC_ESEAL_ID>\n"

////            "<IC_BUSS_TYPE></IC_BUSS_TYPE>\n"

////            "<IC_EX_DATA></IC_EX_DATA>\n"

////            "</IC>\n"

////            , read_data

////            );

//

//    strcpy(pReadData->xml_data, szXMLGatherInfo);

//

//    pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;

//

//    pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;

//

//    MSG_HANDLE_CENTER::instance()->put(pMsg_);

//

//    return;

//}



void ReceiveReaderErrorState(int error_state, void* user_data) 
{

}

CMSG_Handle_Center::~CMSG_Handle_Center() 
{
    ACE_Reactor::instance()->cancel_timer(this);
}

void CMSG_Handle_Center::ThreadHandleMessage(void* lParam) 
{

    CMSG_Handle_Center* pTask = (CMSG_Handle_Center*) lParam;

    while (1) {
        pTask->msg_semaphore_.acquire();
        NET_PACKET_MSG* pMsg_ = NULL;
        {
            ACE_GUARD(ACE_Thread_Mutex, guard, pTask->msg_handle_lock_);
            if (pTask->msgToHandleList_.size() == 0) {
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 100 * 1000;
                select(0, 0, NULL, NULL, &tv);
                continue;
            }

            pMsg_ = pTask->msgToHandleList_.front();
            pTask->msgToHandleList_.pop_front();
        }

        if (!pMsg_) {
            continue;
        }

        try 
		{

            pTask->HandleNetMessage(pMsg_);

            syslog(LOG_DEBUG, "end handle msg ......\n");

        }        
		catch (...) 
		{

            syslog(LOG_DEBUG, "********ThreadHandleMessage exception!msg type %d\n", pMsg_->msg_head.msg_type);

            exit(0);

        }

    }

}

int CMSG_Handle_Center::handle_timeout(const ACE_Time_Value &tv, const void *arg) 
{
    ACE_UNUSED_ARG(tv);
    ACE_UNUSED_ARG(arg);
    NET_PACKET_MSG* pMsg_ = NULL;

    get_message(pMsg_);

    if (!pMsg_) {
        return -1;
    }

    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
    //  put(pMsg_);
    return 0;
}

static ACE_THR_FUNC_RETURN ThreadConnectToCascadeServerFunc(void *lParam) {
    CMSG_Handle_Center* pTask = (CMSG_Handle_Center*) lParam;
    pTask->ConnectToCascadeServer();
    return 0;
}

int CMSG_Handle_Center::open(void*) {
    m_nCascadeSocket = -1;
    m_nTimeCount = 0;
    m_pSendBuffer = new char[20 * 1024];
    init_message_buffer();
    msg_semaphore_.acquire();
    msg_send_semaphore_.acquire();
    m_pDevice = NULL;
    InitDeviceSo();

    //设置两个线程，分两个阶段处理接收的图片信息

    ACE_Thread_Manager::instance()->spawn(ThreadConnectToCascadeServerFunc, this);
    SysUtil::SysSleep(1000);

    int nThreadErr = 0;
    pthread_t rcsThreadId;

    nThreadErr = pthread_create(&rcsThreadId, NULL, (THREADFUNC) ThreadHandleMessage, this);

    if (ACE_Reactor::instance()->schedule_timer(this,
            0,
            ACE_Time_Value(2, 0),
            ACE_Time_Value(2, 0)) == -1) {
        return -1;
    }

    return 0;
}

int CMSG_Handle_Center::get_message(NET_PACKET_MSG *&pMsg) {

    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_lock_, -1);



    pMsg = msgVec_[m_nMsgBufferCount];



    m_nMsgBufferCount++;

    if (m_nMsgBufferCount > MAX_BUFFERD_MESSAGE_NUMBER - 1) {

        m_nMsgBufferCount = 0;

    }

    return 0;
}

int CMSG_Handle_Center::init_message_buffer() {

    for (int i = 0; i < MAX_BUFFERD_MESSAGE_NUMBER; i++) {

        char* chMsgBuffer = msg_buffer_;

        NET_PACKET_MSG* pMsg = (NET_PACKET_MSG*) (chMsgBuffer + i * sizeof (NET_PACKET_MSG));

        msgVec_.push_back(pMsg);

    }



    return 0;

}



int CMSG_Handle_Center::HandleNetMessage(NET_PACKET_MSG *pMsg) {

    char chMsg[512] = {0};



    if (!pMsg) {

        return -1;

    }



    syslog(LOG_DEBUG, "begin handle msg type %d \n", pMsg->msg_head.msg_type);



    switch (pMsg->msg_head.msg_type) {

        case SYS_MSG_SYSTEM_REGISTER_ACK:

            handleRegisterAck(pMsg);

            break;

        case SYS_MSG_PUBLISH_EVENT:

            handleICReaderData(pMsg);

            break;

		case SYS_MSG_SYSTEM_CTRL:
			handleControlEvent(pMsg);
			break;
        default:
            break;
    }
    return 0;
}

int CMSG_Handle_Center::ConnectToCascadeServer() {

    CCascadeCmdHandler * handler = new CCascadeCmdHandler;



    while (1) {

        if (!CCascadeCmdHandler::m_bConnected) 
		{
            if (handler->ConnectToCascadeServer(CSimpleConfig::CHANNEL_CONTROLER_PORT, const_cast<char*>(CSimpleConfig::m_strChannelControlIP.c_str())) == -1) 
			{

            } 
			else 
			{
                CCascadeCmdHandler::m_bConnected = true;
            }

        }

        //select代替sleep,等待一段时间间隔5s

        SysUtil::SysSleep(5 * 1000);

    }

    delete handler;
}

int CMSG_Handle_Center::RecordCascadeConnection(int nSocket) 
{

    m_nCascadeSocket = nSocket;

    RegisterToDeviceServer();



    return 0;

}



int CMSG_Handle_Center::InitDeviceSo() 
{
//
//    if (strlen(CSimpleConfig::DEVICE_SO) > 0) {
//
//        DeviceInfo device_info;
//
//        memset(&device_info, 0, sizeof (DeviceInfo));
//
//
//
//        device_info.com_port = CSimpleConfig::COM_PORT;
//
//        device_info.baud_date = CSimpleConfig::BAUD_RATE;
//
//
//
//        char device_so_path[256] = {0};
//
//        sprintf(device_so_path, "/usr/lib/%s", CSimpleConfig::DEVICE_SO);
//
//
//
//        void* device_so = dlopen(device_so_path, RTLD_LAZY);
//
//        if (!device_so) {
//
//
//
//            syslog(LOG_DEBUG, "********load library fail! dll name is  %s ,error %s %d \n", device_so_path, dlerror(), errno);
//
//            return 1;
//
//        } else {
//
//            syslog(LOG_DEBUG, "load library succ! dll name is  %s \n", device_so_path);
//
//        }
//
//
//
//        // reset errors
//
//        dlerror();
//
//
//
//        // load the symbols
//
//        create_t* create_device = (create_t*) dlsym(device_so, "create");
//
//        const char* dlsym_error = dlerror();
//
//        if (dlsym_error) {
//
//            syslog(LOG_DEBUG, "Cannot load symbol create:  %s \n", dlsym_error);
//
//            return 1;
//
//        }
//
//
//
//        if (create_device) {
//
//            syslog(LOG_DEBUG, "get create function pointer %p succ !\n", create_device);
//
//        }
//
//
//
////        // create an instance of the class
//
////        m_pDevice = create_device();
//
////
//
////        if (m_pDevice != NULL) //创建设备成功
//
////        {
//
////
//
////            CMyLog::m_pLog->_XGSysLog("create device object succ\n");
//
////
//
////            int nRet = m_pDevice->init_device(device_info);
//
////
//
////            //m_pDevice->SetReadDataCallback((_READ_DATA_CALLBACK_) ReceiveReaderData, this);
//
////            m_pDevice->SetErrorStateCallback((_ERROR_STATE_CALLBACK_) ReceiveReaderErrorState, this);
//
////
//
////        } else {
//
////            CMyLog::m_pLog->_XGSysLog("********create device object fail\n");
//
////
//
////        }
//
//    }



    return 0;

}



int CMSG_Handle_Center::handleRegisterAck(NET_PACKET_MSG *pMsg) {

    if (!pMsg) {

        return -1;

    }



    T_DeviceServerRegister_Ack* pRegisterAck = (T_DeviceServerRegister_Ack*) pMsg->msg_body;

    if (pRegisterAck->reg_result == 0) {

        if (pRegisterAck->next_action == 1) {

           // m_pDevice->start_work();

        }

    }



    return 0;

}



int CMSG_Handle_Center::handleICReaderData(NET_PACKET_MSG *pMsg) {

    if (!pMsg) {

        return -1;

    }



    if (m_nCascadeSocket > 0) {

        T_SysEventData* pReadData = (T_SysEventData*) pMsg->msg_body;



        char chReq[1024 * 10] = {0};

        memset(chReq, 1024 * 10, 0);



        int sendLen = 0;

        memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据

        sendLen += 8;



        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);

        pHead->msg_type = pMsg->msg_head.msg_type;

        pHead->packet_len = pMsg->msg_head.packet_len;

        sendLen += sizeof (NET_PACKET_HEAD);



        T_SysEventData* pReadDataToSend = (T_SysEventData*) (chReq + sendLen);

        memcpy(pReadDataToSend, pReadData, sizeof (T_SysEventData) + pReadData->xml_data_len);





        sendLen += sizeof (T_SysEventData) + pReadData->xml_data_len;



        memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据

        sendLen += 8;



        int nRet = SysUtil::SocketWrite(m_nCascadeSocket, chReq, sendLen, 100);

        if (nRet == sendLen) {

            syslog(LOG_DEBUG, "send conta reader data to device server succ...\n");

        } else {

            syslog(LOG_DEBUG, "********send conta reader data to device server fail,to send is %d ,send is %d...\n", sendLen, nRet);

        }



    } else {

        syslog(LOG_DEBUG, "********device server connection is broken...\n");

    }



}



int CMSG_Handle_Center::RegisterToDeviceServer() {

    char chReq[1024] = {0};

    memset(chReq, 1024, 0);



    int sendLen = 0;

    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据

    sendLen += 8;



    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);

    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_REQ;

    pHead->packet_len = sizeof (T_DeviceServerRegister_Req);



    sendLen += sizeof (NET_PACKET_HEAD);





    T_DeviceServerRegister_Req* pRegisterReq = (T_DeviceServerRegister_Req*) (chReq + sendLen);

	strcpy(pRegisterReq->device_tag, CSimpleConfig::DEVICE_TAG);

    strcpy(pRegisterReq->device_id, CSimpleConfig::DEVICE_ID);



    sendLen += sizeof (T_DeviceServerRegister_Req);



    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据

    sendLen += 8;

    int nRet = SysUtil::SocketWrite(m_nCascadeSocket, chReq, sendLen, 100);



    if (nRet == sendLen) {

        syslog(LOG_DEBUG, "register to device server succ...\n");

    } else {

        syslog(LOG_DEBUG, "********egister to device server fail,to send is %d ,send is %d...\n", sendLen, nRet);

    }

}

int CMSG_Handle_Center::handleControlEvent(NET_PACKET_MSG *pMsg)
{
		//case SYS_MSG_LEVERCONTROLER_ON:
		//	g_nRisingLever	= 1;
		//	break;
		//case SYS_MSG_LEVERCONTROLER_OFF:
		//	g_nOffLever		= 1;
		//	break;

	for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
	{
		if (strcmp(CSimpleConfig::m_eventVect[i].event_id, pMsg->msg_body) == 0)
		{

			if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "1") == 0)
			{
				syslog(LOG_DEBUG, "========================Recv EC_START_DELAY=================\n");
				g_sem.post();
				break;
			}

			if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "3") == 0)
			{
				syslog(LOG_DEBUG, "========================Recv EC_STOP_DELAY=================\n");
				g_stopLock.lock();
				g_nStop = 1;
				g_stopLock.unlock();
				break;
			}
		}
	}
}
