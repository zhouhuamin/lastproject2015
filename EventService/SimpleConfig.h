#ifndef _SIMPLE_CONFIG_H_
#define	_SIMPLE_CONFIG_H_

#pragma warning(disable : 4786)	
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "tinyXML/xml_config.h"

class CSimpleConfig
{
public:
    CSimpleConfig();

    virtual ~CSimpleConfig();

    void get_config();
    static int GetProcessPath(char* cpPath);

 
    static int EVENT_LISTEN_PORT;

    static char DATABASE_SERVER_IP[32];
    static char DATABASE_SERVER_PORT;
    static char DATABASE_USER[32];
    static char DATABASE_PASSWORD[32];

    static char EXE_FULL_PATH[256];
};

#endif

