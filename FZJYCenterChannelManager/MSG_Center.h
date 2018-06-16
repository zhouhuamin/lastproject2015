// MSG_Center.h: interface for the MSG_Task class.
#if !defined _MSG_CENTER_
#define _MSG_CENTER_

#include <stdint.h>
#include <list>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "ace/Signal.h"
#include "ace/streams.h"
#include "ace/Task.h"
#include "ace/Semaphore.h"
#include "MySQLConnectionPool.h"
#include "PlatProxy.h"


#define MAX_BUFFERD_MESSAGE_NUMBER             (30)
#define MAX_HANDLE_THREADS                        1
typedef unsigned char	BYTE;

struct T_ChannelStatus
{
    char  channel_id[32];
    int   connect_handle;
    int   last_active;
    int   is_active;
};

struct T_SequenceStatus
{
    char  channel_id[32];
    int   connect_handle;
    int   last_active;
};

struct T_CtrlCmd
{
    char szAreaID[16];
    char szChannelNo[16];
    char szIEType[4];
    char szSequenceNo[64];
    int nLen;
    char szCtrlData[];
};

struct structModifyInfo
{
	std::string AREA_ID;
	std::string CHNL_NO;
	std::string SEQ_NO;
	std::string IE_FLAG;
	std::string USER_NAME;
	std::string USER_ID;
	std::string CLIENT_FINISH_FLAG;
	std::string CLIENT_TIME;
	std::string MODIFY_CAR_NO;
	std::string MODIFY_CONTA_NO_F;
	std::string MODIFY_CONTA_TYPE_F;
	std::string MODIFY_CONTA_NO_B;
	std::string MODIFY_CONTA_TYPE_B;
	std::string MODIFY_CAR_TAIL;
	std::string MODIFY_WEIGHT;
	std::string MODIFY_FREE_FLAG;
	std::string MODIFY_FREE_RESON;
};

struct structCustomInfo
{
	std::string AREA_ID;
	std::string CHNL_NO;
	std::string SEQ_NO;
	std::string IE_FLAG;
	std::string SENDED_CUSTOM_FLAG;
	std::string SENDED_CUSTOM_TIME;
	std::string RECIEVED_CUSTOM_FLAG;
	std::string RECIEVED_CUSTOM_TIME;
	std::string CUSTOM_CONTROL_INFO;
	std::string CUSTOM_FREE_FLAG;
};

class CMSG_Center : public ACE_Task<ACE_SYNCH>
{
    // Instantiated with an MT synchronization trait.
public:

    enum
    {
        MAX_THREADS = 1
    };

    CMSG_Center();
    ~CMSG_Center(void);

    virtual int open();

    virtual int put(NET_PACKET_MSG* pMsg);

    virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

public:
    
    int HandleNetMessage(NET_PACKET_MSG *pPacket);
    int handleChannelRegister(NET_PACKET_MSG* pPacket);
    int handleUploadCustomsData(NET_PACKET_MSG* pPacket);
    int handleCustomsDataAck(NET_PACKET_MSG* pPacket);
    int handleCtrlCmd(NET_PACKET_MSG* pPacket);
    int handleContaPicData(NET_PACKET_MSG* pPacket);
    int handleRegatherData(NET_PACKET_MSG* pMsg);
    int publish_gather_event(T_Upload_Customs_Data* pUploadDataReq);
    int handleDeviceCtrl(NET_PACKET_MSG* pMsg);
    int RecordContaPicInfo(char* szSeqNo,int nPicType,char* szFileName);
    
    int handleChannelPassAck(T_CtrlCmd* pCtrlCmd);
    int publish_pass_ack(T_CtrlCmd* pCtrlCmd);
    
    
    int PackBasicCustomsData(char* szAraeID,char* szChannelNo,char* szIEType,char* szXML,int nXMLLen,char* szDestData,int& nPackedLen);
  
    
    int registerChannel(char* channel_id);
    int channel_exit(int handle);

    int ConnectToEventServer();

    int setEventhandle(int handle)
    {
        event_handle=handle;
    }
    
    int CheckUpdatedSequence();

	int ProcessMySql();

	int queryModifyRecord(structModifyInfo & modifyInfo);
	int UpdateModifyRecord(structModifyInfo & modifyInfo);
	int UpdateSendFlag(structModifyInfo & modifyInfo);
	int UpdateRecvFlag(structModifyInfo & modifyInfo);
	int SendPassInfo(structModifyInfo & modifyInfo);
  
    static int init_message_buffer();
    static int handleChannelKeeplive(NET_PACKET_MSG* pPacket);
    static int get_message(NET_PACKET_MSG*& pMsg);
    static ACE_THR_FUNC_RETURN ThreadHandleMessage(void *arg);
    static ACE_THR_FUNC_RETURN ThreadSuicide(void *arg);

    std::list<NET_PACKET_MSG*> msgToHandleList_;
    ACE_Semaphore msg_semaphore_;
    ACE_Thread_Mutex msg_handle_lock_;

    bool m_bStartOk;

private:

    int         event_handle;
    
    static bool m_bWork;
    static ACE_Thread_Mutex msg_lock_;
    static std::vector<NET_PACKET_MSG*> msgVec_;
    static int m_nMsgBufferCount;
    
    static ACE_Thread_Mutex channel_lock_;
    static std::map<string,T_ChannelStatus*> m_StaChannelStatusMap;


    CPlatformProxy*    m_pPlatformProxy;
   
    ACE_Thread_Mutex seq_lock_;
    std::map<string,T_SequenceStatus> m_StaChannelConnectMap;
  
};

typedef ACE_Singleton<CMSG_Center, ACE_Null_Mutex> MSG_CENTER;

#endif // !defined(AFX_MSG_TASK_H__69346340_854F_4162_801C_94B418746B85__INCLUDED_)
