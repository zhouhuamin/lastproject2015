#pragma once

#define 	MSG_FROM_RFID_PTHREAD			0x48
#define		RFID_NEW_ID_QUEUE_TMP_FILE		"/tmp/848"

typedef enum
{
	msg_RFID_NEW_ID = 0,
}emsg_Player;

typedef struct
{
    u32 cmd;
    u32 src_id;
    u32 parame;
    char f_name[128];
} msg_event_i;

typedef struct
{
    int   msg_type;
    union
    {
        msg_event_i msg;
        char  data[150];
    };
} msg_event_t;


void msg_queue_init(void);
int RFID_msg_send(u8 cmd, u32 src_id, u8 *id_str, int id_len);
int msg_recv_RFID(char *id_str);
