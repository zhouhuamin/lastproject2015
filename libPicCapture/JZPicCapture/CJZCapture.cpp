/* 
 * File:   CJZCapture.cpp
 * Author: root
 * 
 * Created on 2015骞?1??23??, 涓???5:33
 */

#include "CJZCapture.h"

extern "C" CCapturePic* create()
{
    return new CJZCapture;
}

extern "C" void destroy(CCapturePic* p)
{
    delete p;
}

extern "C"
{
    typedef void*(*THREADFUNC)(void*);
}

#define MAX_PIC_SIZE (1*1024*1024)

CJZCapture::CJZCapture()
{
    m_pSnapDataCallback = NULL;
    m_nListemPort = 9999;
    
    m_PSnapServer=new NetCamera_SnapServer;
    
    m_nBufferIndex = 0;

    for (int i = 0; i < MAX_BUFFER_NUM; i++)
    {
        T_SnapperInfo* pSnapBuffer = new T_SnapperInfo;
        pSnapBuffer->pPicBuffer = new char[MAX_PIC_SIZE];
        sprintf(pSnapBuffer->szCameraIP, "%s", "");
        pSnapBuffer->nPicLen = 0;

        m_pSnapBuffer[i] = pSnapBuffer;
    }
}

CJZCapture::~CJZCapture()
{

}

int CJZCapture::SetSnapDataCallback(_CAMERA_SNAPPER_DATA_CALLBACK pSnapDataCallback)
{
    m_pSnapDataCallback = pSnapDataCallback;
}

int CJZCapture::SetSnapServer(char* szCameraIp, int nCmdPort, char* szUser, char* szzPass, char* szSnapperServerIP, int nServerPort)
{
    m_NetCamera.SetIPAddress(szCameraIp, 8886, 8888);

    if (m_NetCamera.IdentifyCameraType() < 0)
    {
        printf("camera offline!\r\n");
        return -1;
    }

    m_NetCamera.ConnectCamera();

    unsigned long server_ip = inet_addr(szSnapperServerIP);
    m_NetCamera.UpdateParam(SERVER_IP, server_ip);


    m_NetCamera.UpdateParam(SERVER_PORT, nServerPort);


    m_NetCamera.UpdateParam(SAVE_PARAM, 0);
    m_NetCamera.ConnectCamera(0, 0);

}

void* GetImageProc(void * pParam)
{
    if (!pParam)
    {
        return 0;
    }
    CJZCapture* pPicCaptureHandler = (CJZCapture*) pParam;
    if (!pPicCaptureHandler)
    {
        return 0;
    }

    pPicCaptureHandler->GetImage();

    usleep(500 * 1000);

    pthread_exit(NULL);

    return 0;
}


void* HandleImageProc(void * pParam)
{
    if (!pParam)
    {
        return 0;
    }
    CJZCapture* pPicCaptureHandler = (CJZCapture*) pParam;
    if (!pPicCaptureHandler)
    {
        return 0;
    }

    pPicCaptureHandler->HandleImage();

    usleep(500 * 1000);

    pthread_exit(NULL);

    return 0;
}


int CJZCapture::SetListenPort(int nListenPort)
{
    m_nListemPort = nListenPort;

    {
        pthread_t localThreadId;
        int nThreadErr = pthread_create(&localThreadId, NULL,
                (THREADFUNC) GetImageProc, this);
        if (nThreadErr == 0)
        {
            pthread_detach(localThreadId); //	释放线程私有数据,不必等待pthread_join();

        }
        
        
        nThreadErr = pthread_create(&localThreadId, NULL,
                (THREADFUNC) HandleImageProc, this);
        if (nThreadErr == 0)
        {
            pthread_detach(localThreadId); //	释放线程私有数据,不必等待pthread_join();

        }

    }


}

int CJZCapture::GetImage()
{

    if(!m_PSnapServer)
    {
        return -1;
    }
    
    int nRet=m_PSnapServer->StartRecv(m_nListemPort);
    if (nRet <= 0)
    {
        printf("start server fail!\r\n");
        return 0;
    }


    while (1)
    {
        unsigned long length = 0;
        length = m_PSnapServer->GetImageSize();
        if (length == 0)
        {
            usleep(10 * 1000);
            continue;
        }

        T_SnapperInfo* pSnapBuffer=NULL;
        
        {
            CMutexGuard guard(buffer_mutex);
            pSnapBuffer=m_pSnapBuffer[m_nBufferIndex];
            m_nBufferIndex++;
            if(m_nBufferIndex>MAX_BUFFER_NUM-1)
            {
                m_nBufferIndex=0;
            }
        }
        
        if (!pSnapBuffer)
        {
            printf("new memory fail!\r\n");
            usleep(10 * 1000);
            continue;
        }
        
        int result = m_PSnapServer->GetImage((unsigned char*)pSnapBuffer->pPicBuffer);
        if (result == 0)
        {
            FRAME_HEADER * pFh = (FRAME_HEADER*) pSnapBuffer->pPicBuffer;
            in_addr addr;
            addr.s_addr = pFh->ip;
            
            switch (pFh->format)
            {
                case 7:
                {
                    sprintf(pSnapBuffer->szCameraIP,"%s",inet_ntoa(addr));
                    pSnapBuffer->nPicLen=pFh->len;
                    
                    {
                        CMutexGuard guard(handle_mutex);
                        m_SnapBufferList.push_back(pSnapBuffer);
                    }      
                }

                    break;

                default:
                    break;
            }
        }
        else
        {
            usleep(1000 * 1000);
        }
    }
  
    return 0;
}

int CJZCapture::HandleImage()
{
    while(1)
    {
        {
            CMutexGuard guard(handle_mutex);
            if(m_SnapBufferList.size()>0)
            {
                T_SnapperInfo* pSnapBuffer=m_SnapBufferList.front();
                m_SnapBufferList.pop_front();
                
                if(m_pSnapDataCallback)
                {
                    m_pSnapDataCallback(pSnapBuffer->szCameraIP,(unsigned char*)(pSnapBuffer->pPicBuffer+sizeof(FRAME_HEADER)),pSnapBuffer->nPicLen);
                }    
            }
        }
          
        usleep(20 * 1000);
        
    }
}
