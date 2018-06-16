#if !defined _PIC_CAPTURE_H
#define _PIC_CAPTURE_H

#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;


typedef void (*_CAMERA_SNAPPER_DATA_CALLBACK)(char* szServerIP, unsigned char* pPicBuffer, int nPicLen);

class CCapturePic {
public:

    CCapturePic() {
    };

    virtual ~CCapturePic() {
    };

public:

    virtual int SetSnapDataCallback(_CAMERA_SNAPPER_DATA_CALLBACK pSnapDataCallback) {
        return 0;
    }

    virtual int SetSnapServer(char* szCameraIp, int nCmdPort, char* szUser, char* szzPass, char* szSnapperServerIP, int nServerPort) {
        return 0;
    }
    
    virtual int SetListenPort(int nListenPort){
        return 0;
    }



};

// the types of the class factories
typedef CCapturePic* create_t();
typedef void destroy_t(CCapturePic*);


#endif
