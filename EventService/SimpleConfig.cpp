#include "SimpleConfig.h"
#include <errno.h>
#include "MyLog.h"
#include <string>
using namespace std;


int CSimpleConfig::EVENT_LISTEN_PORT;

char CSimpleConfig::DATABASE_SERVER_IP[32];
char CSimpleConfig::DATABASE_SERVER_PORT;
char CSimpleConfig::DATABASE_USER[32];
char CSimpleConfig::DATABASE_PASSWORD[32];

char CSimpleConfig::EXE_FULL_PATH[256];

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
        CMyLog::m_pLog->_XGSysLog("********get process absolute path failed,errno:%d !\n", errno);
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

    sprintf(cpPath, "%sEventConfig.xml", strFullExeName.c_str());

    //简单配置部分，一层，用辅助类直接读

    XMLConfig xmlConfig(cpPath);

  
    string center_port = xmlConfig.GetValue("EVENT_LISTEN_PORT");
    EVENT_LISTEN_PORT = atoi(center_port.c_str());



    strcpy(DATABASE_SERVER_IP, xmlConfig.GetValue("DATABASE_SERVER_IP").c_str());

    string database_port = xmlConfig.GetValue("DATABASE_SERVER_PORT");
    DATABASE_SERVER_PORT = atoi(database_port.c_str());


    strcpy(DATABASE_USER, xmlConfig.GetValue("DATABASE_USER").c_str());
    strcpy(DATABASE_PASSWORD, xmlConfig.GetValue("DATABASE_PASSWORD").c_str());

}


