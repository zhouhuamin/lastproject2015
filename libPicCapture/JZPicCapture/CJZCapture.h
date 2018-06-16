/* 
 * File:   CJZCapture.h
 * Author: root
 *
 * Created on 2015å¹?1??23??, ä¸???5:33
 */

#ifndef CJZCAPTURE_H
#define	CJZCAPTURE_H

#include "../include/PicCapture.h"


#include "include/NetCamera_SnapServer.h"
#include "include/NetCamera.h"
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MutexGuard.h"

struct T_SnapperInfo
{
    char   szCameraIP[32];
    char*  pPicBuffer;
    int    nPicLen;
};

#define MAX_BUFFER_NUM   (8)

class CJZCapture : public CCapturePic {
public:
    CJZCapture();
    virtual ~CJZCapture();


public:
    int SetSnapDataCallback(_CAMERA_SNAPPER_DATA_CALLBACK pSnapDataCallback);
    int SetSnapServer(char* szCameraIp, int nCmdPort, char* szUser, char* szzPass, char* szSnapperServerIP, int nServerPort);
    int SetListenPort(int nListenPort);
    
    
    int GetImage();
    int HandleImage();
    
private:

private:
    _CAMERA_SNAPPER_DATA_CALLBACK m_pSnapDataCallback;

    NetCamera_SnapServer* m_PSnapServer;
    NetCamera m_NetCamera;
    int m_nListemPort;
    
    CSysMutex   buffer_mutex;
    CSysMutex   handle_mutex;
    int         m_nBufferIndex;
    T_SnapperInfo* m_pSnapBuffer[MAX_BUFFER_NUM];
    
    std::list<T_SnapperInfo*> m_SnapBufferList;
    
    
};

#endif	/* CJZCAPTURE_H */

