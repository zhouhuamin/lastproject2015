/* 
 * File:   basicdatapacket.cpp
 * Author: root
 * 
 */

#include "basicdatapacket.h"
#include <string.h>
#include <stdio.h>
BasicDataPacket::BasicDataPacket()
{
    //IC PART
    sprintf(m_szICNullData,
            "<IC>\n"
            "<DR_IC_NO></DR_IC_NO>\n"
            "<IC_DR_CUSTOMS_NO></IC_DR_CUSTOMS_NO>\n"
            "<IC_CO_CUSTOMS_NO></IC_CO_CUSTOMS_NO>\n"
            "<IC_BILL_NO></IC_BILL_NO>\n"
            "<IC_GROSS_WT></IC_GROSS_WT>\n"
            "<IC_VE_CUSTOMS_NO></IC_VE_CUSTOMS_NO>\n"
            "<IC_VE_NAME></IC_VE_NAME>\n"
            "<IC_CONTA_ID></IC_CONTA_ID>\n"
            "<IC_ESEAL_ID></IC_ESEAL_ID>\n"
            "<IC_BUSS_TYPE></IC_BUSS_TYPE>\n"
            "<IC_EX_DATA></IC_EX_DATA>\n"
            "</IC>\n"
            );

    //WEIGHT PART
    sprintf(m_szWeightNullData,
            "<WEIGHT>\n"
            "<GROSS_WT></GROSS_WT>\n"
            "</WEIGHT>\n"
            );


    //CAR PART

    sprintf(m_szCarNullData,
            "<CAR>\n"
            "<VE_NAME></VE_NAME>\n"
            "<CAR_EC_NO></CAR_EC_NO>\n"
            "<CAR_EC_NO2></CAR_EC_NO2>\n"
            "<VE_CUSTOMS_NO></VE_CUSTOMS_NO>\n"
            "<VE_WEIGHT></VE_WEIGHT>\n"
            "</CAR>\n"
            );


    //TRAILER PART
    sprintf(m_szTrailerNullData,
            "<TRAILER>\n"
            "<TR_EC_NO></TR_EC_NO>\n"
            "<TR_NAME></TR_NAME>\n"
            "<TR_WT></TR_WT>\n"
            "</TRAILER>\n"
            );

    //CONTA PART
    sprintf(m_szContaNullData,
            "<CONTA>\n"
            "<CONTA_NUM>0</CONTA_NUM>\n"
            "<CONTA_RECO>0</CONTA_RECO>\n"
            "<CONTA_ID_F></CONTA_ID_F>\n"
            "<CONTA_ID_B></CONTA_ID_B>\n"
            "<CONTA_MODEL_F></CONTA_MODEL_F>\n"
            "<CONTA_MODEL_B></CONTA_MODEL_B>\n"
            "</CONTA>\n"
            );

    //SEAL PART
    sprintf(m_szSealNullData,
            "<SEAL>\n"
            "<ESEAL_ID></ESEAL_ID>\n"
            "<ESEAL_IC_NO></ESEAL_IC_NO>\n"
            "</SEAL>\n"
            );


    //TAIL PART
    sprintf(m_szTailData,
            "</GATHER_INFO>\n"
            );

	//OPTCAR PART
	sprintf(m_szOptCarNullData,
		"<OPTCAR>\n"
		"<PCAR_NO></PCAR_NO>\n"
		"<PCAR_NO_PICNAME></PCAR_NO_PICNAME>\n"
		"<PCAR_PICNAME></PCAR_PICNAME>\n"
		"</OPTCAR>\n"
		);
}

BasicDataPacket::~BasicDataPacket()
{
}

int BasicDataPacket::SetHeadData(char* szXML)
{
    memset(&m_szXMLData, 0, sizeof (512));
    sprintf(m_szXMLData, "%s", szXML);

    return 0;
}

int BasicDataPacket::SetTagData(char* szTag, char* szXML)
{
    if (!szTag || !szXML)
    {
        return -1;
    }

    XML_Tag_Data tag_data;
    memset(&tag_data, 0, sizeof (XML_Tag_Data));

    strcpy(tag_data.szTag, szTag);
    strcpy(tag_data.szTagData, szXML);

    m_TagDataMap[string(szTag)] = tag_data;

    return 0;
}

int BasicDataPacket::PacketData(char* szXML)
{
    //head
    sprintf(szXML, "%s\n", m_szXMLData);


    //IC 
    if(m_TagDataMap.find(string("IC"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("IC")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szICNullData);
    }


    //WEIGHT
    if(m_TagDataMap.find(string("WEIGHT"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("WEIGHT")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szWeightNullData);
    }

    //CAR
    if(m_TagDataMap.find(string("CAR"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("CAR")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szCarNullData);
    }


    //TRAILER
    if(m_TagDataMap.find(string("TRAILER"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("TRAILER")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szTrailerNullData);
    }
    
    //CONTA
    if(m_TagDataMap.find(string("CONTA"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("CONTA")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szContaNullData);
    }

    //SEAL
    if(m_TagDataMap.find(string("SEAL"))!=m_TagDataMap.end())
    {
        sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("SEAL")].szTagData);
    }
    else
    {
        sprintf(szXML, "%s\n%s\n",szXML,m_szSealNullData);
    }

	//PCAR
	if(m_TagDataMap.find(string("OPTCAR"))!=m_TagDataMap.end())
	{
		sprintf(szXML, "%s\n%s\n",szXML, m_TagDataMap[string("OPTCAR")].szTagData);
	}
	else
	{
		sprintf(szXML, "%s\n%s\n",szXML,m_szOptCarNullData);
	}


    sprintf(szXML, "%s\n%s\n",szXML,m_szTailData);



    m_TagDataMap.clear();

    return 0;
}

extern "C" CDataPacket* create()
{
    return new BasicDataPacket;
}

extern "C" void destroy(CDataPacket* p)
{
    delete p;
}


