#include "includes.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msg.h"
#include <unistd.h>
#include <sys/stat.h>

static int msg_RFID;

void msg_queue_init(void)
{
	key_t key;

	system("touch "RFID_NEW_ID_QUEUE_TMP_FILE);
	key = ftok(RFID_NEW_ID_QUEUE_TMP_FILE, 'a');

	msg_RFID = msgget(key, IPC_CREAT | 0666);
	if (msg_RFID < 0)
	{
		LOG("mag play record id < 0!\n");
		exit(0);
	}
}

int RFID_msg_send(u8 cmd, u32 src_id, u8 *id_str, int id_len)
{
	msg_event_t msg;
	int ret;
	int i;

	msg.msg_type = MSG_FROM_RFID_PTHREAD;

	msg.msg.cmd = cmd;
	msg.msg.src_id = src_id;
	msg.msg.parame = 0;

	if (id_str == NULL)
	{
		return -1;
	}

	msg.msg.f_name[0] = 0;
	for (i = 0; i < id_len; i++)
	{
		sprintf(msg.msg.f_name + strlen(msg.msg.f_name), "%02X", id_str[i]);
	}

	ret = msgsnd(msg_RFID, &msg, sizeof(msg.data), IPC_NOWAIT);
	if (ret < 0)
	{
		printf("send msg error\n");
	}
	return 0;
}

int msg_recv_RFID(char *id_str)
{
	msg_event_t msg;
	int ret;

	ret = msgrcv(msg_RFID, &msg, sizeof(msg.data), MSG_FROM_RFID_PTHREAD,
	IPC_NOWAIT);

	if (ret > 0)
	{
		strcpy(id_str,msg.msg.f_name);
		return 1;
	}
	return 0;
}
