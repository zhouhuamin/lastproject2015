#ifndef RECO_CLIENT_SDK_H_
#define RECO_CLIENT_SDK_H_

//集装箱号区域

struct JZ_Region {
    int x;
    int y;
    int width;
    int height;
};
//颜色

enum JZ_Color {
    RED, YELLOW,BLUE, WHITE, GRAY, GREEN, OTHER
};

//集装箱号排列方式

enum JZ_AlignType {
    H_Align, T_Align
}; //H:horizontal 横排  T：tandem 竖排

//typedef enum jz_enumBoxNumberDirection
//{
//	FRONT       = 0,
//	FRONTLEFT   = 1,
//	FRONTRIGHT  = 2,
//	BACK        = 3,
//	BACKLEFT    = 4,
//	BACKRIGHT   = 5
//}BoxNumberDirection;

struct JZ_Align {
    JZ_AlignType Atype; //排列方式 横排或者竖排
    int count; //排数
};

struct JZ_RecoContaID {
    int  nResult;
    char ContaID[12]; //集装箱号：4位字母，6位数字，1位校验码
    char Type[5]; //集装箱类型
    float fAccuracy;
    JZ_Region IDReg; //区域
    JZ_Region TypeReg; //区域
    JZ_Color color; //颜色
    JZ_Align ali; //排列方式
    
};

struct JZ_ContaIDCheckBitRequest
{
	char contaID[12]; //集装箱号：4位字母，6位数字，1位校验码
};

struct JZ_ContaIDCheckBitResponse
{
	char contaID[12]; //集装箱号：4位字母，6位数字，1位校验码
	char VerifyCode;
};

struct JZ_ContaIDBefore 
{
	char contaID[12];	//集装箱号：4位字母，6位数字，1位校验码
	char Type[5];		//集装箱类型
	float fAccuracy;
	JZ_Color color;		//颜色
	JZ_Align ali;		//排列方式
};

struct JZ_ContaIDAfter
{
	char contaID[12];	//集装箱号：4位字母，6位数字，1位校验码
	char Type[5];		//集装箱类型
};


struct JZ_ContaIDCountRequest
{
	JZ_ContaIDBefore FrontContaID;		
	JZ_ContaIDBefore FrontLeftContaID;	
	JZ_ContaIDBefore FrontRightContaID; 
	JZ_ContaIDBefore BackContaID;	
	JZ_ContaIDBefore BackLeftContaID;	
	JZ_ContaIDBefore BackRightContaID;
};
struct JZ_ContaCountResponse
{
	int nCount;	//1:单箱;2:双箱
};

struct JZ_ContaIDCorrectionRequest
{
	JZ_ContaIDBefore FrontContaID;		
	JZ_ContaIDBefore LeftContaID;	
	JZ_ContaIDBefore BackContaID;
	JZ_ContaIDBefore RightContaID; 
};

struct JZ_ContaIDCorrectionResponse
{
	JZ_ContaIDAfter ContaIDAfter;	
};


typedef void (*_RECO_RESULT_CALLBACK)(char* szRecoSequence,JZ_RecoContaID* pRecoREsult);

extern "C" int RecoInit(char* szRecoServerIP,int nRecoServerPort);
extern "C" int ContaReco(char* szRecoSequence, char *ImagePath);
extern "C" int Release();
extern "C" int SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallbase);

extern "C" int GetContaIDCheckBit(JZ_ContaIDCheckBitRequest *pRequest,		JZ_ContaIDCheckBitResponse *pResponse);
extern "C" int GetContaIDCorrected(JZ_ContaIDCorrectionRequest *pRequest,	JZ_ContaIDCorrectionResponse *pResponse);
extern "C" int GetContaIDCount(JZ_ContaIDCountRequest *pRequest,			JZ_ContaCountResponse *pResponse);

#endif
