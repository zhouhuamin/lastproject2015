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

struct T_PassResultInfo0X31
{
	char szAeraID[32];
	char szChlNo[32];
	char szIEType[4];
	char szSeqNo[24];
	char EXCUTE_COMMAND[24];
	char szSealID[128];
	char szSealKey[128];
	//char szOpenTimes[32];
	char OP_TYPE[10];
	char OP_REASON[64];
	char OP_ID[64];
	char szVEName[64];
	char szGPSID[64];
	char szOriginCustoms[128];
	char szDestCustoms[128];
};

struct T_PassResultInfo0X39
{
	char szAeraID[32];
	char szChlNo[32];
	char szIEType[4];
	char szSeqNo[24];
	char EXCUTE_COMMAND[24];
	char EXC_TIME[64];
};

class CBasicDataAccess{
public:
    CBasicDataAccess();
    virtual ~CBasicDataAccess();

    int RecordPassVehicleInfo(CppMySQL3DB* pDataabse, char* szXML,int nIsRegather=0);
	int RecordMsg0X32(CppMySQL3DB* pDataabse, char* szXML,int nIsRegather=0);
    int RecordPassResult(CppMySQL3DB* pDataabse, char* szXML,T_PassResultInfo* pPassResult);
    int RecordManPass(CppMySQL3DB* pDatabase,char* szPassSequence,char* szMemo );
    int RecordPassResult(CppMySQL3DB* pDataabse,char* szSequenceNo,int nResult,char* szHint, char *szEventID, char *szXmlInfo);
    
private:
    int handleICTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleWeightTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleCarTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleTrailerTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleContaTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleSealTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML,int nIsRegather=0);
	int handleMsg0X32(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML,int nIsRegather=0);

    int handleResultAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);
    int handlePassResult(T_PassResultInfo& resultInfo, CppMySQL3DB* pDatabase);
    
     
public:
	int RecordPassResult0X31(CppMySQL3DB* pDataabse, char* szXML,T_PassResultInfo0X31* pPassResult);
	int handleResultAll0X31(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);
	int handlePassResult0X31(T_PassResultInfo0X31& resultInfo, CppMySQL3DB* pDatabase);

	int RecordPassResult0X39(CppMySQL3DB* pDataabse, char* szXML, T_PassResultInfo0X39* pPassResult);
	int handleResultAll0X39(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);
	int handlePassResult0X39(T_PassResultInfo0X39& resultInfo, CppMySQL3DB* pDatabase);
};

#endif	/* _CBASICDATAACCESS_H */

