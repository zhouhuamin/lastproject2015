#ifndef RECO_CLIENT_SDK_H_
#define RECO_CLIENT_SDK_H_

//��װ�������

struct JZ_Region {
    int x;
    int y;
    int width;
    int height;
};
//��ɫ

enum JZ_Color {
    RED, YELLOW,BLUE, WHITE, GRAY, GREEN, OTHER
};

//��װ������з�ʽ

enum JZ_AlignType {
    H_Align, T_Align
}; //H:horizontal ����  T��tandem ����

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
    JZ_AlignType Atype; //���з�ʽ ���Ż�������
    int count; //����
};

struct JZ_RecoContaID {
    int  nResult;
    char ContaID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
    char Type[5]; //��װ������
    float fAccuracy;
    JZ_Region IDReg; //����
    JZ_Region TypeReg; //����
    JZ_Color color; //��ɫ
    JZ_Align ali; //���з�ʽ
    
};

struct JZ_ContaIDCheckBitRequest
{
	char contaID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
};

struct JZ_ContaIDCheckBitResponse
{
	char contaID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
	char VerifyCode;
};

struct JZ_ContaIDBefore 
{
	char contaID[12];	//��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
	char Type[5];		//��װ������
	float fAccuracy;
	JZ_Color color;		//��ɫ
	JZ_Align ali;		//���з�ʽ
};

struct JZ_ContaIDAfter
{
	char contaID[12];	//��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
	char Type[5];		//��װ������
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
	int nCount;	//1:����;2:˫��
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
