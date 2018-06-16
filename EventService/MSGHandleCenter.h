// MSGHandleCenter.h: interface for the CMSG_Handle_Center class.
#if !defined _MSG_HANDLE_CENTER_
#define _MSG_HANDLE_CENTER_

#ifdef WIN32
#include "stdafx.h"
#pragma comment(lib,"libmySQL.lib")

#else
#endif
#include "ace/Reactor.h"
#include <ace/Semaphore.h>
#include "ace/Guard_T.h"
#include "ace/Task.h"
#include "SQLDataAccess.h"
#include "CmdHandler_T.h"



#include <vector>
#include <list>
#include <map>
using namespace std;

#define MAX_BUFFERD_MESSAGE_NUMBER                (1000)


struct EventGroup
{
    char group_serial[64];
    std::list<CCmdHandler_T*> handler_list;
};

class CMSG_Handle_Center : public ACE_Task<ACE_SYNCH>
{
    // Instantiated with an MT synchronization trait.
public:

    enum
    {
        MAX_THREADS = 1
    };

    ~CMSG_Handle_Center(void);

    virtual int open(void * = 0)
    {
        timer_count_ = 0;
       
        m_pPicStreamBuffer=new char[6*1024*1024];

        m_pSQLDataAccess = new CSQLDataAccess;

        init_message_buffer();
        msg_semaphore_.acquire();


        this->activate(THR_JOINABLE);

        if (ACE_Reactor::instance()->schedule_timer(this,
            0,
            ACE_Time_Value(60, 0),
            ACE_Time_Value(60, 0)) == -1)
        {
            return -1;
        }


        return 0;
    }

    virtual int put(NET_PACKET_MSG* pMsg)
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_handle_lock_, -1);
        msgToHandleList_.push_back(pMsg);
        if (msgToHandleList_.size() > 5)
        {
            printf("size of msgToHandleList_ is %d\n", msgToHandleList_.size());
        }

        msg_semaphore_.release();
        return 0;
    }

    virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

    virtual int svc(void);

public:
    int handleClientLogin(NET_PACKET_MSG* pMsg);
    int handle_event_publish(NET_PACKET_MSG* pMsg);
    int handle_event_exit(NET_PACKET_MSG* pMsg);
    int handle_event_subscribe(NET_PACKET_MSG* pMsg);
    int handleKeeplive(NET_PACKET_MSG *pMsg);
    int handleQueryGatherData(NET_PACKET_MSG* pMsg);
    int handleQueryPicData(NET_PACKET_MSG* pMsg);
    
    int handleRegatherData(NET_PACKET_MSG* pMsg);
    int handleDeviceCtrl(NET_PACKET_MSG* pMsg);

	// 2015-5-6
	int handleExceptionFree(NET_PACKET_MSG* pMsg);


    int HandleNetMessage(NET_PACKET_MSG *pMsg);

    static int init_message_buffer();
    static int get_message(NET_PACKET_MSG*& pMsg);
  


    std::list<NET_PACKET_MSG*> msgToHandleList_;
    ACE_Semaphore msg_semaphore_;
    ACE_Thread_Mutex msg_handle_lock_;

private:
    CSQLDataAccess* m_pSQLDataAccess;


    static bool m_bWork;
    static ACE_Thread_Mutex msg_lock_;
    static std::vector<NET_PACKET_MSG*> msgVec_;
    static int m_nMsgBufferCount;
    static char msg_buffer_[sizeof (NET_PACKET_MSG) * MAX_BUFFERD_MESSAGE_NUMBER];

    static std::map<string, EventGroup*> event_group_map;

    int timer_count_;
    
    
    char*    m_pPicStreamBuffer;

};


typedef ACE_Singleton<CMSG_Handle_Center, ACE_Null_Mutex> MSG_HANDLE_CENTER;


#endif // !defined(AFX_MSG_TASK_H__69346340_854F_4162_801C_94B418746B85__INCLUDED_)




















































































