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

// 2015-5-19
struct struDeviceAndServiceStatus
{
	std::string szLocalIP;				// 服务IP地址
	int  nLocalPort;					// 服务端口

	std::string szUpstreamIP;			// 上位服务IP
	int  nUpstreamPort;					// 上位服务端口号
	std::string szUpstreamObjectCode;	// 上位服务代号
	int  nServiceType;					// 服务类别代码

	std::string szObjectCode;			// 对象代号
	std::string szObjectStatus;			// 状态值
	std::string szConnectedObjectCode;	// 对接对象代号
	std::string szReportTime;			// 时间戳
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
	static char DI_BIT[9];
	static char DI_ENABLE[9];

	static char SnapshotServerIP[32];
	static int  SnapshotServerPort;

	static char RecogServerIP[32];
	static int  RecogServerPort;

	static std::vector<JZ_PUBLISH_EVENT_STRUCT>	m_eventVect;

	static std::string	m_strGatherControlIP;
	static int			m_nGatherControlPort;
	static std::string	m_strChannelControlIP;
	static std::string  m_publisher_server_ip;
	static int			m_publisher_server_port;
};

#endif
