// MyLog.cpp: implementation of the CMyLog class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#else

#include <string>
int GetProcessPath(char* cpPath)
{
    int iCount;

    iCount = readlink("/proc/self/exe", cpPath, 256);

    if (iCount < 0 || iCount >= 256)
    {
        return -1;
    }

    cpPath[iCount] = '\0';

    return 0;
}

#endif




#include "MyLog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFreeLongLog* CMyLog::m_pLog = NULL;

ACE_Thread_Mutex CMyLog::log_lock_;

bool CMyLog::m_bInit = false;


void CMyLog::Init()
{
    if (!m_bInit)
    {
        m_bInit = true;

        if (!m_pLog)
        {

            /*
             *	获得当前绝对路径
             */
            char szFullPath[256]; // MAX_PATH

#ifdef WIN32
            GetModuleFileName(NULL, szFullPath, 256); //得到程序模块名称，全路径
#else
            GetProcessPath(szFullPath);
#endif

            string strCurrentpath(szFullPath);
            int nLast = strCurrentpath.find_last_of("\\");

            strCurrentpath = strCurrentpath.substr(0, nLast + 1);
            m_pLog = new CFreeLongLog((char*) strCurrentpath.c_str(), "ControlCenter", 4);
        }

      
    }

}

void CMyLog::Release()
{
    if (m_bInit)
    {
    
        if (m_pLog)
        {
            delete m_pLog;
            m_pLog = NULL;
        }
    }

}




