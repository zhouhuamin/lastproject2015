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

#define MAX_RECORD_NUMBER    8

ACE_Thread_Mutex CMSG_Handle_Center::msg_lock_;

std::vector<NET_PACKET_MSG*> CMSG_Handle_Center::msgVec_;
int CMSG_Handle_Center::m_nMsgBufferCount = 0;
char CMSG_Handle_Center::msg_buffer_[sizeof (NET_PACKET_MSG) * MAX_BUFFERD_MESSAGE_NUMBER] = {0};
std::map<string, EventGroup*> CMSG_Handle_Center::event_group_map;

CMSG_Handle_Center::~CMSG_Handle_Center()
{
    ACE_Reactor::instance()->cancel_timer(this);
}

int CMSG_Handle_Center::svc(void)
{
    while (1)
    {
        msg_semaphore_.acquire();

        NET_PACKET_MSG* pMsg_ = NULL;
        {
            ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_handle_lock_, -1);

            if (msgToHandleList_.size() == 0)
            {
#ifdef WIN32
                Sleep(1);
#else
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 1 * 1000;

                //select代替sleep,等待一段时间间隔
                select(0, 0, NULL, NULL, &tv);
#endif
                continue;
            }

            pMsg_ = msgToHandleList_.front();
            msgToHandleList_.pop_front();

        }

        if (!pMsg_)
        {
            continue;
        }

        try
        {
            HandleNetMessage(pMsg_);
        }
        catch (...)
        {
            CMyLog::m_pLog->_XGSysLog("********HandleMessage exception!\n");
        }
    }
}

int CMSG_Handle_Center::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{

    ACE_UNUSED_ARG(tv);
    ACE_UNUSED_ARG(arg);

    NET_PACKET_MSG* pMsg_ = NULL;
    get_message(pMsg_);
    if (!pMsg_)
    {
        return -1;
    }

    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

    if (timer_count_ % 2 == 0)
    {

    }
    else
    {
        //		pMsg_->msg_head.msg_type=SYS_MSG_CHECK_HEARTBEAT;
        //		put(pMsg_);
    }

    timer_count_++;
    if (timer_count_ > 10000)
    {
        timer_count_ = 0;
    }

    return 0;
}

int CMSG_Handle_Center::get_message(NET_PACKET_MSG *&pMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_lock_, -1);

    pMsg = msgVec_[m_nMsgBufferCount];

    m_nMsgBufferCount++;
    if (m_nMsgBufferCount > MAX_BUFFERD_MESSAGE_NUMBER - 1)
    {
        m_nMsgBufferCount = 0;
    }

    return 0;

}

int CMSG_Handle_Center::init_message_buffer()
{

    for (int i = 0; i < MAX_BUFFERD_MESSAGE_NUMBER; i++)
    {
        char* chMsgBuffer = msg_buffer_;
        NET_PACKET_MSG* pMsg = (NET_PACKET_MSG*) (chMsgBuffer + i * sizeof (NET_PACKET_MSG));
        msgVec_.push_back(pMsg);
    }

    return 0;
}

int CMSG_Handle_Center::HandleNetMessage(NET_PACKET_MSG *pMsg)
{

    if (!pMsg)
    {
        return -1;
    }

    switch (pMsg->msg_head.msg_type)
    {

        case SYS_MSG_SYSTEM_REGISTER_REQ:
            printf("handle client login...\n");
            handleClientLogin(pMsg);
            printf("handle client login ok...\n");
            break;

        case SYS_MSG_SYSTEM_MSG_KEEPLIVE:
            handleKeeplive(pMsg);
            break;

            //锟铰硷拷锟斤拷锟斤拷
        case SYS_MSG_EVENT_SUBSCRIBE:
            printf("handle event subscribe...\n");
            handle_event_subscribe(pMsg);
            printf("handle event subscribe ok...\n");
            break;

        case SYS_MSG_EVENT_EXIT:
            handle_event_exit(pMsg);
            break;

        case SYS_MSG_SYSEVENT_PUBLISH:
            handle_event_publish(pMsg);
            break;


        case SYS_MSG_QUERY_GATHERDATA:
            handleQueryGatherData(pMsg);
            break;

        case SYS_MSG_QUERY_GATHERPICDATA:
            handleQueryPicData(pMsg);
            break;

        case SYS_MSG_REGATHERDATA:
            handleRegatherData(pMsg);
            break;

        case SYS_MSG_DEVICE_CTRL:
            handleDeviceCtrl(pMsg);
            break;

		case CLIENT_EXCEPTIION_FREE:
			handleExceptionFree(pMsg);
			break;

        default:
            break;
    }

    return 0;

}

int CMSG_Handle_Center::handleKeeplive(NET_PACKET_MSG *pMsg)
{
    if (!pMsg)
    {
        return -1;
    }

    CMyLog::m_pLog->_XGSysLog("recv heartbeat ...\n");
    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }


    int nSocket = pMsg->msg_head.net_proxy[0];


    {
        //----------------------------------------------------------
        ACE_SOCK_Stream server((ACE_HANDLE) nSocket);
        char chMsgAck[1024 * 100] = {0};

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //锟斤拷头锟斤拷锟?
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK;
        pHead->packet_len = sizeof (T_KeepliveAck);


        sendLen += sizeof (NET_PACKET_HEAD);


        T_KeepliveAck* pAck = (T_KeepliveAck*) (chMsgAck + sendLen);
        pAck->nResult = 0;


        sendLen += sizeof (T_KeepliveAck);
        pHead->packet_len = sizeof (T_KeepliveAck);
        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8); //锟斤拷头锟斤拷锟?
        sendLen += 8;

        ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
        int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

    }

    return 0;
}

int CMSG_Handle_Center::handle_event_subscribe(NET_PACKET_MSG *pMsg)
{
    if (!pMsg)
    {
        return -1;
    }

    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    CCmdHandler_T* pHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];

    NET_EVENT_SUBSCRIBE* pEventSubscribe = (NET_EVENT_SUBSCRIBE*) pMsg->msg_body;

    printf("recv client subscribe %s...\n", pEventSubscribe->event_serial);

    EventGroup* pEventGroup = event_group_map[pEventSubscribe->event_serial];
    if (!pEventGroup)
    {
        EventGroup* pEventGroup = new EventGroup;
        strcpy(pEventGroup->group_serial, pEventSubscribe->event_serial);
        pEventGroup->handler_list.push_back(pHandler);

        printf("pEventSubscribe->event_serial:%s\n", pEventSubscribe->event_serial);
        event_group_map[pEventSubscribe->event_serial] = pEventGroup;
    }
    else
    {
        pEventGroup->handler_list.push_back(pHandler);
    }

    int nSocket = pMsg->msg_head.net_proxy[0];

    ACE_SOCK_Stream server((ACE_HANDLE) nSocket);
    char chMsgAck[1024 * 100] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_EVENT_SUBSCRIBE_ACK;
    pHead->packet_len = sizeof (NET_EVENT_SUBSCRIBE_ACK);

    sendLen += sizeof (NET_PACKET_HEAD);



    NET_EVENT_SUBSCRIBE_ACK* pSetAck = (NET_EVENT_SUBSCRIBE_ACK*) (chMsgAck + sendLen);
    pSetAck->result = 0;

    sendLen += sizeof (NET_EVENT_SUBSCRIBE_ACK);

    pHead->packet_len = sizeof (NET_EVENT_SUBSCRIBE_ACK);
    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

    printf("subscribe!\n");

    return 0;
}

int CMSG_Handle_Center::handle_event_exit(NET_PACKET_MSG *pMsg)
{
    if (!pMsg)
    {
        return -1;
    }

    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    CCmdHandler_T* pHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];

    if (event_group_map.size() > 0)
    {
        std::map<string, EventGroup*>::iterator iter;
        for (iter = event_group_map.begin(); iter != event_group_map.end(); iter++)
        {
            EventGroup* pEventGroup = iter->second;
            if (pEventGroup)
            {
                if (pEventGroup)
                {
                    pEventGroup->handler_list.remove(pHandler);
                }
            }
        }
    }

    printf("exit subscribe!\n");
    return 0;
}

int CMSG_Handle_Center::handle_event_publish(NET_PACKET_MSG *pMsg)
{
    CMyLog::m_pLog->_XGSysLog("recv event publish!\n");
    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    CCmdHandler_T* pPublishHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];


    NET_EVENT_UPLOAD_CUSTOMSDATA* pEventData = (NET_EVENT_UPLOAD_CUSTOMSDATA*) pMsg->msg_body;
    EventGroup* pEventGroup = event_group_map[pEventData->main_type];
    printf("pEventData->main_type:%s\n", pEventData->main_type);

    if (!pEventGroup)
    {
        return -1;
    }

    if (pEventGroup)
    {
        if (pEventGroup->handler_list.size() > 0)
        {
            std::list<CCmdHandler_T*>::iterator iter;
            for (iter = pEventGroup->handler_list.begin(); iter != pEventGroup->handler_list.end(); iter++)
            {
                CCmdHandler_T* pHandler = *iter;
                if (pHandler != pPublishHandler)
                {
                    pHandler->send_message(pMsg);
                    CMyLog::m_pLog->_XGSysLog("publish event ......%d !\n", pEventGroup->handler_list.size());
                }
            }
        }
    }

    return 0;
}

int CMSG_Handle_Center::handleClientLogin(NET_PACKET_MSG* pMsg)
{
    if (!pMsg)
    {
        return -1;
    }

    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    LOG_IN* pUserInfo = (LOG_IN*) pMsg->msg_body;

    int nUserID = m_pSQLDataAccess->log_in(pUserInfo->user_name, pUserInfo->pass_word);

    if (nUserID < 1)
    {
        int nSocket = pMsg->msg_head.net_proxy[0];
        ACE_SOCK_Stream server((ACE_HANDLE) nSocket);
        char chMsgAck[1024 * 40] = {0};

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_ACK;
        pHead->packet_len = sizeof (LOG_IN_ACK);

        sendLen += sizeof (NET_PACKET_HEAD);



        LOG_IN_ACK* pSetAck = (LOG_IN_ACK*) (chMsgAck + sendLen);
        pSetAck->result = 0;

        sendLen += sizeof (LOG_IN_ACK);

        pHead->packet_len = sizeof (LOG_IN_ACK);
        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
        sendLen += 8;

        ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
        int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

        return 0;
    }

    MONITORPORTVec portVec;
    portVec.reserve(100);

    m_pSQLDataAccess->query_monitor_Port(nUserID, portVec);


    int nSocket = pMsg->msg_head.net_proxy[0];

    unsigned int nSendTimes = portVec.size() / MAX_RECORD_NUMBER;
    float fSendTimes = (float) portVec.size() / MAX_RECORD_NUMBER;

    if (fSendTimes - nSendTimes > 0.00001)
    {
        nSendTimes = nSendTimes + 1;
    }

    if (nSendTimes == 0)
    {
        nSendTimes = nSendTimes + 1;
    }

    ACE_SOCK_Stream server((ACE_HANDLE) nSocket);
    for (unsigned int nn = 0; nn < nSendTimes; nn++)
    {

        char chMsgAck[1024 * 10] = {0};

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //包头数据
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_ACK;
        pHead->packet_len = sizeof (SYS_MSG_SYSTEM_REGISTER_ACK);

        sendLen += sizeof (NET_PACKET_HEAD);

        /* 返回对应的IP地址和端口号 */
        LOG_IN_ACK* pLogInAck = (LOG_IN_ACK*) (chMsgAck + sendLen);
        pLogInAck->result = 0;

        if (nn == nSendTimes - 1)
        {
            pLogInAck->is_end = 1;
            pLogInAck->monitor_num = portVec.size() - nn*MAX_RECORD_NUMBER;

            sendLen += sizeof (LOG_IN_ACK) + pLogInAck->monitor_num * sizeof (MONITOR_PORT);
            pHead->packet_len = sizeof (LOG_IN_ACK) + pLogInAck->monitor_num * sizeof (MONITOR_PORT);

        }
        else
        {
            pLogInAck->is_end = 0;
            pLogInAck->monitor_num = MAX_RECORD_NUMBER;

            sendLen += sizeof (LOG_IN_ACK) + pLogInAck->monitor_num * sizeof (MONITOR_PORT);
            pHead->packet_len = sizeof (LOG_IN_ACK) + pLogInAck->monitor_num * sizeof (MONITOR_PORT);

        }

        int nSendRecord = portVec.size() - nn*MAX_RECORD_NUMBER;
        if (nSendRecord > MAX_RECORD_NUMBER)
        {
            nSendRecord = MAX_RECORD_NUMBER;
        }

        for (int j = 0; j < nSendRecord; j++)
        {
            MONITOR_PORT monitor_port = portVec[j + nn * MAX_RECORD_NUMBER];
            memcpy(&pLogInAck->port_info[j], &monitor_port, sizeof (MONITOR_PORT));
        }


        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
        sendLen += 8;

        ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
        int nRet = server.send_n(chMsgAck, sendLen, &expire_time);


        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10 * 1000;
        select(0, 0, NULL, NULL, &tv);
    }

    portVec.clear();


    return 0;
}

int CMSG_Handle_Center::handleQueryGatherData(NET_PACKET_MSG* pMsg)
{

    if (!pMsg)
    {
        return -1;
    }

    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    NET_QUERY_GATHERINFO* pQueryGatherInfo = (NET_QUERY_GATHERINFO*) pMsg->msg_body;


    CMyLog::m_pLog->_XGSysLog("recv query gatherdata......!\n");

    GATHERINFOVec gatherVec;
    gatherVec.reserve(10000);

    m_pSQLDataAccess->queryAllGatherInfo(pQueryGatherInfo->szChnnl_No, pQueryGatherInfo->szBeginTime, pQueryGatherInfo->szStopTime, gatherVec);


    CMyLog::m_pLog->_XGSysLog("query gatherdata %d......!\n", gatherVec.size());

    int nSocket = pMsg->msg_head.net_proxy[0];

    unsigned int nSendTimes = gatherVec.size() / MAX_RECORD_NUMBER;
    float fSendTimes = (float) gatherVec.size() / MAX_RECORD_NUMBER;

    if (fSendTimes - nSendTimes > 0.00001)
    {
        nSendTimes = nSendTimes + 1;
    }

    if (nSendTimes == 0)
    {
        nSendTimes = nSendTimes + 1;
    }

    ACE_SOCK_Stream server((ACE_HANDLE) nSocket);
    for (unsigned int nn = 0; nn < nSendTimes; nn++)
    {

        char chMsgAck[1024 * 50] = {0};

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //包头数据
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_QUERY_GATHERDATA_ACK;
        pHead->packet_len = sizeof (NET_GATHERINFO_ACK);

        sendLen += sizeof (NET_PACKET_HEAD);


        NET_GATHERINFO_ACK* pQueryGatherInfoAck = (NET_GATHERINFO_ACK*) (chMsgAck + sendLen);
        pQueryGatherInfoAck->nResult = 0;

        if (nn == nSendTimes - 1)
        {
            pQueryGatherInfoAck->nIsEnd = 1;
            pQueryGatherInfoAck->nGatherInfoNum = gatherVec.size() - nn*MAX_RECORD_NUMBER;

            sendLen += sizeof (NET_GATHERINFO_ACK) + pQueryGatherInfoAck->nGatherInfoNum * sizeof (QUERY_GATHER_INFO);
            pHead->packet_len = sizeof (NET_GATHERINFO_ACK) + pQueryGatherInfoAck->nGatherInfoNum * sizeof (QUERY_GATHER_INFO);

        }
        else
        {
            pQueryGatherInfoAck->nIsEnd = 0;
            pQueryGatherInfoAck->nGatherInfoNum = MAX_RECORD_NUMBER;

            sendLen += sizeof (NET_GATHERINFO_ACK) + pQueryGatherInfoAck->nGatherInfoNum * sizeof (QUERY_GATHER_INFO);
            pHead->packet_len = sizeof (NET_GATHERINFO_ACK) + pQueryGatherInfoAck->nGatherInfoNum * sizeof (QUERY_GATHER_INFO);

        }

        int nSendRecord = gatherVec.size() - nn*MAX_RECORD_NUMBER;
        if (nSendRecord > MAX_RECORD_NUMBER)
        {
            nSendRecord = MAX_RECORD_NUMBER;
        }

        for (int j = 0; j < nSendRecord; j++)
        {
            QUERY_GATHER_INFO gather_info = gatherVec[j + nn * MAX_RECORD_NUMBER];
            memcpy(&pQueryGatherInfoAck->gatherInfo[j], &gather_info, sizeof (QUERY_GATHER_INFO));
        }


        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
        sendLen += 8;

        ACE_Time_Value expire_time = ACE_Time_Value(1, 0);
        int nRet = server.send_n(chMsgAck, sendLen, &expire_time);


        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1 * 1000;
        select(0, 0, NULL, NULL, &tv);
    }




    CMyLog::m_pLog->_XGSysLog("send gatherdata %d......!\n", gatherVec.size());
    gatherVec.clear();

    return 0;
}

int CMSG_Handle_Center::handleQueryPicData(NET_PACKET_MSG* pMsg)
{

    if (!pMsg)
    {
        return -1;
    }

    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    NET_CONTAPIC_REQ* pQueryContaPic = (NET_CONTAPIC_REQ*) pMsg->msg_body;


    GONTAPICVec picVec;
    picVec.reserve(6);

    m_pSQLDataAccess->queryContaPicInfo(pQueryContaPic->sequence_no, picVec);

    int nSocket = pMsg->msg_head.net_proxy[0];


    ACE_SOCK_Stream server((ACE_HANDLE) nSocket);

    char* chMsgAck = m_pPicStreamBuffer;

    {

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //包头数据
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_QUERY_GATHERPICDATA_ACK;
        pHead->packet_len = sizeof (NET_CONTAPIC_ACK);

        sendLen += sizeof (NET_PACKET_HEAD);


        NET_CONTAPIC_ACK* pQueryContaPicAck = (NET_CONTAPIC_ACK*) (chMsgAck + sendLen);
        memset(pQueryContaPicAck, 0, sizeof (NET_CONTAPIC_ACK));

        strcpy(pQueryContaPicAck->channel_no, pQueryContaPic->channel_no);
        strcpy(pQueryContaPicAck->sequence_no, pQueryContaPic->sequence_no);


        pQueryContaPicAck->stream_num = picVec.size();

        int nStreamOffset = 0;

        for (int i = 0; i < picVec.size(); i++)
        {
            pQueryContaPicAck->stream_info[i].stream_type = picVec[i].pic_pos;


            int nFileLen = 0;
            FILE* pImageFile = fopen(picVec[i].file_path, "rb");
            if (!pImageFile)
            {
                continue;
            }

            fseek(pImageFile, 0, SEEK_END);
            nFileLen = ftell(pImageFile);
            fseek(pImageFile, 0, SEEK_SET);

            if (nFileLen < 1)
            {
                continue;
            }

            int nRead = fread((char*) (pQueryContaPicAck->stream_buffer + nStreamOffset), 1, nFileLen, pImageFile);

            if (nRead != nFileLen)
            {
                fclose(pImageFile);
                continue;
            }

            fclose(pImageFile);

            pQueryContaPicAck->stream_info[i].off_set = nStreamOffset;
            pQueryContaPicAck->stream_info[i].stream_len = nRead;
            nStreamOffset += nRead;

            pQueryContaPicAck->stream_len += nRead;

        }

        pHead->packet_len = sizeof (NET_CONTAPIC_ACK) + pQueryContaPicAck->stream_len;
        sendLen += sizeof (NET_CONTAPIC_ACK) + pQueryContaPicAck->stream_len;

        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
        sendLen += 8;


        int nRet = server.send(chMsgAck, sendLen);


    }

    picVec.clear();



    return 0;
}

int CMSG_Handle_Center::handleRegatherData(NET_PACKET_MSG* pMsg)
{


    CMyLog::m_pLog->_XGSysLog("recv regather req!\n");
    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    CCmdHandler_T* pPublishHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];

    NET_REGATHERDATA_REQ* pReGatherReq = (NET_REGATHERDATA_REQ*) pMsg->msg_body;
    EventGroup* pEventGroup = event_group_map[pReGatherReq->channel_no];
    printf("pReGatherReq->channel_no:%s\n", pReGatherReq->channel_no);
    

    if (!pEventGroup)
    {
        return -1;
    }

    if (pEventGroup)
    {
        if (pEventGroup->handler_list.size() > 0)
        {
            std::list<CCmdHandler_T*>::iterator iter;
            for (iter = pEventGroup->handler_list.begin(); iter != pEventGroup->handler_list.end(); iter++)
            {
                CCmdHandler_T* pHandler = *iter;
                if (pHandler != pPublishHandler)    //2015-3-28
                {
                    pHandler->send_message(pMsg);
                    CMyLog::m_pLog->_XGSysLog("publish regather event ......%d !\n", pEventGroup->handler_list.size());
                }
            }
        }
    }

    return 0;

}

int CMSG_Handle_Center::handleDeviceCtrl(NET_PACKET_MSG* pMsg)
{
    CMyLog::m_pLog->_XGSysLog("recv device ctrl req!\n");
    int nProxyCount = pMsg->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    CCmdHandler_T* pPublishHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];

    CLIENT_COMMAND_INFO* pDeviceCtrlReq = (CLIENT_COMMAND_INFO*) pMsg->msg_body;
    EventGroup* pEventGroup = event_group_map[pDeviceCtrlReq->szChnnl_No];
    printf("pDeviceCtrlReq->szChnnl_No:%s\n", pDeviceCtrlReq->szChnnl_No);

    if (!pEventGroup)
    {
        return -1;
    }

    if (pEventGroup)
    {
        if (pEventGroup->handler_list.size() > 0)
        {
            std::list<CCmdHandler_T*>::iterator iter;
            for (iter = pEventGroup->handler_list.begin(); iter != pEventGroup->handler_list.end(); iter++)
            {
                CCmdHandler_T* pHandler = *iter;
                if (pHandler != pPublishHandler)
                {
                    pHandler->send_message(pMsg);
                    CMyLog::m_pLog->_XGSysLog("publish device control event ......%d !\n", pEventGroup->handler_list.size());
                }
            }
        }
    }

    return 0;
}

int CMSG_Handle_Center::handleExceptionFree(NET_PACKET_MSG* pMsg)
{
	CMyLog::m_pLog->_XGSysLog("recv exception free req!\n");
	int nProxyCount = pMsg->msg_head.proxy_count;
	if (nProxyCount < 2)
	{
		return -1;
	}

	CCmdHandler_T* pPublishHandler = (CCmdHandler_T*) pMsg->msg_head.net_proxy[nProxyCount - 1];

	CLIENT_EXCEPTION_FREE* pDeviceCtrlReq = (CLIENT_EXCEPTION_FREE*) pMsg->msg_body;
	EventGroup* pEventGroup = event_group_map[pDeviceCtrlReq->szChnnl_No];
	printf("pDeviceCtrlReq->szChnnl_No:%s\n", pDeviceCtrlReq->szChnnl_No);

	if (!pEventGroup)
	{
		return -1;
	}

	if (pEventGroup)
	{
		if (pEventGroup->handler_list.size() > 0)
		{
			std::list<CCmdHandler_T*>::iterator iter;
			for (iter = pEventGroup->handler_list.begin(); iter != pEventGroup->handler_list.end(); iter++)
			{
				CCmdHandler_T* pHandler = *iter;
				if (pHandler != pPublishHandler)
				{
					pHandler->send_message(pMsg);
					CMyLog::m_pLog->_XGSysLog("publish exception free event ......%d !\n", pEventGroup->handler_list.size());
				}
			}
		}
	}

	return 0;
}
