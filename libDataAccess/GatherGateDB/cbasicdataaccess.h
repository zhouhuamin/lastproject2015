/* 
 * File:   cbasicdataaccess.h
 * Author: root
 *
 * Created on 2014年11月22日, 下午9:59
 */

#ifndef _CBASICDATAACCESS_H
#define	_CBASICDATAACCESS_H
#include "../include/DataAccess.h"
#include "tinyXML/tinyxml.h"

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
};

class CBasicDataAccess : public CDataAccess {
public:
    CBasicDataAccess();
    virtual ~CBasicDataAccess();

    int RecordPassVehicleInfo(CppMySQL3DB* pDataabse, char* szXML);
    int RecordPassResult(CppMySQL3DB* pDataabse, char* szXML);
private:
    int handleICTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleWeightTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleCarTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleTrailerTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleContaTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleSealTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item);
    int handleAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);

    int handleResultAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML);
    int handlePassResult(T_PassResultInfo& resultInfo, CppMySQL3DB* pDatabase);
};

#endif	/* _CBASICDATAACCESS_H */

