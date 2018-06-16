/* 
 * File:   CJZPlatformProxy.cpp
 * Author: root
 * 
 * Created on 2015年1月15日, 下午11:03
 */

#include "CJZPlatformProxy.h"
#include <string>

using std::string;

extern "C" CPlatformProxy* create() {
    return new CJZPlatformProxy;
}

extern "C" void destroy(CPlatformProxy* p) {
    delete p;
}

CJZPlatformProxy::CJZPlatformProxy() {
    m_pCrtlCmdCallback = NULL;
    m_pBasicDataAccess = new CBasicDataAccess;
}

CJZPlatformProxy::~CJZPlatformProxy() {
}

int CJZPlatformProxy::PackUploadData(CppMySQL3DB* pDatabase, char* szPlatName, char* pData, int nLen, char* szPackedData, int& nPacketDataLen,int nIsRegather) {
    if (strcmp(szPlatName, "CUSTOMS") == 0) {
        m_pBasicDataAccess->RecordPassVehicleInfo(pDatabase, pData,nIsRegather);
        memcpy(szPackedData, pData, nLen);
        nPacketDataLen = nLen;
    }

    return 0;
}

int CJZPlatformProxy::UnpackCtrlData(CppMySQL3DB* pDatabase, char* pData, int nLen, char *pAreaNo, char *pChannelNo, char *pIE, char *pSeqNo) 
{
	unsigned char chType = ' ';
	if (GetPacketType(pData, nLen, chType) == 0)
	{
		;
	}

	switch (chType)
	{
	case 0x22:
		ProcessPacket0X22(pDatabase, pData, nLen);
		break;

	case 0x31:	// 卡口后台系统向卡口前端集成系统发出的设备操作控制指令，流程上对应0x32
		ProcessPacket0X31(pDatabase, pData, nLen, pAreaNo, pChannelNo, pIE, pSeqNo);
		break;

	case 0x39:	// 人工放行指令，直接由平台发送到卡口抬杆
		ProcessPacket0X39(pDatabase, pData, nLen);
		break;

	default:
		break;
	}

    return 0;
}

int CJZPlatformProxy::BuildCtrlData(CppMySQL3DB* pDatabase, char* szChannelNo, char* szPassSequence, char* pData, char* szMemo) {
    char m_szResultCtrlXML[4096] = {0};

    string strFullChannelID(szChannelNo);

    string strAreaID = strFullChannelID.substr(0, 10);
    string strChannelNo = strFullChannelID.substr(10, 10);
    string strIEType = strFullChannelID.substr(20, 1);



    if (strcmp(pData, "00") == 0) {
        m_pBasicDataAccess->RecordManPass(pDatabase, szPassSequence, szMemo);

        sprintf(m_szResultCtrlXML, "%s", "<CONTROL_INFO>");
        sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"></EVENT>", m_szResultCtrlXML, "M000000001");
        sprintf(m_szResultCtrlXML, "%s </CONTROL_INFO>", m_szResultCtrlXML);
    }

    if (m_pCrtlCmdCallback) {
        m_pCrtlCmdCallback((char*) strAreaID.c_str(), (char*) strChannelNo.c_str(), (char*) strIEType.c_str(), szPassSequence, m_szResultCtrlXML, strlen(m_szResultCtrlXML) + 1);
    }

}

int CJZPlatformProxy::BuildExceptionFreeData(CppMySQL3DB* pDatabase, char* szChannelNo, char* szPassSequence, char* pData, int nDataLen, char* szMemo) 
{
    if (pData == NULL || nDataLen <= 0)
        return 0;
    
	char m_szResultCtrlXML[4096] = {0};

	string strFullChannelID(szChannelNo);

	string strAreaID = strFullChannelID.substr(0, 10);
	string strChannelNo = strFullChannelID.substr(10, 10);
	string strIEType = strFullChannelID.substr(20, 1);



	// if (strcmp(pData, "00") == 0) 
	{
		//m_pBasicDataAccess->RecordManPass(pDatabase, szPassSequence, szMemo);

strncpy(m_szResultCtrlXML, pData, nDataLen);
//		std::string strControlData = "";
//		strControlData = \
//			"<LED>\r\n"
//			"<LED_CODE>00</LED_CODE>\r\n"
//			"<LED_MSG>直接放行</LED_MSG>\r\n"
//			"</LED>\r\n"
//			"<BROADCAST>\r\n"
//			"<BROADCAST_CODE>00</BROADCAST_CODE>\r\n"
//			"<BROADCAST_MSG>直接放行</BROADCAST_MSG>\r\n"
//			"</BROADCAST>\r\n"
//			"<PRINT>\r\n"
//			"<VENAME>2</VENAME>\r\n"
//			"<CONTA_ID_F>HLXU4074106</CONTA_ID_F>\r\n"
//			"<CONTA_ID_B>FCIU9176921</CONTA_ID_B>\r\n"
//			"<CONTA_MODEL_F>20</CONTA_MODEL_F>\r\n"
//			"<CONTA_MODEL_B>20</CONTA_MODEL_B>\r\n"
//			"<Oper_Name></Oper_Name>\r\n"
//			"</PRINT>\r\n";
//
//		sprintf(m_szResultCtrlXML, "%s", "<CONTROL_INFO>");
//		//sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"></EVENT>", m_szResultCtrlXML, "M000000001");
//		sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"><![CDATA[%s]]></EVENT>", m_szResultCtrlXML, "M000000001", strControlData.c_str());
//		sprintf(m_szResultCtrlXML, "%s </CONTROL_INFO>", m_szResultCtrlXML);
		m_pBasicDataAccess->RecordPassResult(pDatabase, szPassSequence, 99, "货物放行", "EC_FREE_MAN",m_szResultCtrlXML);
	}

	if (m_pCrtlCmdCallback) {
		m_pCrtlCmdCallback((char*) strAreaID.c_str(), (char*) strChannelNo.c_str(), (char*) strIEType.c_str(), szPassSequence, m_szResultCtrlXML, strlen(m_szResultCtrlXML) + 1);
	}

}


int CJZPlatformProxy::VerifyBasicPlatformPacket(char *chRecvBuffer, int nRecLen) {
    /*缓冲区长度小于最小帧长度*/
    if (nRecLen < 40) {
        return -1;
    }

    unsigned char* pRecvBuffer = (unsigned char*) chRecvBuffer;

    if (pRecvBuffer[0] == 0XE2 && pRecvBuffer[1] == 0X5C && pRecvBuffer[2] == 0X4B && pRecvBuffer[3] == 0X89) {
        if (pRecvBuffer[nRecLen - 2] == 0XFF && pRecvBuffer[nRecLen - 1] == 0XFF) {
            return 0;
        }
    }

    return -1;
}

int CJZPlatformProxy::GetPacketType(char *chRecvBuffer, int nRecLen, unsigned char &chType) 
{
	/*缓冲区长度小于最小帧长度*/
	if (nRecLen < 40) {
		return -1;
	}

	chType = (unsigned char) chRecvBuffer[8];

	return 0;
}

int CJZPlatformProxy::ProcessPacket0X22(CppMySQL3DB* pDatabase, char* pData, int nLen)
{
	char szPlatName[32] = {0};

	if (VerifyBasicPlatformPacket(pData, nLen) == 0) 
	{
		strcpy(szPlatName, "CUSTOMS");
	}

	T_PassResultInfo passResult;
	memset(&passResult, 0, sizeof (T_PassResultInfo));

	char m_szResultCtrlXML[4096] = {0};

	if (strcmp(szPlatName, "CUSTOMS") == 0) {
		m_pBasicDataAccess->RecordPassResult(pDatabase, pData, &passResult);


		std::string strControlData = "";
		strControlData = \
			"<LED>\r\n"
			"<LED_CODE>00</LED_CODE>\r\n"
			"<LED_MSG>直接放行</LED_MSG>\r\n"
			"</LED>\r\n"
			"<BROADCAST>\r\n"
			"<BROADCAST_CODE>00</BROADCAST_CODE>\r\n"
			"<BROADCAST_MSG>直接放行</BROADCAST_MSG>\r\n"
			"</BROADCAST>\r\n"
			"<PRINT>\r\n"
			"<VENAME>2</VENAME>\r\n"
			"<CONTA_ID_F>HLXU4074106</CONTA_ID_F>\r\n"
			"<CONTA_ID_B>FCIU9176921</CONTA_ID_B>\r\n"
			"<CONTA_MODEL_F>20</CONTA_MODEL_F>\r\n"
			"<CONTA_MODEL_B>20</CONTA_MODEL_B>\r\n"
			"<Oper_Name></Oper_Name>\r\n"
			"</PRINT>\r\n";


		sprintf(m_szResultCtrlXML, "%s", "<CONTROL_INFO>");
		sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"><![CDATA[%s]]></EVENT>", m_szResultCtrlXML, "EC_FREE_PLATFORM", strControlData.c_str());
		sprintf(m_szResultCtrlXML, "%s </CONTROL_INFO>", m_szResultCtrlXML);


		m_pBasicDataAccess->RecordPassResult(pDatabase,passResult.szSeqNo,0,"临时车辆放行","EC_FREE_PLATFORM",m_szResultCtrlXML);
	}

	if (m_pCrtlCmdCallback) 
	{

		m_pCrtlCmdCallback(passResult.szAeraID, passResult.szChlNo, passResult.szIEType, passResult.szSeqNo, m_szResultCtrlXML, strlen(m_szResultCtrlXML) + 1);
	}

	return 0;
}

int CJZPlatformProxy::ProcessPacket0X31(CppMySQL3DB* pDatabase, char* pData, int nLen, char *pAreaNo, char *pChannelNo, char *pIE, char *pSeqNo)
{
	char szPlatName[32] = {0};

	if (VerifyBasicPlatformPacket(pData, nLen) == 0) 
	{
		strcpy(szPlatName, "CUSTOMS");
	}

	T_PassResultInfo0X31 passResult;
	memset(&passResult, 0, sizeof (T_PassResultInfo0X31));

	char m_szResultCtrlXML[4096] = {0};

	if (strcmp(szPlatName, "CUSTOMS") == 0) {
		m_pBasicDataAccess->RecordPassResult0X31(pDatabase, pData, &passResult);
		memcpy(pAreaNo,		passResult.szAeraID,	strlen(passResult.szAeraID));
		memcpy(pChannelNo,	passResult.szChlNo,		strlen(passResult.szChlNo));
		memcpy(pIE,			passResult.szIEType,	strlen(passResult.szIEType));
		memcpy(pSeqNo,		passResult.szSeqNo,		strlen(passResult.szSeqNo));


		std::string strControlData = "";
		if (passResult.EXCUTE_COMMAND[1] == '0')
		{
			strControlData = \
				"<LED>\r\n"
				"<LED_CODE>00</LED_CODE>\r\n"
				"<LED_MSG>直接放行</LED_MSG>\r\n"
				"</LED>\r\n"
				"<BROADCAST>\r\n"
				"<BROADCAST_CODE>00</BROADCAST_CODE>\r\n"
				"<BROADCAST_MSG>直接放行</BROADCAST_MSG>\r\n"
				"</BROADCAST>\r\n"
				"<PRINT>\r\n"
				"<VENAME>2</VENAME>\r\n"
				"<CONTA_ID_F>HLXU4074106</CONTA_ID_F>\r\n"
				"<CONTA_ID_B>FCIU9176921</CONTA_ID_B>\r\n"
				"<CONTA_MODEL_F>20</CONTA_MODEL_F>\r\n"
				"<CONTA_MODEL_B>20</CONTA_MODEL_B>\r\n"
				"<Oper_Name></Oper_Name>\r\n"
				"</PRINT>\r\n";
		}
		else
		{
			strControlData = \
				"<LED>\r\n"
				"<LED_CODE>00</LED_CODE>\r\n"
				"<LED_MSG>禁止放行</LED_MSG>\r\n"
				"</LED>\r\n"
				"<BROADCAST>\r\n"
				"<BROADCAST_CODE>00</BROADCAST_CODE>\r\n"
				"<BROADCAST_MSG>禁止放行</BROADCAST_MSG>\r\n"
				"</BROADCAST>\r\n"
				"<PRINT>\r\n"
				"<VENAME>2</VENAME>\r\n"
				"<CONTA_ID_F>HLXU4074106</CONTA_ID_F>\r\n"
				"<CONTA_ID_B>FCIU9176921</CONTA_ID_B>\r\n"
				"<CONTA_MODEL_F>20</CONTA_MODEL_F>\r\n"
				"<CONTA_MODEL_B>20</CONTA_MODEL_B>\r\n"
				"<Oper_Name></Oper_Name>\r\n"
				"</PRINT>\r\n";
		}

		sprintf(m_szResultCtrlXML, "%s", "<CONTROL_INFO>");
		sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"><![CDATA[%s]]></EVENT>", m_szResultCtrlXML, "EC_FREE_PLATFORM", strControlData.c_str());
		sprintf(m_szResultCtrlXML, "%s </CONTROL_INFO>", m_szResultCtrlXML);


		m_pBasicDataAccess->RecordPassResult(pDatabase,passResult.szSeqNo,0,"临时车辆放行","EC_FREE_PLATFORM",m_szResultCtrlXML);
	}

	if (m_pCrtlCmdCallback) 
	{

		m_pCrtlCmdCallback(passResult.szAeraID, passResult.szChlNo, passResult.szIEType, passResult.szSeqNo, m_szResultCtrlXML, strlen(m_szResultCtrlXML) + 1);
	}

	return 0;
}

int CJZPlatformProxy::ProcessPacket0X39(CppMySQL3DB* pDatabase, char* pData, int nLen)
{
	char szPlatName[32] = {0};

	if (VerifyBasicPlatformPacket(pData, nLen) == 0) 
	{
		strcpy(szPlatName, "CUSTOMS");
	}

	T_PassResultInfo0X39 passResult;
	memset(&passResult, 0, sizeof (T_PassResultInfo0X39));

	char m_szResultCtrlXML[4096] = {0};

	if (strcmp(szPlatName, "CUSTOMS") == 0) {
		m_pBasicDataAccess->RecordPassResult0X39(pDatabase, pData, &passResult);

		std::string strControlData = "";
		strControlData = \
			"<LED>\r\n"
			"<LED_CODE>00</LED_CODE>\r\n"
			"<LED_MSG>人工放行</LED_MSG>\r\n"
			"</LED>\r\n"
			"<BROADCAST>\r\n"
			"<BROADCAST_CODE>00</BROADCAST_CODE>\r\n"
			"<BROADCAST_MSG>人工放行</BROADCAST_MSG>\r\n"
			"</BROADCAST>\r\n"
			"<PRINT>\r\n"
			"<VENAME>2</VENAME>\r\n"
			"<CONTA_ID_F>HLXU4074106</CONTA_ID_F>\r\n"
			"<CONTA_ID_B>FCIU9176921</CONTA_ID_B>\r\n"
			"<CONTA_MODEL_F>20</CONTA_MODEL_F>\r\n"
			"<CONTA_MODEL_B>20</CONTA_MODEL_B>\r\n"
			"<Oper_Name></Oper_Name>\r\n"
			"</PRINT>\r\n";


		sprintf(m_szResultCtrlXML, "%s", "<CONTROL_INFO>");
		sprintf(m_szResultCtrlXML, "%s <EVENT type=\"CAR\" hint=\"临时车辆放行\" eventid=\"%s\"><![CDATA[%s]]></EVENT>", m_szResultCtrlXML, "EC_FREE_PLATFORM", strControlData.c_str());
		sprintf(m_szResultCtrlXML, "%s </CONTROL_INFO>", m_szResultCtrlXML);


		m_pBasicDataAccess->RecordPassResult(pDatabase,passResult.szSeqNo,0,"临时车辆放行","EC_FREE_PLATFORM",m_szResultCtrlXML);
	}

	if (m_pCrtlCmdCallback) 
	{

		m_pCrtlCmdCallback(passResult.szAeraID, passResult.szChlNo, passResult.szIEType, passResult.szSeqNo, m_szResultCtrlXML, strlen(m_szResultCtrlXML) + 1);
	}

	return 0;
}
