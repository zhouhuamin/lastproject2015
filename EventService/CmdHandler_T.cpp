// CmdHandler_T.cpp: implementation of the CCmdHandler_T class.
//
//////////////////////////////////////////////////////////////////////
#include "CmdHandler_T.h"
#include "Cmd_Acceptor.h"

#include "SysMessage.h"
#include "MSGHandleCenter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdHandler_T::CCmdHandler_T() : message_semaphore_(0)
{
    m_nObjID = 0;
    dwRecvBuffLen = 0;
    chRecvBuffer = new char[MAX_CMD_BUFFERLEN];
    chDest = new char[MAX_CMD_BUFFERLEN];

}

CCmdHandler_T::~CCmdHandler_T()
{
    delete [] chRecvBuffer;
    printf("......close connection!\n");
}

void CCmdHandler_T::destroy()
{


    {
        NET_PACKET_MSG* pMsg_ = NULL;
        CMSG_Handle_Center::get_message(pMsg_);
        if (!pMsg_)
        {
            return;
        }

        memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

        //拷贝初始报文
        pMsg_->msg_head.msg_type = SYS_MSG_EVENT_EXIT;

        //记录连接句柄
        pMsg_->msg_head.net_proxy[pMsg_->msg_head.proxy_count] = this->peer().get_handle();
        pMsg_->msg_head.proxy_count++;

        //记录处理对象句柄
        pMsg_->msg_head.net_proxy[pMsg_->msg_head.proxy_count] = (long) this;
        pMsg_->msg_head.proxy_count++;

        MSG_HANDLE_CENTER::instance()->put(pMsg_);
    }


    this->reactor()->remove_handler(this,
            ACE_Event_Handler::READ_MASK
            | ACE_Event_Handler::DONT_CALL);

    this->peer().close();
    CCmd_Acceptor::free_handler(m_nObjID);
    //	inherited::destroy();
}

static ACE_THR_FUNC_RETURN ThreadSendMessageFunc(void *lParam)
{
    CCmdHandler_T* pHandler = (CCmdHandler_T*) lParam;
    pHandler->send_message();
    return 0;
}

int CCmdHandler_T::open(void *void_acceptor)
{


    int rcvbuf = 2 * 1024 * 1024;
    int rcvbufsize = sizeof (int);
    
    setsockopt(this->peer().get_handle(), SOL_SOCKET, SO_RCVBUF, (char*) &rcvbuf,rcvbufsize);
    setsockopt(this->peer().get_handle(), SOL_SOCKET, SO_SNDBUF, (char*) &rcvbuf,rcvbufsize);



    dwRecvBuffLen = 0;
    //	this->activate (THR_DETACHED);
    if (this->reactor()->register_handler(this,
            ACE_Event_Handler::READ_MASK) == -1)
    {
        ACE_ERROR_RETURN((LM_ERROR,
                "(%P|%t) can't register with reactor\n"),
                -1);
    }

    ACE_Thread_Manager::instance()->spawn(ThreadSendMessageFunc, this);

    printf("recv socket is %d\n", (int) this->peer().get_handle());

    return 0;
}

int CCmdHandler_T::close(u_long flags)
{
    ACE_UNUSED_ARG(flags);
    this->destroy();
    return 0;
}

int CCmdHandler_T::handle_input(ACE_HANDLE handle)
{
    ACE_UNUSED_ARG(handle);

    return this->process();
}

int CCmdHandler_T::handle_close(ACE_HANDLE handle,
        ACE_Reactor_Mask mask)
{
    ACE_UNUSED_ARG(handle);
    ACE_UNUSED_ARG(mask);

    this->destroy();
    return 0;
}

int CCmdHandler_T::svc(void)
{
    while (!bStopFlag_)
    {
        if (this->process() == -1)
            return -1;
    }

    bStopFlag_ = true;
    return 0;
}

int CCmdHandler_T::stop(void)
{
    bStopFlag_ = true;
    return 0;
}

int CCmdHandler_T::process()
{
    char chMsg[512] = {0};
    ssize_t bytes_read;
    switch ((bytes_read = this->peer().recv(chRecvBuffer + dwRecvBuffLen, MAX_CMD_BUFFERLEN - dwRecvBuffLen)))
    {
        case -1:
            return -1;
        case 0:
            return -1;
        default:
            dwRecvBuffLen += bytes_read;

            int nOffset = 0;
            int nLen = 0;

            printf("recv data %d\n", dwRecvBuffLen);
            memset(chDest, 0, MAX_MSG_BODYLEN);
            while (VerifyRecvPacket(chRecvBuffer, chDest, dwRecvBuffLen, nOffset, nLen) == 0)
            {
                NET_PACKET_HEAD* pPacketHead = (NET_PACKET_HEAD*) (chDest + nOffset);


                if (nLen == pPacketHead->packet_len + sizeof (NET_PACKET_HEAD))
                {
                    printf("parse data ok...\n");

                    NET_PACKET_MSG* pMsg_ = NULL;
                    CMSG_Handle_Center::get_message(pMsg_);
                    if (!pMsg_)
                    {
                        return -1;
                    }

                    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

                    //拷贝初始报文
                    memcpy(pMsg_, chDest + nOffset, pPacketHead->packet_len + sizeof (NET_PACKET_HEAD));

                    //记录连接句柄
                    pMsg_->msg_head.net_proxy[pPacketHead->proxy_count] = this->peer().get_handle();
                    pMsg_->msg_head.proxy_count++;

                    //记录处理对象句柄
                    pMsg_->msg_head.net_proxy[pMsg_->msg_head.proxy_count] = (long) this;
                    pMsg_->msg_head.proxy_count++;

                    MSG_HANDLE_CENTER::instance()->put(pMsg_);
                }
            }
    }

    return 0;
}

int CCmdHandler_T::VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen)
{
    /*缓冲区长度小于最小帧长度*/
    if (nRecLen < sizeof (NET_PACKET_HEAD))
    {
        return -1;
    }

    if (nRecLen > MAX_MSG_BODYLEN)
    {
        return -1;
    }

    int nHeadPos = SearchHeadPos(chRecvBuffer, nRecLen);
    if (nHeadPos < 0)
    {
        return -1;
    }

    int nTailPos = SearchTailPos(chRecvBuffer, nRecLen);
    if (nTailPos < 0)
    {
        return -1;
    }

    printf("nTailPos %d---nHeadPos %d\n", nTailPos, nHeadPos);
    memcpy(chDest, chRecvBuffer + nHeadPos, (nTailPos - nHeadPos) + 8);

    if (nRecLen - nTailPos - 8 > 0)
    {
        //接收的数据可能是连续的包
        memcpy(chRecvBuffer, chRecvBuffer + nTailPos + 8, nRecLen - nTailPos - 8);
    }

    nOffset = nHeadPos = 8;
    nLen = nTailPos - nHeadPos;
    nRecLen = nRecLen - nTailPos - 8;

    return 0;
}

int CCmdHandler_T::SearchHeadPos(char *chBuffer, int nDataLen)
{
    int end, i, j;
    end = nDataLen - 8; /* 计算结束位置*/

    if (end > 0)
    {
        for (i = 0; i <= end; i++)
        {
            //循环比较
            for (j = i; chBuffer[j] == SYS_NET_MSGHEAD[j - i]; j++)
            {
                if (SYS_NET_MSGHEAD[j - i + 1] == '\0')
                {
                    return i; /* 找到了子字符串   */
                }
            }
        }
    }

    return -1;
}

int CCmdHandler_T::SearchTailPos(char *chBuffer, int nDataLen)
{
    int end, i, j;
    end = nDataLen - 8; /* 计算结束位置*/

    if (end > 0)
    {
        for (i = 0; i <= end; i++)
        {
            //循环比较
            for (j = i; chBuffer[j] == SYS_NET_MSGTAIL[j - i]; j++)
            {
                if (SYS_NET_MSGTAIL[j - i + 1] == '\0')
                {
                    return i; /* 找到了子字符串   */
                }
            }
        }
    }

    return -1;
}

int CCmdHandler_T::send_message(NET_PACKET_MSG *pMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, message_lock_, -1);
    message_list_.push_back(pMsg);
    message_semaphore_.release();

    return 0;
}

int CCmdHandler_T::send_message()
{
    thread_count_++;

    while (!bStopFlag_)
    {
        message_semaphore_.acquire();

        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, message_lock_, -1);
        if (message_list_.size() > 0)
        {
            NET_PACKET_MSG* pMsg = message_list_.front();
            message_list_.pop_front();

            if (!pMsg)
            {
                continue;
            }

            int packet_len = pMsg->msg_head.packet_len + sizeof (NET_PACKET_HEAD);
            ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);

            this->peer().send_n(SYS_NET_MSGHEAD, 8, &expire_time);
            this->peer().send_n(pMsg, packet_len, &expire_time);
            this->peer().send_n(SYS_NET_MSGTAIL, 8, &expire_time);
            
            printf("this->peer():%d\n", this->peer().get_handle());

        }

    }

    thread_count_--;
    return 0;
}

