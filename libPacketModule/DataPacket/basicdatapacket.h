/* 
 * File:   basicdatapacket.h
 * Author: root
 *
 */

#ifndef _BASICDATAPACKET_H
#define	_BASICDATAPACKET_H

#include "../include/DataPacket.h"

struct XML_Tag_Data {
    char szTag[64];
    char szTagData[5 * 1024];
};

class BasicDataPacket : public CDataPacket {
public:
    BasicDataPacket();
    virtual ~BasicDataPacket();
public:
    int SetHeadData(char* szXML);
    int SetTagData(char* szTag, char* szXML);
    int PacketData(char* szXML);

private:
    char m_szXMLData[512];
    char m_szICNullData[512];
    char m_szWeightNullData[128];
    char m_szCarNullData[512];
    char m_szTrailerNullData[128];
    char m_szContaNullData[512];
    char m_szSealNullData[128];
    char m_szTailData[64];
	char m_szOptCarNullData[1024];


    std::map<string, XML_Tag_Data> m_TagDataMap;


};

#endif	/* _BASICDATAPACKET_H */

