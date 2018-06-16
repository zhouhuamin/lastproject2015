/* 
 * File:   CJZPlatformProxy.h
 * Author: root
 *
 * Created on 2015年1月15日, 下午11:03
 */

#ifndef CJZPLATFORMPROXY_H
#define	CJZPLATFORMPROXY_H

#include "../include/PlatProxy.h"
#include "../include/cppmysql.h"
#include "tinyXML/tinyxml.h"
#include "cbasicdataaccess.h"

class CJZPlatformProxy : public CPlatformProxy
{
public:
    CJZPlatformProxy();
    virtual ~CJZPlatformProxy();

    int SetCtrlCmdCallback(_CONTROL_CMD_CALLBACK pCtrlCmdcallback) {
        m_pCrtlCmdCallback = pCtrlCmdcallback;
    }

    int PackUploadData(CppMySQL3DB* pDatabase, char* szPlatName, char* pData, int nLen, char* szPackedData, int& nPacketDataLen);
    int UnpackCtrlData(CppMySQL3DB* pDatabase, char* pData, int nLen);
    int BuildCtrlData(CppMySQL3DB* pDatabase,char* szChannelNo,char* szPassSequence,char* pData,char* szMemo);

private:
    int VerifyBasicPlatformPacket(char *chRecvBuffer, int nRecLen);

private:
    CBasicDataAccess* m_pBasicDataAccess;
    _CONTROL_CMD_CALLBACK m_pCrtlCmdCallback;

};

#endif	/* CJZPLATFORMPROXY_H */

