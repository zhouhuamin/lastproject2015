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

    int PackUploadData(CppMySQL3DB* pDatabase, char* szPlatName, char* pData, int nLen, char* szPackedData, int& nPacketDataLen,int nIsRegather=0);
    int UnpackCtrlData(CppMySQL3DB* pDatabase, char* pData, int nLen, char *pAreaNo, char *pChannelNo, char *pIE, char *pSeqNo);
    int BuildCtrlData(CppMySQL3DB* pDatabase,char* szChannelNo,char* szPassSequence,char* pData,char* szMemo);
    int BuildExceptionFreeData(CppMySQL3DB* pDatabase,char* szChannelNo,char* szPassSequence,char* pData, int nDataLen, char* szMemo);

private:
    int VerifyBasicPlatformPacket(char *chRecvBuffer, int nRecLen);
	int GetPacketType(char *chRecvBuffer, int nRecLen, unsigned char &chType);

	int ProcessPacket0X22(CppMySQL3DB* pDatabase, char* pData, int nLen);
	int ProcessPacket0X31(CppMySQL3DB* pDatabase, char* pData, int nLen, char *pAreaNo, char *pChannelNo, char *pIE, char *pSeqNo);
	int ProcessPacket0X39(CppMySQL3DB* pDatabase, char* pData, int nLen);

private:
    CBasicDataAccess* m_pBasicDataAccess;
    _CONTROL_CMD_CALLBACK m_pCrtlCmdCallback;

};

#endif	/* CJZPLATFORMPROXY_H */

