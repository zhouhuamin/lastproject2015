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
    int reg_result; //注册结果，成功还是失败
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
//颜色

enum Color {
    red, blue, white, gray, green, other
};


//集装箱号排列方式

enum AlignType {
    H, T
}; //H:horizontal 横排  T：tandem 竖排

struct Align {
    AlignType Atype; //排列方式 横排或者竖排
    int count; //排数
};

struct ContaID {
    char ID[12]; //集装箱号：4位字母，6位数字，1位校验码
    char Type[5]; //集装箱类型
    Region reg; //区域
    Color color; //颜色
    Align ali; //排列方式
};

struct T_ContaRecoResult {
    char req_sequence[32];
    int result;
    ContaID conta_id;
};


struct NET_KEEP_LIVE {
    int client_type;
};


#endif
