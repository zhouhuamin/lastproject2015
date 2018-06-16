#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#define MAX_MSG_BODYLEN  (2*1024*1024)

#define SYS_NET_MSGHEAD                          "0XJZTECH"
#define SYS_NET_MSGTAIL                           "NJTECHJZ"


#define SYS_MSG_SYSTEM_REGISTER_REQ            0X01
#define SYS_MSG_SYSTEM_REGISTER_ACK            0X02

#define SYS_MSG_SYSTEM_MSG_KEEPLIVE            0X03
#define SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK        0X04

#define SYS_MSG_SYSTEM_MSG_RECO_REQ            0X05
#define SYS_MSG_SYSTEM_MSG_RECO_ACK            0X06

#define SYS_MSG_CONTAID_CHECK_BIT_REQ          0X11
#define SYS_MSG_CONTAID_CHECK_BIT_RES          0X12

#define SYS_MSG_CONTAID_CORRECTED_REQ          0X13
#define SYS_MSG_CONTAID_CORRECTED_RES          0X14

#define SYS_MSG_CONTAID_COUNT_REQ			   0X15
#define SYS_MSG_CONTAID_COUNT_RES			   0X16


struct NET_PACKET_HEAD {
    int msg_type;
    int packet_len;
    char version_no[16];
    int proxy_count;
    int net_proxy[10];
};

struct NET_PACKET_MSG {
    NET_PACKET_HEAD msg_head;
    char msg_body[MAX_MSG_BODYLEN];
};

struct T_Register_Req {
    int client_type;
};

struct T_Register_Ack {
    int reg_result; //ע�������ɹ�����ʧ��
};

struct T_KeepliveAck {
    int nResult;
};

struct T_ContaRecoReq {
    char req_sequence[32];
    int pic_len;
    char pic_buffer[];
};

struct Region {
    int x;
    int y;
    int width;
    int height;
};
//��ɫ

enum Color {
    red, blue, white, gray, green, other
};

enum JZ_Color {
	RED, YELLOW,BLUE, WHITE, GRAY, GREEN, OTHER
};


//��װ������з�ʽ

enum AlignType {
    H, T
}; //H:horizontal ����  T��tandem ����

struct Align {
    AlignType Atype; //���з�ʽ ���Ż�������
    int count; //����
};

//��װ������з�ʽ

enum JZ_AlignType {
	H_Align, T_Align
}; //H:horizontal ����  T��tandem ����

struct JZ_Align {
	JZ_AlignType Atype; //���з�ʽ ���Ż�������
	int count; //����
};

struct ContaID {
    char ID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
    char Type[5]; //��װ������
    Region reg; //����
    Color color; //��ɫ
    Align ali; //���з�ʽ
};

struct T_ContaRecoResult {
    char req_sequence[32];
    int result;
    ContaID conta_id;
};


struct NET_KEEP_LIVE {
    int client_type;
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
	JZ_ContaIDAfter contaIDAfter;	
};

#endif
