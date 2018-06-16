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

#define SYS_MSG_EVENT_SUBSCRIBE               SYS_SYSTEM+0X11                                        //事件订阅
#define SYS_MSG_EVENT_SUBSCRIBE_ACK           SYS_SYSTEM+0X12                                        //事件订阅返回

#define SYS_MSG_EVENT_EXIT                    SYS_SYSTEM+0X16


#define SYS_MSG_SYSEVENT_PUBLISH                  SYS_SYSTEM+0X20                                        //事件发布
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



#define SYS_EVENT_ICREADER_READ_COMPLETE   "1000000000"      //ic读卡器读完成
#define SYS_EVENT_PACKET_DATA                "C000000001"     //打包数据

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
    char device_id[32]; //服务模块ID，唯一值
};

struct T_DeviceServerRegister_Ack {
    int reg_result; //注册结果，成功还是失败
    int next_action; //后续动作，是立即开始工作还是等待启动指令
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
    int is_data_full; //数据是否完整
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
    int status; //0,成功连接  1-工作正常  2-停止工作
};

struct LOG_IN_ACK {
    int result;
    int monitor_num;
    int is_end;
    MONITOR_PORT port_info[];
};

struct IC_INFO {
    char szDR_IC_NO[16]; //IC卡号
    char szIC_DR_CUSTOMS_NO[128]; //司机信息
    char szIC_CO_CUSTOMS_NO[128]; //海关编号struct 
    char szIC_BILL_NO[32]; //单证号
    int lIC_GROSS_WT; //卡内地磅重量
    char szIC_VE_CUSTOMS_NO[32]; //卡内车辆海关编号
    char szIC_VE_NAME [32]; //卡内车牌号
    char szIC_CONTA_ID [32]; // 卡内集装箱号
    char szIC_ESEAL_ID [48]; // 卡内关锁号
    char szIC_EX_DATA [256]; //卡内扩展数据
    char szIC_BUSS_TYPE[16]; //业务类型
};

struct CAR_INFO//电子车牌结构信息
{
    char szVE_NAME [128]; //车牌号
    char szCAR_EC_NO [128]; //标签ID（允许多个以“|”分隔）
    int lVE_WT; //地磅重量
    char szVE_CUSTOMS_NO[32]; //海关编号
};

struct TRAILER_INFO//车架结构信息
{
    char szTR_NAME [128]; //车牌号
    char szTR_EC_NO [128]; //车架标签ID（允许多个以“|”分隔）
    int lTR_WT; //重量
};

struct CONTA_INFO//集装箱结构信息
{
    int iCONTA_NUM; //箱数量
    int iCONTA_RECO; //识别是否正常：0-正常；1-错误
    char szCONTA_ID_F[32]; //前箱号
    char szCONTA_ID_B[32]; // 后箱号
    char szCONTA_MODEL_F[16]; //前箱型
    char szCONTA_MODEL_B[16]; // 后箱型
};

struct ESEAL_INFO//电子关锁信息结构体
{
    char szESEAL_ID [48]; // 关锁号
    char szESEAL_KEY [256]; //锁密钥
};

struct GPS_INFO //GPS结构信息
{
    char szVE_NAME [128]; //车牌号
    char szGPS_ID [32]; //GPS ID号
    char szORIGIN_CUSTOMS[128]; //起始地址
    char szDEST_CUSTOMS[128]; //指运地
};

struct GATHER_INFO //采集类报文
{
    int iArrange_Type; //处理类别：0-采集报文；1-补采报文 
    char szChannelNo[32]; //场站号+通道号+进出标志（0为进、1为出）
    char szSeq_No[32]; //序列号
    IC_INFO IC;
    CAR_INFO CAR;
    TRAILER_INFO TRAILER;
    CONTA_INFO CONTA;
    ESEAL_INFO Eseal;
    float Gross_WT;
    int iMODI_FLAG; //数据类别：0-正常；1-补采；2-修改
};

struct QUERY_GATHER_INFO//历史查询报文
{
    int iArrange_Type; //处理类别： 2-历史查询结果报文; 
    char szChannelNo[32]; //场站号+通道号+进出标志（0为进、1为出）
    char szSeq_No[32]; //序列号
    IC_INFO IC;
    CAR_INFO CAR;
    TRAILER_INFO TRAILER;
    CONTA_INFO CONTA;
    ESEAL_INFO Eseal;
    float Gross_WT;
    GPS_INFO GPS;
    char szCHECK_RESULT[32]; //放行比对指令
    char szCOMMAND[32]; //控制指令
    int iMODI_FLAG; //数据类别：0-正常；1-补采；2-修改
    int iREL_TYPE; //放行方式：0-正常放行；1-人工放行
    char szOp_Reason[128]; //抬杆原因
    char szForm_ID[32]; //单证号
    char szOp_Hint[128];
};

struct NET_EVENT_UPLOAD_CUSTOMSDATA_ACK {
    char szChannelNo[32]; //场站号+通道号+进出标志（0为进、1为出）
    int sub_type; //报文类型，目前分为采集报文和放行指令报文
    T_Upload_Customs_Data customs_data;
};

struct NET_REGATHER//补采结构体
{
    int iArrange_Type; //补采类别：1-手工录入数据方式补采；2-启动前端设备方式补采; 
    char szChnnl_No[32]; //场站号+通道号+进出标志（0为进、1为出）
    char szDeviceTag[16]; // 启动前端设备补采时传递的设备标识号
    T_Upload_Customs_Data customs_data; //手工方式补采所存放的报文；否则为空

};

struct NET_QUERY_GATHERINFO//客户端设置查询一个时间端的历史记录
{
    char szChnnl_No[32]; //场站号+通道号+进出标志（0为进、1为出）
    char szBeginTime[32];
    char szStopTime[32];
};

struct NET_GATHERINFO_ACK {
    int nResult; //查询结果，0-成功，1-失败
    int nGatherInfoNum; //查询数据个数
    int nIsEnd;
    QUERY_GATHER_INFO gatherInfo[]; //存放查询数据
};

struct NET_STREAM_INFO {
    int stream_type; //0,前箱, 1,左前箱，2，右前箱 3,B  4 左后箱  5，右后箱
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
    int nRegatherType; //1,手工录入  2，启动前端设备方式
    char channel_no[32];
    char device_tag[16];
    T_Upload_Customs_Data customs_data; //手工方式有效，其他无效

};

struct CLIENT_COMMAND_INFO//控制类报文
{
    int iArrange_Type; //处理类别：0-放行报文；1-人工处理;2-设备控制
    char szChnnl_No[32]; // 场站号+通道号+进出标志（0为进、1为出）
    char szDEAL_TYPE[4]; //人工处理类控制类型（可以为空）00：抬杆01：落杆02：启动ＩＣ卡读写	03：启动地磅04：启动电子车牌05: 启动车间器
    char szMEM[256]; //备注说明（备案到数据库中供后期查询）
    int nCustomsDataLen; //报文XML大小
    char szCustomsData[]; //存放XML采集报文和放行报文	
};

struct CLIENT_EXCEPTION_FREE//异常放行控制报文
{
	int iArrange_Type; //处理类别：3-异常放行控制
	char szChnnl_No[32]; // 场站号+通道号+进出标志（I为进、E为出）
	char szSequence_No[32]; // 场站号+通道号+进出标志（0为进、1为出）
	char szDEAL_TYPE[4]; //异常放行标记，值为00
	char szMEM[256]; //备注说明（备案到数据库中供后期查询）
	int nExceptionFreeDataLen; //异常放行报文XML大小
	char szExceptionFreeData[]; //存放异常放行XML控制报文，其中存放control_info	
};

#endif
