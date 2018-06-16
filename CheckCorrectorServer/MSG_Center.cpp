// MSG_Center.cpp: implementation of the MSG_Task class.
//
//////////////////////////////////////////////////////////////////////

#include "ace/Select_Reactor.h"
#include "ace/SOCK_Stream.h"

#include "tinyXML/tinyxml.h"
#include "SimpleConfig.h"

#include "SysMessage.h"
#include "MSG_Center.h"
#include "Cmd_Acceptor.h"
#include "SysUtil.h"
#include "ProcessContaData.h"


ACE_Thread_Mutex CMSG_Center::msg_lock_;

std::vector<NET_PACKET_MSG*> CMSG_Center::msgVec_;
int CMSG_Center::m_nMsgBufferCount = 0;

ACE_Thread_Mutex CMSG_Center::server_lock_;
map<int, T_Server_Info*> CMSG_Center::m_StaServerHandleMap;
std::list<T_Server_Info*> CMSG_Center::m_StaServerHandleList;

CMSG_Center::~CMSG_Center()
{
    ACE_Reactor::instance()->cancel_timer(this);
}

CMSG_Center::CMSG_Center()
{


}

int CMSG_Center::open()
{

    //³õÊ¼»¯ÐÅÏ¢Êý¾Ý»º³åÇø£¬´Ë»º³åÇøÓÃÓÚCCtrlCmdReceiver_TÏò¸ÃÀà´«ÊäÊý¾Ý
    init_message_buffer();
    msg_semaphore_.acquire();


    m_bStartOk = false;


    ACE_Thread_Manager::instance()->spawn_n(MAX_HANDLE_THREADS, ThreadHandleMessage, this);

    //ACE_Thread_Manager::instance()->spawn_n(1, ThreadCheckRecoServer, this);



    //if (ACE_Reactor::instance()->schedule_timer(this, 0,
    //        ACE_Time_Value(2, 0),
    //        ACE_Time_Value(2, 0)) == -1)
    //{
    //    return -1;
    //}

    return 0;
}

int CMSG_Center::CheckRecoServer()
{
    int nTimeNow = time(NULL);

    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, CMSG_Center::server_lock_, -1);

    T_Server_Info* pServer = NULL;

    map<int, T_Server_Info*>::iterator iterMap;
    for (iterMap = CMSG_Center::m_StaServerHandleMap.begin(); iterMap != CMSG_Center::m_StaServerHandleMap.end(); iterMap++)
    {
        T_Server_Info* pInfo = iterMap->second;

        if (pInfo && nTimeNow - pInfo->last_active > 5)
        {
            pServer = pInfo;

            CMSG_Center::m_StaServerHandleList.remove(pInfo);
            CMSG_Center::m_StaServerHandleMap.erase(iterMap);

            printf("size of server map is %d,list is %d\n", CMSG_Center::m_StaServerHandleMap.size(), CMSG_Center::m_StaServerHandleList.size());

            break;
        }
    }





    if (pServer)
    {

        delete pServer;
        pServer = NULL;

    }
}

ACE_THR_FUNC_RETURN CMSG_Center::ThreadCheckRecoServer(void *arg)
{
    CMSG_Center* pTask = (CMSG_Center*) arg;
    while (1)
    {
        {
            pTask->CheckRecoServer();

        }

        SysUtil::SysSleep(500);
    }
}

ACE_THR_FUNC_RETURN CMSG_Center::ThreadHandleMessage(void *arg)
{
    CMSG_Center* pTask = (CMSG_Center*) arg;
    while (1)
    {
        pTask->msg_semaphore_.acquire();

        NET_PACKET_MSG* pMsg_ = NULL;
        {
            ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, pTask->msg_handle_lock_, 0);
            //printlog("Handle count : %d ****************************\n\n", CHA_Proactive_Acceptor::GetValidHandleCount());
            if (pTask->msgToHandleList_.size() == 0)
            {

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                select(0, 0, NULL, NULL, &tv);


                continue;
            }

            pMsg_ = pTask->msgToHandleList_.front();
            pTask->msgToHandleList_.pop_front();

        }

        if (!pMsg_)
        {
            continue;
        }

        //cast the pointer to Client_Handler*
        try
        {
            //printlog("+1\n");
            pTask->HandleNetMessage(pMsg_);
            //printlog("-1\n");
        } catch (...)
        {
            CMyLog::m_pLog->_XGSysLog("crash Message : %d ...\n", pMsg_->msg_head.msg_type);
            continue;
        }
    }
}

int CMSG_Center::client_exit(int handle)
{

    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, CMSG_Center::server_lock_, -1);

    T_Server_Info* pServer = NULL;

    map<int, T_Server_Info*>::iterator iterMap;
    for (iterMap = CMSG_Center::m_StaServerHandleMap.begin(); iterMap != CMSG_Center::m_StaServerHandleMap.end(); iterMap++)
    {
        T_Server_Info* pInfo = iterMap->second;

        if (pInfo && handle == pInfo->connect_handle)
        {
            pServer = pInfo;

            CMSG_Center::m_StaServerHandleList.remove(pInfo);
            CMSG_Center::m_StaServerHandleMap.erase(iterMap);

            printf("size of server map is %d,list is %d\n", CMSG_Center::m_StaServerHandleMap.size(), CMSG_Center::m_StaServerHandleList.size());

            break;
        }
    }

    if (pServer)
    {

        delete pServer;
        pServer = NULL;

    }
    return 0;
}

int CMSG_Center::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
    ACE_UNUSED_ARG(tv);
    ACE_UNUSED_ARG(arg);

    NET_PACKET_MSG* pMsg = NULL;
    get_message(pMsg);
    if (!pMsg)
    {
        return -1;
    }


    //    put(pMsg);
    return 0;
}

int CMSG_Center::get_message(NET_PACKET_MSG *&pMsg)
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

int CMSG_Center::put(NET_PACKET_MSG* pMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_handle_lock_, -1);
    msgToHandleList_.push_back(pMsg);

    if (msgToHandleList_.size() > 10)
    {
        CMyLog::m_pLog->_XGSysLog("--------msg to handle %d! \n", msgToHandleList_.size());
    }

    msg_semaphore_.release();
    return 0;
}

int CMSG_Center::init_message_buffer()
{
    msgVec_.reserve(MAX_BUFFERD_MESSAGE_NUMBER);

    for (int i = 0; i < MAX_BUFFERD_MESSAGE_NUMBER; i++)
    {

        NET_PACKET_MSG* pMsg = new NET_PACKET_MSG;
        msgVec_.push_back(pMsg);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10 * 1000;
        select(0, 0, NULL, NULL, &tv);
    }

    return 0;
}

int CMSG_Center::HandleNetMessage(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    switch (pPacket->msg_head.msg_type)
    {
        case SYS_MSG_SYSTEM_REGISTER_REQ:
            handleClientRegister(pPacket);
            break;

        case SYS_MSG_SYSTEM_MSG_KEEPLIVE:
            handleChannelKeeplive(pPacket);
            break;


        case SYS_MSG_SYSTEM_MSG_RECO_REQ:
            handleContaReco(pPacket);
            break;

        case SYS_MSG_SYSTEM_MSG_RECO_ACK:
            handleContaRecoResult(pPacket);
            break;

		case SYS_MSG_CONTAID_CHECK_BIT_REQ:
			ProcessContaIDCheckBit(pPacket);
			break;

		case SYS_MSG_CONTAID_CORRECTED_REQ:
			ProcessContaIDCorrected(pPacket);
			break;

		case SYS_MSG_CONTAID_COUNT_REQ:
			ProcessContaIDCount(pPacket);
			break;

    }

    return 0;
}

int CMSG_Center::handleChannelKeeplive(NET_PACKET_MSG* pPacket)
{
    int nProxyCount = pPacket->msg_head.proxy_count;
    if (nProxyCount < 1)
    {
        return -1;
    }

    int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];


    NET_KEEP_LIVE* pKeepLive = (NET_KEEP_LIVE*) pPacket->msg_body;


    {

        if (pKeepLive->client_type == 9)
        {
            ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, server_lock_, -1);
            if (m_StaServerHandleMap[connect_handle])
            {
                m_StaServerHandleMap[connect_handle]->last_active = time(NULL);
            }

            //CMyLog::m_pLog->_XGSysLog("recv server %d keeplive,current server is %d\n", connect_handle, m_StaServerHandleMap.size());
        }

    }


    if (connect_handle > 0)
    {

        ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
        char chMsgAck[1024] = {0};

        int sendLen = 0;
        memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //ï¿½ï¿½Í·ï¿½ï¿½ï¿?
        sendLen += 8;

        NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
        pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK;
        pHead->packet_len = 0;


        sendLen += sizeof (NET_PACKET_HEAD);

        memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
        sendLen += 8;

        ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
        int nRet = server.send_n(chMsgAck, sendLen, &expire_time);
    }


    return 0;

    //    CMyLog::m_pLog->_XGSysLog("recv connect %d keeplive...\n", connect_handle);

}

int CMSG_Center::handleClientRegister(NET_PACKET_MSG *pPacket)
{

    if (!pPacket)
    {
        return -1;
    }


    int nProxyCount = pPacket->msg_head.proxy_count;
    //CMyLog::m_pLog->_XGSysLog("nProxyCount: %d\n", nProxyCount);
    if (nProxyCount < 1)
    {
        return -1;
    }
    
    

    int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];


    T_Register_Req* pClientType = (T_Register_Req*) pPacket->msg_body;
    if (pClientType->client_type == 9)
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, server_lock_, -1);
        if (!m_StaServerHandleMap[connect_handle])
        {
            T_Server_Info* pServerInfo = new T_Server_Info;
            pServerInfo->connect_handle = connect_handle;
            pServerInfo->last_active = time(NULL);

            m_StaServerHandleMap[connect_handle] = pServerInfo;
            m_StaServerHandleList.push_back(pServerInfo);

        } else
        {
            m_StaServerHandleMap[connect_handle]->last_active = time(NULL);
        }


        //CMyLog::m_pLog->_XGSysLog("recv server %d,current server connection is %d\n", connect_handle, m_StaServerHandleMap.size());
    }



    ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
    char chMsgAck[256] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //ï¿½ï¿½Í·ï¿½ï¿½ï¿?
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_ACK;
    pHead->packet_len = sizeof (T_Register_Ack);
    sendLen += sizeof (NET_PACKET_HEAD);

    T_Register_Ack* pRegisterAck = (T_Register_Ack*) (chMsgAck + sendLen);
    pRegisterAck->reg_result = 0;

    sendLen += sizeof (T_Register_Ack);


    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);
    printf("send client ack %d\n", nRet);


}

int CMSG_Center::handleContaReco(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    printf("recv conta req......\n");


    int nHandle = -1;

    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, server_lock_, -1);
        if (m_StaServerHandleList.size() > 0)
        {
            T_Server_Info* pServerInfo = m_StaServerHandleList.front();
            nHandle = pServerInfo->connect_handle;
            m_StaServerHandleList.pop_front();

            m_StaServerHandleList.push_back(pServerInfo);
        }

    }

    if (nHandle < 0)
    {
        return -1;
    }


    printf("dospatch conta task to %d......\n", nHandle);


    ACE_Time_Value expire_time = ACE_Time_Value(2, 0);


    ACE_SOCK_Stream server((ACE_HANDLE) nHandle);
    char chMsgAck[1024] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    int nRet = server.send_n(chMsgAck, 8, &expire_time);

    nRet += server.send_n(pPacket, sizeof (NET_PACKET_HEAD) + pPacket->msg_head.packet_len, &expire_time);


    memcpy(chMsgAck, SYS_NET_MSGTAIL, 8);
    nRet += server.send_n(chMsgAck, 8, &expire_time);


}

int CMSG_Center::handleContaRecoResult(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    printf("recv conta result......\n");


    int nProxyCount = pPacket->msg_head.proxy_count;
    if (nProxyCount < 2)
    {
        return -1;
    }

    int connect_handle = pPacket->msg_head.net_proxy[0];
    
    CMyLog::m_pLog->_XGSysLog("recv proxy_count is %d,%d\n", pPacket->msg_head.net_proxy[0],pPacket->msg_head.net_proxy[1]);


    T_ContaRecoResult* pRecoResultAck = (T_ContaRecoResult*) pPacket->msg_body;
    printf("pic %s reco result no: %s status :%s\n", pRecoResultAck->req_sequence, pRecoResultAck->conta_id.ID, pRecoResultAck->conta_id.Type);


    //send result to client
    ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
    char chMsgAck[1024] = {0};

    ACE_Time_Value expire_time = ACE_Time_Value(2, 0);



    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    int nRet = server.send_n(chMsgAck, 8, &expire_time);

    nRet += server.send_n(pPacket, sizeof (NET_PACKET_HEAD) + pPacket->msg_head.packet_len, &expire_time);


    memcpy(chMsgAck, SYS_NET_MSGTAIL, 8);
    nRet += server.send_n(chMsgAck, 8, &expire_time);


}

int CMSG_Center::ProcessContaIDCheckBit(NET_PACKET_MSG *pPacket)
{
	if (!pPacket)
	{
		return -1;
	}

	int nProxyCount = pPacket->msg_head.proxy_count;
	if (nProxyCount < 1)
	{
		return -1;
	}
	int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];


	JZ_ContaIDCheckBitResponse tmpResponse = {0};
	JZ_ContaIDCheckBitRequest* pRequest = (JZ_ContaIDCheckBitRequest*) pPacket->msg_body;
	ProcessContaData processData;
	processData.GetContaIDCheckBit(pRequest, &tmpResponse);

	//send ack to reco manager
	ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
	char chMsgAck[10240] = {0};

	int sendLen = 0;
	memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //ï¿½ï¿½Í·ï¿½ï¿½ï¿?
	sendLen += 8;

	NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);

	pHead->proxy_count = nProxyCount - 1;

	printf("........proxy count %d...\n", pHead->proxy_count);

	for (int i = 0; i < nProxyCount - 1; i++)
	{
		pHead->net_proxy[i] = pPacket->msg_head.net_proxy[i];
	}


	pHead->msg_type = SYS_MSG_CONTAID_CHECK_BIT_RES;
	pHead->packet_len = sizeof (JZ_ContaIDCheckBitResponse);


	sendLen += sizeof (NET_PACKET_HEAD);


	JZ_ContaIDCheckBitResponse* pResponse = (JZ_ContaIDCheckBitResponse*) (chMsgAck + sendLen);

	//strcpy(pRecoResultAck->ContaID, tmpResponse.ContaID);
	//pRecoResultAck->VerifyCode = tmpResponse.VerifyCode;
	*pResponse	= tmpResponse;

	sendLen += sizeof (JZ_ContaIDCheckBitResponse);

	pHead->packet_len = sizeof (JZ_ContaIDCheckBitResponse);
	memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
	sendLen += 8;

	ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
	int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

	return 0;
}

int CMSG_Center::ProcessContaIDCorrected(NET_PACKET_MSG *pPacket)
{
	if (!pPacket)
	{
		return -1;
	}

	int nProxyCount = pPacket->msg_head.proxy_count;
	if (nProxyCount < 1)
	{
		return -1;
	}
	int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];


	JZ_ContaIDCorrectionResponse tmpResponse = {0};
	JZ_ContaIDCorrectionRequest* pRequest	 = (JZ_ContaIDCorrectionRequest*) pPacket->msg_body;
	ProcessContaData processData;
	processData.GetContaIDCorrected(pRequest, &tmpResponse);

	//send ack to reco manager
	ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
	char chMsgAck[10240] = {0};

	int sendLen = 0;
	memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //ï¿½ï¿½Í·ï¿½ï¿½ï¿?
	sendLen += 8;

	NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);

	pHead->proxy_count = nProxyCount - 1;

	printf("........proxy count %d...\n", pHead->proxy_count);

	for (int i = 0; i < nProxyCount - 1; i++)
	{
		pHead->net_proxy[i] = pPacket->msg_head.net_proxy[i];
	}


	pHead->msg_type = SYS_MSG_CONTAID_CORRECTED_RES;
	pHead->packet_len = sizeof (JZ_ContaIDCorrectionResponse);


	sendLen += sizeof (NET_PACKET_HEAD);

	JZ_ContaIDCorrectionResponse* pResponse = (JZ_ContaIDCorrectionResponse*) (chMsgAck + sendLen);
	*pResponse							    = tmpResponse;

	sendLen += sizeof (JZ_ContaIDCorrectionResponse);

	pHead->packet_len = sizeof (JZ_ContaIDCorrectionResponse);
	memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
	sendLen += 8;

	ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
	int nRet = server.send_n(chMsgAck, sendLen, &expire_time);
	return 0;
}

int CMSG_Center::ProcessContaIDCount(NET_PACKET_MSG *pPacket)
{
	if (!pPacket)
	{
		return -1;
	}

	int nProxyCount = pPacket->msg_head.proxy_count;
	if (nProxyCount < 1)
	{
		return -1;
	}
	int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];


	JZ_ContaCountResponse tmpResponse = {0};
	JZ_ContaIDCountRequest* pRequest = (JZ_ContaIDCountRequest*) pPacket->msg_body;

	ProcessContaData processData;
	processData.GetContaIDCount(pRequest, &tmpResponse);

	//send ack to reco manager
	ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
	char chMsgAck[10240] = {0};

	int sendLen = 0;
	memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //ï¿½ï¿½Í·ï¿½ï¿½ï¿?
	sendLen += 8;

	NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);

	pHead->proxy_count = nProxyCount - 1;

	printf("........proxy count %d...\n", pHead->proxy_count);

	for (int i = 0; i < nProxyCount - 1; i++)
	{
		pHead->net_proxy[i] = pPacket->msg_head.net_proxy[i];
	}


	pHead->msg_type		= SYS_MSG_CONTAID_COUNT_RES;
	pHead->packet_len	= sizeof (JZ_ContaCountResponse);


	sendLen += sizeof (NET_PACKET_HEAD);

	JZ_ContaCountResponse* pResponse = (JZ_ContaCountResponse*) (chMsgAck + sendLen);
	*pResponse						 = tmpResponse;

	sendLen += sizeof (JZ_ContaCountResponse);

	pHead->packet_len = sizeof (JZ_ContaCountResponse);
	memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
	sendLen += 8;

	ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
	int nRet = server.send_n(chMsgAck, sendLen, &expire_time);
	return 0;
}

