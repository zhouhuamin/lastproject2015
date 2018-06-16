#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#define MAX_MSG_BODYLEN  20*1024

#define SYS_NET_MSGHEAD                          "0XJZTECH"
#define SYS_NET_MSGTAIL                           "NJTECHJZ"


#define SYS_SYSTEM                             0X00
#define SYS_SUB_SYSTEM_ICCARDREADER         1000000
#define SYS_SUB_SYSTEM_LEVERCONTROLER      2000000
#define SYS_SUB_SYSTEM_LCDCONTROLER        3000000


#define SYS_MSG_SYSTEM_REGISTER_REQ            SYS_SYSTEM+1
#define SYS_MSG_SYSTEM_REGISTER_ACK           SYS_SYSTEM+2

#define SYS_MSG_SYSTEM_MSG_KEEPLIVE            SYS_SYSTEM+3
#define SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK        SYS_SYSTEM+4

#define SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH  SYS_SYSTEM+5


#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS            SYS_SYSTEM+7
#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK        SYS_SYSTEM+8

#define SYS_MSG_EVENT_SUBSCRIBE               SYS_SYSTEM+0X11                                        //�¼�����
#define SYS_MSG_EVENT_SUBSCRIBE_ACK           SYS_SYSTEM+0X12                                        //�¼����ķ���

#define SYS_MSG_EVENT_EXIT                    SYS_SYSTEM+0X16


#define SYS_MSG_SYSEVENT_PUBLISH                  SYS_SYSTEM+0X20                                        //�¼�����
#define SYS_EVENT_TYPE_UPLOAD_CUSTOMS_DATA  SYS_SYSTEM+0X01


#define SYS_MSG_ICCARDREADER_READ            SYS_SUB_SYSTEM_ICCARDREADER+3
#define SYS_MSG_ICCARDREADER_READ_ACK       SYS_SUB_SYSTEM_ICCARDREADER+4


#define SYS_MSG_LEVERCONTROLER_ON            SYS_SUB_SYSTEM_ICCARDREADER+5
#define SYS_MSG_LEVERCONTROLER_ON_ACK       SYS_SUB_SYSTEM_ICCARDREADER+6

#define SYS_MSG_LEVERCONTROLER_OFF         SYS_SUB_SYSTEM_ICCARDREADER+7
#define SYS_MSG_LEVERCONTROLER_OFF_ACK    SYS_SUB_SYSTEM_ICCARDREADER+8


#define SYS_MSG_LCD_DISPLAY                     SYS_SUB_SYSTEM_ICCARDREADER+9
#define SYS_MSG_LCD_DISPLAY_ACK              SYS_SUB_SYSTEM_ICCARDREADER+10

#define SYS_MSG_REGATHERDATA					0X13   

#define SYS_MSG_QUERY_GATHERDATA				0X15
#define SYS_MSG_QUERY_GATHERDATA_ACK			0X16

#define SYS_MSG_DEVICE_CTRL						0X17
#define CLIENT_EXCEPTIION_FREE					0X18


#define SYS_MSG_QUERY_GATHERPICDATA             0X23
#define SYS_MSG_QUERY_GATHERPICDATA_ACK			0X24



#define SYS_EVENT_ICREADER_READ_COMPLETE   "1000000000"      //ic�����������
#define SYS_EVENT_PACKET_DATA                "C000000001"     //�������

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

struct T_DeviceServerRegister_Req {
    char device_id[32]; //����ģ��ID��Ψһֵ
};

struct T_DeviceServerRegister_Ack {
    int reg_result; //ע�������ɹ�����ʧ��
    int next_action; //������������������ʼ�������ǵȴ�����ָ��
};

struct T_KeepliveAck {
    int nResult;
};

struct T_ICReader_Data {
    char event_id[32];
    char reader_data[64];
};

struct T_Sequence_ControlData {
    char event_id[32];
    char ic_card_data[32];
    int has_ic;
    int xml_len;
    char xml_data[];

};

struct T_Len {
    int length;
};

struct NET_EVENT_SUBSCRIBE {
    char client_type;
    char user_name[32];
    char pass_word[32];
    char event_serial[32];

};

struct NET_EVENT_SUBSCRIBE_ACK {
    int result;
};

struct T_Upload_Customs_Data {
    char szChannelNo[32];
    char szSequenceNo[32];
    int is_data_full; //�����Ƿ�����
    int nCustomsDataLen;
    char szCustomsData[];
};

struct NET_EVENT {
    char main_type[32];
    int sub_type;
};

struct NET_EVENT_UPLOAD_CUSTOMSDATA {
    char main_type[32];
    int sub_type;
    T_Upload_Customs_Data customs_data;
};

struct LOG_IN {
    char user_name[128];
    char pass_word[128];
};

struct MONITOR_PORT {
    char channel_no[32];
    char area_name[100];
    char channel_name[100];
    int status; //0,�ɹ�����  1-��������  2-ֹͣ����
};

struct LOG_IN_ACK {
    int result;
    int monitor_num;
    int is_end;
    MONITOR_PORT port_info[];
};

struct IC_INFO {
    char szDR_IC_NO[16]; //IC����
    char szIC_DR_CUSTOMS_NO[128]; //˾����Ϣ
    char szIC_CO_CUSTOMS_NO[128]; //���ر��struct 
    char szIC_BILL_NO[32]; //��֤��
    int lIC_GROSS_WT; //���ڵذ�����
    char szIC_VE_CUSTOMS_NO[32]; //���ڳ������ر��
    char szIC_VE_NAME [32]; //���ڳ��ƺ�
    char szIC_CONTA_ID [32]; // ���ڼ�װ���
    char szIC_ESEAL_ID [48]; // ���ڹ�����
    char szIC_EX_DATA [256]; //������չ����
    char szIC_BUSS_TYPE[16]; //ҵ������
};

struct CAR_INFO//���ӳ��ƽṹ��Ϣ
{
    char szVE_NAME [128]; //���ƺ�
    char szCAR_EC_NO [128]; //��ǩID���������ԡ�|���ָ���
    int lVE_WT; //�ذ�����
    char szVE_CUSTOMS_NO[32]; //���ر��
};

struct TRAILER_INFO//���ܽṹ��Ϣ
{
    char szTR_NAME [128]; //���ƺ�
    char szTR_EC_NO [128]; //���ܱ�ǩID���������ԡ�|���ָ���
    int lTR_WT; //����
};

struct CONTA_INFO//��װ��ṹ��Ϣ
{
    int iCONTA_NUM; //������
    int iCONTA_RECO; //ʶ���Ƿ�������0-������1-����
    char szCONTA_ID_F[32]; //ǰ���
    char szCONTA_ID_B[32]; // �����
    char szCONTA_MODEL_F[16]; //ǰ����
    char szCONTA_MODEL_B[16]; // ������
};

struct ESEAL_INFO//���ӹ�����Ϣ�ṹ��
{
    char szESEAL_ID [48]; // ������
    char szESEAL_KEY [256]; //����Կ
};

struct GPS_INFO //GPS�ṹ��Ϣ
{
    char szVE_NAME [128]; //���ƺ�
    char szGPS_ID [32]; //GPS ID��
    char szORIGIN_CUSTOMS[128]; //��ʼ��ַ
    char szDEST_CUSTOMS[128]; //ָ�˵�
};

struct GATHER_INFO //�ɼ��౨��
{
    int iArrange_Type; //�������0-�ɼ����ģ�1-���ɱ��� 
    char szChannelNo[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szSeq_No[32]; //���к�
    IC_INFO IC;
    CAR_INFO CAR;
    TRAILER_INFO TRAILER;
    CONTA_INFO CONTA;
    ESEAL_INFO Eseal;
    float Gross_WT;
    int iMODI_FLAG; //�������0-������1-���ɣ�2-�޸�
};

struct QUERY_GATHER_INFO//��ʷ��ѯ����
{
    int iArrange_Type; //������� 2-��ʷ��ѯ�������; 
    char szChannelNo[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szSeq_No[32]; //���к�
    IC_INFO IC;
    CAR_INFO CAR;
    TRAILER_INFO TRAILER;
    CONTA_INFO CONTA;
    ESEAL_INFO Eseal;
    float Gross_WT;
    GPS_INFO GPS;
    char szCHECK_RESULT[32]; //���бȶ�ָ��
    char szCOMMAND[32]; //����ָ��
    int iMODI_FLAG; //�������0-������1-���ɣ�2-�޸�
    int iREL_TYPE; //���з�ʽ��0-�������У�1-�˹�����
    char szOp_Reason[128]; //̧��ԭ��
    char szForm_ID[32]; //��֤��
    char szOp_Hint[128];
};

struct NET_EVENT_UPLOAD_CUSTOMSDATA_ACK {
    char szChannelNo[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    int sub_type; //�������ͣ�Ŀǰ��Ϊ�ɼ����ĺͷ���ָ���
    T_Upload_Customs_Data customs_data;
};

struct NET_REGATHER//���ɽṹ��
{
    int iArrange_Type; //�������1-�ֹ�¼�����ݷ�ʽ���ɣ�2-����ǰ���豸��ʽ����; 
    char szChnnl_No[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szDeviceTag[16]; // ����ǰ���豸����ʱ���ݵ��豸��ʶ��
    T_Upload_Customs_Data customs_data; //�ֹ���ʽ��������ŵı��ģ�����Ϊ��

};

struct NET_QUERY_GATHERINFO//�ͻ������ò�ѯһ��ʱ��˵���ʷ��¼
{
    char szChnnl_No[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szBeginTime[32];
    char szStopTime[32];
};

struct NET_GATHERINFO_ACK {
    int nResult; //��ѯ�����0-�ɹ���1-ʧ��
    int nGatherInfoNum; //��ѯ���ݸ���
    int nIsEnd;
    QUERY_GATHER_INFO gatherInfo[]; //��Ų�ѯ����
};

struct NET_STREAM_INFO {
    int stream_type; //0,ǰ��, 1,��ǰ�䣬2����ǰ�� 3,B  4 �����  5���Һ���
    int stream_len;
    int off_set;
};

struct NET_CONTAPIC_REQ {
    char channel_no[32];
    char sequence_no[32];
};

struct NET_CONTAPIC_ACK {
    int stream_len;
    char channel_no[32];
    char sequence_no[32];
    int stream_num;
    NET_STREAM_INFO stream_info[16];
    char stream_buffer[];
};

struct NET_REGATHERDATA_REQ {
    int nRegatherType; //1,�ֹ�¼��  2������ǰ���豸��ʽ
    char channel_no[32];
    char device_tag[16];
    T_Upload_Customs_Data customs_data; //�ֹ���ʽ��Ч��������Ч

};

struct CLIENT_COMMAND_INFO//�����౨��
{
    int iArrange_Type; //�������0-���б��ģ�1-�˹�����;2-�豸����
    char szChnnl_No[32]; // ��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szDEAL_TYPE[4]; //�˹�������������ͣ�����Ϊ�գ�00��̧��01�����02�������ɣÿ���д	03�������ذ�04���������ӳ���05: ����������
    char szMEM[256]; //��ע˵�������������ݿ��й����ڲ�ѯ��
    int nCustomsDataLen; //����XML��С
    char szCustomsData[]; //���XML�ɼ����ĺͷ��б���	
};

struct CLIENT_EXCEPTION_FREE//�쳣���п��Ʊ���
{
	int iArrange_Type; //�������3-�쳣���п���
	char szChnnl_No[32]; // ��վ��+ͨ����+������־��IΪ����EΪ����
	char szSequence_No[32]; // ��վ��+ͨ����+������־��0Ϊ����1Ϊ����
	char szDEAL_TYPE[4]; //�쳣���б�ǣ�ֵΪ00
	char szMEM[256]; //��ע˵�������������ݿ��й����ڲ�ѯ��
	int nExceptionFreeDataLen; //�쳣���б���XML��С
	char szExceptionFreeData[]; //����쳣����XML���Ʊ��ģ����д��control_info	
};

#endif
