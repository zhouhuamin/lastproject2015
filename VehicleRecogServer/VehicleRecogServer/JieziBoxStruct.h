/* 
 * File:   JieziBoxStruct.h
 * Author: root
 *
 * Created on 2015年1月28日, 上午10:12
 */

#ifndef JIEZIBOXSTRUCT_H
#define	JIEZIBOXSTRUCT_H
#include <string>

typedef enum jz_enumBoxNumberDirection
{
    FRONT       = 0,
    FRONTLEFT   = 1,
    FRONTRIGHT  = 2,
    BACK        = 3,
    BACKLEFT    = 4,
    BACKRIGHT   = 5
}BoxNumberDirection;

typedef enum jz_enumCameraDirection
{
    CFRONT       = 0,
    CLEFT        = 1,
    CRIGHT       = 2,
    CBACK        = 3
}CameraDirection;


struct structImagePath
{
    BoxNumberDirection  direct;
    std::string         strFileName;
	std::string			strGUID;
    
};

struct structImageData
{
        CameraDirection         direct;
	std::string		strIp;
	int				nPicLen;
	unsigned char	*pPicBuffer;

	structImageData()
	{
                direct   = CFRONT;
		strIp	= "";
		nPicLen	= 0;
		pPicBuffer	= NULL;
	}
};

struct structContaOwnerData
{
	int nLineNo;
	std::string strCompanyCode;
	std::string strCompanyName;
};


struct structVP_PASS_VEHICLE_INFO 
{
	std::string strDeviceID;
	std::string strHPZL;
	std::string strHPHM;
	std::string strHPYS;
	std::string strPassTime;
	std::string strLaneID;
	std::string strVehCLLX;
	std::string strVehCLYS;
	std::string strHPPos; //
	int			nPicLen;
	int			nPlateLen;
	unsigned char* pPicBuffer;
	unsigned char* pPlateBuffer;

	structVP_PASS_VEHICLE_INFO()
	{
		strDeviceID		= "";
		strHPZL			= "";
		strHPHM			= "";
		strHPYS			= "";
		strPassTime		= "";
		strLaneID		= "";
		strVehCLLX		= "";
		strVehCLYS		= "";
		strHPPos		= "";
		nPicLen			= 0;
		nPlateLen		= 0;
		pPicBuffer		= NULL;
		pPlateBuffer	= NULL;
	}

};

#endif	/* JIEZIBOXSTRUCT_H */
