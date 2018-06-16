#if !defined DV_DEVICE_BASE_H
#define DV_DEVICE_BASE_H

#pragma warning(disable:4786)

#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;

/*
 *	����ֵ����
 */
#define     MAX_CHANNEL_TIMEOUT              1200       //ͨ������ʱ��(s)
#define     MAX_DEVICE_TIMEOUT               20        //�豸����ʱ��(s)

#define		VP_RESULT_SUCCESSFUL			0X00		//�ɹ�
#define		VP_ERROR_UNKNOW					0X01		//����δ֪ԭ��
#define		VP_ERROR_NOTLOGIN				0X02		//����δ��½


#define     DEVICE_STATE_LOGIN_SUCCESS  0x01
#define     DEVICE_STATE_LOGIN_FAIL     0x02 
#define     DEVICE_STATE_OFF_LINE       0x03
#define     DEVICE_STATE_RECONNECTTED   0x04

struct VP_DEVICE_INFO {
    char device_id[32]; //�豸ID
    char device_ip[32]; //�豸IP��ַ
    char point_id[32]; //�豸IP��ַ
    int channel_num; //�豸ͨ��
    int command_port; //�豸����˿�
    char device_user[32]; //�豸�û�
    char device_pass[32]; //�豸����
    char  dir_id[32];     
    int  lane_starter;
    int  pic_width;
    int  pic_height;
     char road_name[128];
     int is_road_gate;
     char szKKBH[16];
     char szSBBH[16];
    char szFXLX[8];
    int  nLimitSpeed;
};

struct VP_SendPicInfo {
    int nPicType; //0,������һ��ͼƬ 1�������ڶ���ͼƬ 2������������ͼƬ 3������ͼƬ  4����ʻԱͼƬ
    int nOffSet;
    int nPicDataLen;

};


struct VP_PASS_VEHICLE_INFO {
    char szDeviceID[16];
    char szHPZL[8];
    char szHPHM[16];
    char szHPYS[16];
    char szPassTime[32];
    char szLaneID[8];
    char szVehCLLX[4];
    char szVehCLYS[16];
    char szHPPos[64]; //
    int  nPicLen;
    int  nPlateLen;
    unsigned char* pPicBuffer;
    unsigned char* pPlateBuffer;
    
};


#ifdef WIN32
typedef void(CALLBACK *_DEVICE_STATUS_CALLBACK_) (char* szDeviceIP, unsigned char ucStatus);
#else
typedef void (*_DEVICE_STATUS_CALLBACK_) (char* szDeviceIP, unsigned char ucStatus);
#endif


typedef void(*_PASS_VEHICLE_CALLBACK_) (VP_PASS_VEHICLE_INFO& pVehicleInfo);


class CDeviceBase {
public:

    CDeviceBase() {
    }
    ;

    virtual ~CDeviceBase() {
    };

public:

    /*
     *�豸��¼��ע��
     */
    virtual int Login() {
        return 0;
    }

    virtual int Logout() {
        return 0;
    }

    /*
     *�����豸������Ϣ
     */
    virtual int SetDeviceInfo(VP_DEVICE_INFO *pDeviceInfo) {
        return 0;
    }

    /*
     *	�豸����״̬
     */
    virtual bool IsConnect() {
        return false;
    }

    /*
     *
     */
    virtual int SetPicSavePath(char* szSavePath) {
        return 0;
    }

    virtual int SetDeviceStatuesCB(_DEVICE_STATUS_CALLBACK_ cbDeviceStatus) {
        return 0;
    }

    virtual int SetPassVehicleCB(_PASS_VEHICLE_CALLBACK_ cbPassVehicle) {
        return 0;
    }
};

// the types of the class factories
typedef CDeviceBase* create_t();
typedef void destroy_t(CDeviceBase*);


#endif

