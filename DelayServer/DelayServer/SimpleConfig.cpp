#include "SimpleConfig.h"

#ifdef WIN32 

//#include   <Windows.h>

#endif





#include "MyLog.h"
#include "tinyXML/tinyxml.h"
#include <syslog.h>
#include <string>

using namespace std;

int CSimpleConfig::CHANNEL_CONTROLER_PORT;

char CSimpleConfig::DEVICE_ID[32];

//int CSimpleConfig::COM_PORT;
//
//int CSimpleConfig::BAUD_RATE;
//
//char CSimpleConfig::DEVICE_FACTORY[256];
//
//char CSimpleConfig::DEVICE_SO[64];
//
//char CSimpleConfig::DEVICE_NAME[128];

char CSimpleConfig::EXE_FULL_PATH[256];

char CSimpleConfig::DEVICE_TAG[32];

char CSimpleConfig::CameraFront[50];
char CSimpleConfig::CameraLeft[50];
char CSimpleConfig::CameraRight[50];
char CSimpleConfig::CameraBack[50];

char CSimpleConfig::RELAY_BIT[5];
char CSimpleConfig::RELAY_ENABLE[9];


char CSimpleConfig::SnapshotServerIP[32];
int  CSimpleConfig::SnapshotServerPort;

char CSimpleConfig::RecogServerIP[32];
int  CSimpleConfig::RecogServerPort;

std::vector<JZ_PUBLISH_EVENT_STRUCT> CSimpleConfig::m_eventVect;
std::string CSimpleConfig::m_strGatherControlIP;
int			CSimpleConfig::m_nGatherControlPort;
std::string CSimpleConfig::m_strChannelControlIP;
std::vector<std::string> CSimpleConfig::m_typeVect;
int			CSimpleConfig::m_nDelayTime;


CSimpleConfig::CSimpleConfig()
{

}

CSimpleConfig::~CSimpleConfig()
{

}

int CSimpleConfig::GetProcessPath(char* cpPath)
{
    int iCount;
    iCount = readlink("/proc/self/exe", cpPath, 256);

    if (iCount < 0 || iCount >= 256)
    {
        syslog(LOG_DEBUG, "********get process absolute path failed,errno:%d !\n", errno);
        return -1;
    }

    cpPath[iCount] = '\0';

    return 0;
}

void CSimpleConfig::get_config()
{
    char exeFullPath[256];
    GetProcessPath(exeFullPath); //得到程序模块名称，全路径
    string strFullExeName(exeFullPath);
    int nLast = strFullExeName.find_last_of("/");
    strFullExeName = strFullExeName.substr(0, nLast + 1);
    strcpy(EXE_FULL_PATH, strFullExeName.c_str());
    char cpPath[256] = {0};

    char temp[256] = {0};



    //sprintf(cpPath, "%sDeviceConfig.xml", strFullExeName.c_str());
    //sprintf(cpPath, "%sLeverConfig.xml", strFullExeName.c_str());
	sprintf(cpPath, "%sDelayConfig.xml", strFullExeName.c_str());



    //简单配置部分，一层，用辅助类直接读



    XMLConfig xmlConfig(cpPath);
	//m_strGatherControlIP	= xmlConfig.GetValue("GATHER_CONTROLER_IP");
	//string strGatherPort	= xmlConfig.GetValue("GATHER_CONTROLER_PORT");
	//m_nGatherControlPort	= atoi(strGatherPort.c_str());
	m_strChannelControlIP	= xmlConfig.GetValue("CHANNE_CONTROLER_IP");


    string conntroler_port = xmlConfig.GetValue("CHANNE_CONTROLER_PORT");

    CHANNEL_CONTROLER_PORT = atoi(conntroler_port.c_str());



	strcpy(DEVICE_TAG, xmlConfig.GetValue("DEVICE_TAG").c_str());
    strcpy(DEVICE_ID, xmlConfig.GetValue("DEVICEID").c_str());





    //string comport = xmlConfig.GetValue("COMPORT");

    //COM_PORT = atoi(comport.c_str());





    //string strbaudrate = xmlConfig.GetValue("BAUDRATE");

    //BAUD_RATE = atoi(strbaudrate.c_str());





    //strcpy(DEVICE_FACTORY, xmlConfig.GetValue("FACTORYNAME").c_str());



    //// strncpy(DEVICE_SO, xmlConfig.GetValue("DEVICESONAME").c_str(), xmlConfig.GetValue("DEVICESONAME").size());

    //strcpy(DEVICE_NAME, xmlConfig.GetValue("DEVICENAME").c_str());


	string strDelayTime = xmlConfig.GetValue("DELAY_TIME");

	m_nDelayTime = atoi(strDelayTime.c_str());


	ReadConfig(cpPath);

}

void CSimpleConfig::ReadConfig(std::string strFileName)
{
	TiXmlDocument doc(strFileName.c_str());
	doc.LoadFile();

	TiXmlElement *xmlRootElement		= NULL;
	TiXmlElement *xmlSubElement			= NULL;
	TiXmlNode 	*pNode					= NULL;
	TiXmlNode 	*pNodeTmp				= NULL;
	TiXmlNode 	*pConfigNode			= NULL;
	TiXmlNode 	*pCameraConfigNode		= NULL;
	TiXmlNode   *pSnapshotConfigNode	= NULL;
	TiXmlNode 	*pRecogConfigNode		= NULL;

	pConfigNode = doc.FirstChild("Config");
	if (pConfigNode == NULL)
	{
		syslog(LOG_ERR, "pConfigNode:%p\n", pConfigNode);
		return ;
	}

	xmlRootElement = pConfigNode->ToElement();
	syslog(LOG_DEBUG, "xmlRootElement:%p\n", xmlRootElement);
	if(xmlRootElement)
	{	
		TiXmlNode *pEventSequenceNode = NULL;
		TiXmlNode *pEventNode = NULL;
		pEventSequenceNode = xmlRootElement->FirstChild("EVENT_SEQUENCE");
		if (pEventSequenceNode != NULL)
		{
			for (pEventNode = pEventSequenceNode->FirstChild("EVENT"); pEventNode != NULL; pEventNode = pEventNode->NextSibling("EVENT"))
			{

				JZ_PUBLISH_EVENT_STRUCT tmpEvent = {0};
				string strTAG 			= "";
				string strEVENT_ID		= "";
				string strSEQUENCE_NO	= "";

				xmlSubElement = pEventNode->ToElement() ;

				//if (pEventNode != NULL && pEventNode->ToElement() != NULL && pEventNode->ToElement()->GetText() != 0)
				//{
				//	strDI_BIT = pEventNode->ToElement()->GetText();
				//	printf("strDI_BIT:%s", strDI_BIT.c_str());
				//	if (i >= 0)
				//		DI_BIT[i]	= strDI_BIT[0];
				//}

				if (xmlSubElement != NULL)
				{
					if (xmlSubElement->Attribute("TAG") != NULL)
					{
						strTAG			= xmlSubElement->Attribute("TAG");
					}

					if (xmlSubElement->Attribute("EVENT_ID") != NULL)
					{
						strEVENT_ID		= xmlSubElement->Attribute("EVENT_ID");
					}

					if (xmlSubElement->Attribute("SEQUENCE_NO") != NULL)
					{
						strSEQUENCE_NO = xmlSubElement->Attribute("SEQUENCE_NO");
					}

					strncpy(tmpEvent.device_tag, strTAG.c_str(), strTAG.size() < 32 ? strTAG.size() : 31);
					strncpy(tmpEvent.event_id, strEVENT_ID.c_str(), strEVENT_ID.size() < 32 ? strEVENT_ID.size() : 31);
					strncpy(tmpEvent.sequence_no, strSEQUENCE_NO.c_str(), strSEQUENCE_NO.size() < 32 ? strSEQUENCE_NO.size() : 31);
					m_eventVect.push_back(tmpEvent);

					syslog(LOG_DEBUG, "tag,eventid,sequenceno:%s,%s,%s\n", strTAG.c_str(), strEVENT_ID.c_str(), strSEQUENCE_NO.c_str());
				}
			}
		}
	}
}
