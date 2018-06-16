// SQLDataAccess.h: interface for the DeviceControlServer class.
//
//////////////////////////////////////////////////////////////////////
#if !defined _SQL_DATA_ACCESS_H_
#define _SQL_DATA_ACCESS_H_


#include <map>
#include <vector>
#include "SysMessage.h"
#include "CppMysql.h"

using namespace std;


struct CONTA_PIC_INFO
{
    int pic_pos;
    char  file_path[256];
};


typedef std::vector<MONITOR_PORT> MONITORPORTVec;
typedef std::vector<QUERY_GATHER_INFO> GATHERINFOVec;
typedef std::vector<CONTA_PIC_INFO> GONTAPICVec;





class CSQLDataAccess {
public:
    int testDatabase();

    CSQLDataAccess();
    virtual ~CSQLDataAccess();

    int log_in(char* user_name, char* pass_word);
    int query_monitor_Port(int user_id, MONITORPORTVec& portVec);
    
    int query_gather_info(char* channel_id,char* begin_time,char* stop_time,GATHERINFOVec& gatherVec);
    int queryAllGatherInfo(char* channel_id, char* begin_time, char* stop_time,GATHERINFOVec& gatherVec);
    int queryContaPicInfo(char* sequence_no, GONTAPICVec& picVec);
    
    
private:
    int query_ic_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_car_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_trailer_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_conta_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_eseal_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_weight_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    int query_gps_info(char* sequence_no,QUERY_GATHER_INFO& gatherInfo);
    

   
};

#endif 
