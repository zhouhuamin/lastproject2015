// SQLDataAccess.cpp: implementation of the DeviceControlServer class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "stdafx.h"
#else
#endif

#include "SQLDataAccess.h"
#include "SimpleConfig.h"
#include "SysMessage.h"
#include "MySQLConnectionPool.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class CSqlAutoLock
{
private:
    CppMySQL3DB *pDatabase_;
public:

    CSqlAutoLock(CppMySQL3DB* pDatabase)
    {
        pDatabase_ = pDatabase;
    };

    ~CSqlAutoLock()
    {
        if (pDatabase_ != NULL)
            pDatabase_->setIdle();
    };
};

CSQLDataAccess::CSQLDataAccess()
{

    CMySQLConnectionPool::Init(CSimpleConfig::DATABASE_SERVER_IP, CSimpleConfig::DATABASE_USER, CSimpleConfig::DATABASE_PASSWORD
            , "GatherGateSys", CSimpleConfig::DATABASE_SERVER_PORT);


}

CSQLDataAccess::~CSQLDataAccess()
{

}

int CSQLDataAccess::testDatabase()
{

    return 0;
}

int CSQLDataAccess::log_in(char* user_name, char* pass_word)
{
    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM  T_USERINFO WHERE UserName ='%s' AND PassWord = '%s' "
            , user_name
            , pass_word);



    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);

    while (!query.eof())
    {
        int nUserID = query.getIntField("UserID");
        return nUserID;
    }

    return -1;
}

int CSQLDataAccess::query_monitor_Port(int user_id, MONITORPORTVec& portVec)
{


    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }


    CSqlAutoLock autolock(m_pDatabase);

    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_USER_CHANNEL userchannel,T_CHANNE_INFO channel\
                         WHERE userchannel.ChannelID=channel.CHANNEL_ID AND userchannel.UserID = %d "
            , user_id);



    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);

    char area_id[24] = {0};
    char channel_no[24] = {0};
    char i_e_type[4] = {0};


    while (!query.eof())
    {
        MONITOR_PORT port;
        memset(&port, 0, sizeof (MONITOR_PORT));

        strcpy(area_id, query.getStringField("AREA_ID"));
        strcpy(channel_no, query.getStringField("CHANNEL_NO"));
        strcpy(i_e_type, query.getStringField("I_E_TYPE"));

        sprintf(port.channel_no, "%s%s%s", area_id, channel_no, i_e_type);

        sprintf(port.area_name, "%s", query.getStringField("AREA_NAME"));
        sprintf(port.channel_name, "%s", query.getStringField("CHANNEL_NAME"));

        port.status = 0;

        portVec.push_back(port);

        query.nextRow();
    }

    return portVec.size();
}

int CSQLDataAccess::query_gather_info(char* channel_id, char* begin_time, char* stop_time, GATHERINFOVec& gatherVec)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    string strChannel(channel_id);
    string strArea = strChannel.substr(0, 10);
    string strChannelNo = strChannel.substr(10, 10);
    string strIEType = strChannel.substr(20, 1);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO info,T_GATHERINFO_CAR car,T_GATHERINFO_CONTA conta,T_GATHERINFO_GPS gps,T_GATHERINFO_IC ic,T_GATHERINFO_SEAL seal,\
                         T_GATHERINFO_TRAILER trailer,T_GATHERINFO_WEIGHT weight\
                         WHERE info.SEQUENCE_NO=car.SEQUENCE_NO AND info.SEQUENCE_NO=conta.SEQUENCE_NO AND info.SEQUENCE_NO=gps.SEQUENCE_NO\
                         AND info.SEQUENCE_NO=ic.SEQUENCE_NO  AND info.SEQUENCE_NO=seal.SEQUENCE_NO  AND info.SEQUENCE_NO=trailer.SEQUENCE_NO\
                         AND info.SEQUENCE_NO=weight.SEQUENCE_NO AND  info.GATHER_AREA='%s' AND info.CHANNEL_NO='%s' AND info.I_E_TYPE='%s'\
                         AND info.PASS_TIME <='%s' AND info.PASS_TIME>='%s' "
            , strArea.c_str()
            , strChannelNo.c_str()
            , strIEType.c_str()
            , stop_time
            , begin_time);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    while (!query.eof())
    {
        QUERY_GATHER_INFO gatherInfo;
        memset(&gatherInfo, 0, sizeof (QUERY_GATHER_INFO));

        gatherInfo.iArrange_Type = 2;
        strcpy(gatherInfo.szChannelNo, channel_id);
        strcpy(gatherInfo.szSeq_No, query.getStringField("SEQUENCE_NO"));
        gatherInfo.iMODI_FLAG = query.getIntField("IS_REGATHER");
        gatherInfo.iREL_TYPE = query.getIntField("PASS_MODE");
        
        
        
        //ic
        strcpy(gatherInfo.IC.szDR_IC_NO, query.getStringField("DR_IC_NO"));
        strcpy(gatherInfo.IC.szIC_DR_CUSTOMS_NO, query.getStringField("IC_DR_CUSTOMS_NO"));
        strcpy(gatherInfo.IC.szIC_CO_CUSTOMS_NO, query.getStringField("IC_CO_CUSTOMS_NO"));
        strcpy(gatherInfo.IC.szIC_BILL_NO, query.getStringField("IC_BILL_NO"));
        gatherInfo.IC.lIC_GROSS_WT = atoi(query.getStringField("IC_GROSS_WT"));
        strcpy(gatherInfo.IC.szIC_VE_CUSTOMS_NO, query.getStringField("IC_VE_CUSTOMS_NO"));

        strcpy(gatherInfo.IC.szIC_VE_NAME, query.getStringField("IC_VE_NAME"));

        strcpy(gatherInfo.IC.szIC_CONTA_ID, query.getStringField("IC_CONTA_ID"));
        strcpy(gatherInfo.IC.szIC_ESEAL_ID, query.getStringField("IC_ESEAL_ID"));

        strcpy(gatherInfo.IC.szIC_EX_DATA, query.getStringField("IC_EX_DATA"));
        strcpy(gatherInfo.IC.szIC_BUSS_TYPE, query.getStringField("IC_BUSS_TYPE"));
        
        
        //car
        strcpy(gatherInfo.CAR.szVE_NAME, query.getStringField("VE_NAME"));
        strcpy(gatherInfo.CAR.szCAR_EC_NO, query.getStringField("CAR_EC_NO"));
        gatherInfo.CAR.lVE_WT = atoi(query.getStringField("VE_WEIGHT"));

        strcpy(gatherInfo.CAR.szVE_CUSTOMS_NO, query.getStringField("VE_CUSTOMS_NO"));
        
        //trailer
        strcpy(gatherInfo.TRAILER.szTR_NAME, query.getStringField("TR_NAME"));
        strcpy(gatherInfo.TRAILER.szTR_EC_NO, query.getStringField("TR_EC_NO"));
        gatherInfo.TRAILER.lTR_WT = atoi(query.getStringField("TR_WT"));


        //conta
        gatherInfo.CONTA.iCONTA_NUM = atoi(query.getStringField("CONTA_NUM"));
        gatherInfo.CONTA.iCONTA_RECO = atoi(query.getStringField("CONTA_RECO"));

        strcpy(gatherInfo.CONTA.szCONTA_ID_F, query.getStringField("CONTA_ID_F"));
        strcpy(gatherInfo.CONTA.szCONTA_ID_B, query.getStringField("CONTA_ID_B"));

        strcpy(gatherInfo.CONTA.szCONTA_MODEL_F,query.getStringField("CONTA_MODEL_F"));
        strcpy(gatherInfo.CONTA.szCONTA_MODEL_B,query.getStringField("CONTA_MODEL_B"));
        
        
        //seal
        strcpy(gatherInfo.Eseal.szESEAL_ID ,query.getStringField("ESEAL_ID"));
        strcpy(gatherInfo.Eseal.szESEAL_KEY ,query.getStringField("ESEAL_KEY"));
        
        //weight
        gatherInfo.Gross_WT = atof(query.getStringField("GROSS_WT"));
        
        //gps
        
        strcpy(gatherInfo.GPS.szVE_NAME,query.getStringField("VE_NAME"));
        strcpy(gatherInfo.GPS.szGPS_ID,query.getStringField("GPS_ID"));
        strcpy(gatherInfo.GPS.szORIGIN_CUSTOMS ,query.getStringField("ORIGIN_CUSTOMS"));
        strcpy(gatherInfo.GPS.szDEST_CUSTOMS ,query.getStringField("DEST_CUSTOMS"));
        
        
        
        
        

        gatherVec.push_back(gatherInfo);

        query.nextRow();
    }

    return gatherVec.size();
}

int CSQLDataAccess::queryAllGatherInfo(char* channel_id, char* begin_time, char* stop_time, GATHERINFOVec& gatherVec)
{

    query_gather_info(channel_id, begin_time, stop_time, gatherVec);
    if (gatherVec.size() > 0)
    {
 //       std::vector<QUERY_GATHER_INFO>::iterator iter;
 //       for (iter = gatherVec.begin(); iter != gatherVec.end(); iter++)
        {
 //           QUERY_GATHER_INFO& gather = *iter;

  //          query_ic_info(gather.szSeq_No, gather);
 //           query_car_info(gather.szSeq_No, gather);

 //           query_trailer_info(gather.szSeq_No, gather);
  //          query_conta_info(gather.szSeq_No, gather);
  //          query_eseal_info(gather.szSeq_No, gather);
  //          query_weight_info(gather.szSeq_No, gather);
  //          query_gps_info(gather.szSeq_No, gather);

        }
    }

    return gatherVec.size();
}

int CSQLDataAccess::query_ic_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_IC ic,T_GATHERINFO_CAR car,T_GATHERINFO_TRAILER trailer,T_GATHERINFO_CONTA conta,T_GATHERINFO_SEAL seal,\
                         T_GATHERINFO_WEIGHT weight,T_GATHERINFO_GPS gps WHERE ic.SEQUENCE_NO=car.SEQUENCE_NO AND car.SEQUENCE_NO=trailer.SEQUENCE_NO \
                         AND conta.SEQUENCE_NO=trailer.SEQUENCE_NO  AND conta.SEQUENCE_NO=seal.SEQUENCE_NO  AND conta.SEQUENCE_NO=gps.SEQUENCE_NO \
                         AND conta.SEQUENCE_NO=weight.SEQUENCE_NO AND  ic.SEQUENCE_NO='%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {
        //ic
        strcpy(gatherInfo.IC.szDR_IC_NO, query.getStringField("DR_IC_NO"));
        strcpy(gatherInfo.IC.szIC_DR_CUSTOMS_NO, query.getStringField("IC_DR_CUSTOMS_NO"));
        strcpy(gatherInfo.IC.szIC_CO_CUSTOMS_NO, query.getStringField("IC_CO_CUSTOMS_NO"));
        strcpy(gatherInfo.IC.szIC_BILL_NO, query.getStringField("IC_BILL_NO"));
        gatherInfo.IC.lIC_GROSS_WT = atoi(query.getStringField("IC_GROSS_WT"));
        strcpy(gatherInfo.IC.szIC_VE_CUSTOMS_NO, query.getStringField("IC_VE_CUSTOMS_NO"));

        strcpy(gatherInfo.IC.szIC_VE_NAME, query.getStringField("IC_VE_NAME"));

        strcpy(gatherInfo.IC.szIC_CONTA_ID, query.getStringField("IC_CONTA_ID"));
        strcpy(gatherInfo.IC.szIC_ESEAL_ID, query.getStringField("IC_ESEAL_ID"));

        strcpy(gatherInfo.IC.szIC_EX_DATA, query.getStringField("IC_EX_DATA"));
        strcpy(gatherInfo.IC.szIC_BUSS_TYPE, query.getStringField("IC_BUSS_TYPE"));
        
        
        //car
        strcpy(gatherInfo.CAR.szVE_NAME, query.getStringField("VE_NAME"));
        strcpy(gatherInfo.CAR.szCAR_EC_NO, query.getStringField("CAR_EC_NO"));
        gatherInfo.CAR.lVE_WT = atoi(query.getStringField("VE_WEIGHT"));

        strcpy(gatherInfo.CAR.szVE_CUSTOMS_NO, query.getStringField("VE_CUSTOMS_NO"));
        
        //trailer
        strcpy(gatherInfo.TRAILER.szTR_NAME, query.getStringField("TR_NAME"));
        strcpy(gatherInfo.TRAILER.szTR_EC_NO, query.getStringField("TR_EC_NO"));
        gatherInfo.TRAILER.lTR_WT = atoi(query.getStringField("TR_WT"));


        //conta
        gatherInfo.CONTA.iCONTA_NUM = atoi(query.getStringField("CONTA_NUM"));
        gatherInfo.CONTA.iCONTA_RECO = atoi(query.getStringField("CONTA_RECO"));

        strcpy(gatherInfo.CONTA.szCONTA_ID_F, query.getStringField("CONTA_ID_F"));
        strcpy(gatherInfo.CONTA.szCONTA_ID_B, query.getStringField("CONTA_ID_B"));

        strcpy(gatherInfo.CONTA.szCONTA_MODEL_F,query.getStringField("CONTA_MODEL_F"));
        strcpy(gatherInfo.CONTA.szCONTA_MODEL_B,query.getStringField("CONTA_MODEL_B"));
        
        
        //seal
        strcpy(gatherInfo.Eseal.szESEAL_ID ,query.getStringField("ESEAL_ID"));
        strcpy(gatherInfo.Eseal.szESEAL_KEY ,query.getStringField("ESEAL_KEY"));
        
        //weight
        gatherInfo.Gross_WT = atof(query.getStringField("GROSS_WT"));
        
        //gps
        
        strcpy(gatherInfo.GPS.szVE_NAME,query.getStringField("VE_NAME"));
        strcpy(gatherInfo.GPS.szGPS_ID,query.getStringField("GPS_ID"));
        strcpy(gatherInfo.GPS.szORIGIN_CUSTOMS ,query.getStringField("ORIGIN_CUSTOMS"));
        strcpy(gatherInfo.GPS.szDEST_CUSTOMS ,query.getStringField("DEST_CUSTOMS"));
        
        
        
        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_car_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_CAR WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {

        strcpy(gatherInfo.CAR.szVE_NAME, query.getStringField("VE_NAME"));
        strcpy(gatherInfo.CAR.szCAR_EC_NO, query.getStringField("CAR_EC_NO"));
        gatherInfo.CAR.lVE_WT = atoi(query.getStringField("VE_WEIGHT"));

        strcpy(gatherInfo.CAR.szVE_CUSTOMS_NO, query.getStringField("VE_CUSTOMS_NO"));


        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_trailer_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_TRAILER WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {

        strcpy(gatherInfo.TRAILER.szTR_NAME, query.getStringField("TR_NAME"));
        strcpy(gatherInfo.TRAILER.szTR_EC_NO, query.getStringField("TR_EC_NO"));
        gatherInfo.TRAILER.lTR_WT = atoi(query.getStringField("TR_WT"));


        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_conta_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_CONTA WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {
        gatherInfo.CONTA.iCONTA_NUM = atoi(query.getStringField("CONTA_NUM"));
        gatherInfo.CONTA.iCONTA_RECO = atoi(query.getStringField("CONTA_RECO"));

        strcpy(gatherInfo.CONTA.szCONTA_ID_F, query.getStringField("CONTA_ID_F"));
        strcpy(gatherInfo.CONTA.szCONTA_ID_B, query.getStringField("CONTA_ID_B"));

        strcpy(gatherInfo.CONTA.szCONTA_MODEL_F,query.getStringField("CONTA_MODEL_F"));
        strcpy(gatherInfo.CONTA.szCONTA_MODEL_B,query.getStringField("CONTA_MODEL_B"));


        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_eseal_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_SEAL WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {
        strcpy(gatherInfo.Eseal.szESEAL_ID ,query.getStringField("ESEAL_ID"));
        strcpy(gatherInfo.Eseal.szESEAL_KEY ,query.getStringField("ESEAL_KEY"));


        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_weight_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_WEIGHT WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {
        gatherInfo.Gross_WT = atof(query.getStringField("GROSS_WT"));



        query.nextRow();
    }

    return 0;
}

int CSQLDataAccess::query_gps_info(char* sequence_no, QUERY_GATHER_INFO& gatherInfo)
{

    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_GPS WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    if (!query.eof())
    {
        strcpy(gatherInfo.GPS.szVE_NAME,query.getStringField("VE_NAME"));
        strcpy(gatherInfo.GPS.szGPS_ID,query.getStringField("GPS_ID"));
        strcpy(gatherInfo.GPS.szORIGIN_CUSTOMS ,query.getStringField("ORIGIN_CUSTOMS"));
        strcpy(gatherInfo.GPS.szDEST_CUSTOMS ,query.getStringField("DEST_CUSTOMS"));


        query.nextRow();
    }

    return 0;
}


int CSQLDataAccess::queryContaPicInfo(char* sequence_no, GONTAPICVec& picVec)
{
    CppMySQL3DB* m_pDatabase = NULL;
    CMySQLConnectionPool::GetDataAccess(m_pDatabase);
    if (!m_pDatabase)
    {
        return -1;
    }

    CSqlAutoLock autolock(m_pDatabase);



    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "SELECT * FROM T_GATHERINFO_CONTAPIC WHERE SEQUENCE_NO = '%s' ", sequence_no);


    m_pDatabase->SetCharsetName("GB2312");

    CppMySQLQuery query = m_pDatabase->querySQL(chQuerySQL);


    while (!query.eof())
    {
        CONTA_PIC_INFO pic_info;
        pic_info.pic_pos=query.getIntField("PIC_TYPE");
        strcpy(pic_info.file_path,query.getStringField("PIC_PATH"));
       
        picVec.push_back(pic_info);
        query.nextRow();
    }

    return picVec.size();
}