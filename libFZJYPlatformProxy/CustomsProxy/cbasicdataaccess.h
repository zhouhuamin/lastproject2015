/* 
 * File:   cbasicdataaccess.h
 * Author: root
 *
 * Created on 2014年11月22日, 下午9:59
 */

#ifndef _CBASICDATAACCESS_H
#define	_CBASICDATAACCESS_H

#include "tinyXML/tinyxml.h"
#include "../include/cppmysql.h"

#include <string>
using std::string;


struct T_PassResultInfo {
    char szAeraID[32];
    char szChlNo[32];
    char szIEType[4];
    char szSeqNo[24];
    char szCheckResult[24];
    char szSealID[128];
    char szSealKey[128];
    char szOpenTimes[32];
    char szFormID[32];
    char szOPHint[64];
    char szVEName[64];
    char szGPSID[64];
    char szOriginCustoms[128];
    char szDestCustoms[128];
};

struct structGatherInfo
{
	std::string AREA_ID;
	std::string CHNL_NO;
	std::string SEQ_NO;
	std::string IE_FLAG;
	std::string GATHER_TIME;
	std::string ECAR_NO;
	std::string ETAG_ID;
	std::string PCAR_NO;
	std::string PCAR_NO_PICNAME;
	std::string PCAR_PICNAME;
};

//struct structModifyInfo
//{
//	std::string AREA_ID;
//	std::string CHNL_NO;
//	std::string SEQ_NO;
//	std::string IE_FLAG;
//	std::string USER_NAME;
//	std::string USER_ID;
//	std::string CLIENT_FINISH_FLAG;
//	std::string CLIENT_TIME;
//	std::string MODIFY_CAR_NO;
//	std::string MODIFY_CONTA_NO_F;
//	std::string MODIFY_CONTA_TYPE_F;
//	std::string MODIFY_CONTA_NO_B;
//	std::string MODIFY_CONTA_TYPE_B;
//	std::string MODIFY_CAR_TAIL;
//	std::string MODIFY_WEIGHT;
//	std::string MODIFY_FREE_FLAG;
//	std::string MODIFY_FREE_RESON;
//};


class CBasicDataAccess{
public:
    CBasicDataAccess();
    virtual ~CBasicDataAccess();

    int RecordPassVehicleInfo(CppMySQL3DB* pDataabse, char* szXML);
    int RecordPassResult(CppMySQL3DB* pDataabse, char* szXML,T_PassResultInfo* pPassResult);
    int RecordManPass(CppMySQL3DB* pDatabase,char* szPassSequence,char* szMemo );
private:
    int handleICTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleWeightTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleCarTag(structGatherInfo & gatherInfo, char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleTrailerTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleContaTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleSealTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);

    int handleResultAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);
    int handlePassResult(T_PassResultInfo& resultInfo, CppMySQL3DB* pDatabase);

	int handlePCarTag(structGatherInfo & gatherInfo, CppMySQL3DB* pDatabase, TiXmlElement *item);

	int queryCarTag(structGatherInfo & gatherInfo, CppMySQL3DB* pDatabase);
	//int queryModifyRecord(structModifyInfo & modifyInfo, CppMySQL3DB* pDatabase);
	//int UpdateModifyRecord(structModifyInfo & modifyInfo, CppMySQL3DB* pDatabase);
	//int UpdateSendFlag(structModifyInfo & modifyInfo, CppMySQL3DB* pDatabase);
};

#endif	/* _CBASICDATAACCESS_H */

