#ifndef _SIMPLE_CONFIG_H_

#define	_SIMPLE_CONFIG_H_

#pragma warning(disable : 4786)	

#include <string>
#include <map>
#include <vector>

using namespace std;

#include "tinyXML/xml_config.h"

struct JZ_PUBLISH_EVENT_STRUCT
{
	char  event_id[32];
	char  device_tag[32];
	char  sequence_no[32];
};

class CSimpleConfig
{
public:
    CSimpleConfig();
    virtual ~CSimpleConfig();
    void get_config();
	void ReadConfig(std::string strFileName);
    static int GetProcessPath(char* cpPath);
    static int CHANNEL_CONTROLER_PORT;
    static char DEVICE_ID[32];
    //static int COM_PORT;
    //static int BAUD_RATE;
    //static char DEVICE_FACTORY[256];
    //static char DEVICE_SO[64];
    //static char DEVICE_NAME[128];
    static char DEVICE_TAG[32];
    static char EXE_FULL_PATH[256];
	static char CameraFront[50];
	static char CameraLeft[50];
	static char CameraRight[50];
	static char CameraBack[50];
	static char RELAY_BIT[4 + 1];
	static char RELAY_ENABLE[9];

	static char SnapshotServerIP[32];
	static int  SnapshotServerPort;

	static char RecogServerIP[32];
	static int  RecogServerPort;

	static std::vector<JZ_PUBLISH_EVENT_STRUCT>	m_eventVect;

	static std::string	m_strGatherControlIP;
	static int			m_nGatherControlPort;
	static std::string	m_strChannelControlIP;
	static std::vector<std::string>				m_typeVect;
	static int			m_nDelayTime;
};

#endif
