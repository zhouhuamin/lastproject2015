#include "includes.h"
#include "packet_format.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "config.h"
#include <errno.h>
#include "reader1000api.h"
#include "msg.h"
#include "common.h"

NVP_REGISTER reg_unit;
net_packet_head_t packet_head;
static net_packet_t net_packet;
static u8 data_buf[0x800];

int server_read_timeout(int server_fd, u8 *buf, int len, int timeout);
int get_one_packet(int net_fd, u8* buf);
static int net_system_CTRL_handle(u8 *buf, int len);

NET_SYS_CTRL sys_ctrl;
extern volatile u32 RFID_request_count;
extern RFID_UNIT_t RFID_UNIT;

int connect_server(const char *host_name, int port_id)
{
	struct sockaddr_in server_addr;
	struct hostent *host;
	int sock;
	int flag;

	host = gethostbyname(host_name);
	if ( NULL == host)
	{
		LOG("get hostname error!\n");
		return -1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sock)
	{
		LOG("socket error:%s\a\n!\n", strerror(errno));
		return -2;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_id);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

	if (connect(sock, (struct sockaddr *) (&server_addr),
			sizeof(struct sockaddr_in)) == -1)
	{
		LOG_M("connect error:%s\n", strerror(errno));
		return -3;
	}

	flag = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flag | O_NONBLOCK);

	return sock;
}

int net_handle(int server_fd)
{
	volatile int net_disconnected = 0;
	int ret;

	while (1)
	{
		ret = get_one_packet(server_fd, data_buf);

		if (ret > 0) //get data
		{
			memcpy((void *) &net_packet, data_buf, ret);

			switch (net_packet.msg_head.msg_type)
			{
			case MSG_TYPE_CTRL_EVENT:
				net_system_CTRL_handle(net_packet.msg_body,
						net_packet.msg_head.packet_len);
				break;
			default:
				printf("invalid request! len=%d\n", ret);
				break;
			}
		}
		else if (-99 == ret) 	//server close
		{
			LOG("the server is closed!");
			net_disconnected = 1;
		}
		else
		{

		}

		upload_gather_data_handle(server_fd);

		ret = net_dev_keeplive_handle(server_fd);
		if (ret < 0)
		{
			net_disconnected = 1;
		}

		if (0 != net_disconnected)
		{
			net_disconnected = 0;
			close(server_fd);
			log_add("ERR:\tthe server's connection is lost!\n", 1);
			log_add("MSG:\tRetry to connect to the server...\n", 1);

			//连接服务器
			while (1)
			{
				server_fd = connect_server(gSetting_system.server_ip,
						gSetting_system.server_port);
				if (server_fd <= 0)
				{
					printf("connect to server error!\n");
				}
				else
				{
					log_add("MSG:\tconnected to the server!\n", 1);
					ret = net_dev_register_handle(server_fd);
					if (0 == ret)
					{
						log_add("MSG:\tregister to the server!\n", 1);
						break;
					}
					else
					{
						close(server_fd);
					}
				}
				sleep(2);
			}
		}
	}

	close(server_fd);
}

//dev login server
int net_dev_register_handle(int server_fd)
{
	NVP_REGISTER *p_reg;
	NVP_REGISTER_ACK *p_reg_ack;
	char str_tmp[16];
	int w_len, r_len;
	int ret;

	packet_head.msg_type = MSG_TYPE_DEV_REGISTER;
	packet_head.packet_len = sizeof(NVP_REGISTER);
	strcpy(packet_head.szVersion, "1.0");

	strcpy((char *) data_buf, "0XJZTECH");
	memcpy(data_buf + 8, &packet_head, sizeof(net_packet_head_t));
	p_reg = (NVP_REGISTER *) (data_buf + 8 + sizeof(net_packet_head_t));

	strcpy(p_reg->dev_tag, gSetting_system.register_DEV_TAG);
	strcpy(p_reg->dev_id, gSetting_system.register_DEV_ID);

	w_len = sizeof(net_packet_head_t) + sizeof(NVP_REGISTER) + 8;
	strcpy((char *) (data_buf + w_len), "NJTECHJZ");
	w_len += 8;

	tcflush(server_fd, TCIFLUSH);
	ret = write(server_fd, data_buf, w_len);
	if (ret != w_len)
	{
		LOG_M("write data error! %d\n", ret);
		return -1;
	}

	r_len = 8 + sizeof(net_packet_head_t) + sizeof(NVP_REGISTER_ACK) + 8;
	ret = server_read_timeout(server_fd, data_buf, r_len, 500);
	if (ret != r_len)
	{
		LOG("get register ack r_len is error! %d\n", ret);
		return -2;
	}

	memcpy(str_tmp, data_buf, 8);
	str_tmp[8] = 0;
	if (strcmp(str_tmp, NET_PACKET_HEAD_STRING))
	{
		LOG("head string check error!\n");
		return -3;
	}

	memcpy(str_tmp, data_buf + r_len - 8, 8);
	str_tmp[8] = 0;
	if (strcmp(str_tmp, NET_PACKET_TAIL_STRING))
	{
		LOG("tail string check error!\n");
		return -4;
	}

	memcpy(&packet_head, data_buf + 8, sizeof(net_packet_head_t));
	if ((packet_head.msg_type != MSG_TYPE_DEV_REGISTER_ACK)
			|| (packet_head.packet_len != sizeof(NVP_REGISTER_ACK)))
	{
		LOG_M("packer_len =%d\n", packet_head.packet_len);
		return -5;
	}

	p_reg_ack = (NVP_REGISTER_ACK*) (data_buf + sizeof(net_packet_head_t) + 8);
	if (0 == p_reg_ack->reg_result)
	{
		gReal_system.dev_register_flag = 1;
		gReal_system.work_flag = p_reg_ack->next_action;
	}
	else
	{
		LOG("dev register error!\n");
	}

	LOG_M("result: %d, action: %d", p_reg_ack->reg_result,
			p_reg_ack->next_action);

	return 0;
}

int net_dev_keeplive_handle(int server_fd)
{
	static u32 keeplive_trig_systick = 0;
	int w_len, r_len;
	int ret;
	char str_tmp[16];

	if ((get_system_ms() - keeplive_trig_systick)
			< gSetting_system.keeplive_gap)
	{
		return 0;
	}

	keeplive_trig_systick = get_system_ms();

	packet_head.msg_type = MSG_TYPE_KEEP_LIVE;
	packet_head.packet_len = 0;
	strcpy(packet_head.szVersion, "1.0");

	strcpy((char *) data_buf, NET_PACKET_HEAD_STRING);
	memcpy(data_buf + NET_PACKET_HEAD_STRING_LEN, &packet_head,
			sizeof(net_packet_head_t));

	w_len = sizeof(net_packet_head_t) + NET_PACKET_HEAD_STRING_LEN;

	strcpy((char*) (data_buf + w_len), NET_PACKET_TAIL_STRING);
	w_len += NET_PACKET_TAIL_STRING_LEN;

	tcflush(server_fd, TCIFLUSH);
	ret = write(server_fd, data_buf, w_len);
	if (ret != w_len)
	{
		LOG_M("write data error! %d\n", ret);
	}

	r_len = sizeof(net_packet_head_t) + 8 + 8;
	ret = server_read_timeout(server_fd, data_buf, r_len, 1000);
	if (ret != r_len)
	{
		LOG_M("read data error! ret =%d\n", ret);
		return -1;
	}

	memcpy(str_tmp, data_buf, 8);
	str_tmp[8] = 0;
	if (0 == strcmp(str_tmp, NET_PACKET_HEAD_STRING))
	{
		memcpy(&packet_head, data_buf + 8, sizeof(net_packet_head_t));
		if (packet_head.msg_type != MSG_TYPE_KEEP_LIVE_ACK
//				|| packet_head.packet_len != 0
				)
		{
			LOG_M("packet_len =%d\n", packet_head.packet_len);
			return -2;
		}
	}
	else
	{
		LOG("net packet head check error!\n");
	}

	LOG("keep live trigger!\n");

	return 0;
}

int server_read_timeout(int server_fd, u8 *buf, int len, int time_out)
{
	int ret;
	int r_len = 0;

	while (1)
	{
		ret = read(server_fd, buf + r_len, len - r_len);

		if (ret > 0)
		{
			r_len += ret;
			break;
		}
		else if (ret == 0)  //server close
		{
			return -99;
		}

		usleep(1000);
		time_out--;
		if (time_out <= 0)
		{
			break;
		}
	}

	return r_len;
}

int upload_gather_data_handle(int server_fd)
{
	char id_str[100] =
	{ 0 };
	int ret = 0;

	do
	{
		ret = msg_recv_RFID(id_str);
		if (ret > 0)
		{
//			if (gReal_system.work_flag)
			{
				xml_upload_gather_data(server_fd, id_str);
			}
		}
	} while (ret > 0);

	return 0;
}

int xml_upload_gather_data(int server_fd, char *ic_number)
{
	NET_gather_data *p_gather_data;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int w_len;
	xmlChar * xml_buf;
	char *p_send;
	int xml_len;
	int ret;

	packet_head.msg_type = MSG_TYPE_GATHER_DATA;
	strcpy(packet_head.szVersion, "1.0");

	p_gather_data = (NET_gather_data *) (data_buf + sizeof(net_packet_head_t)
			+ 8);

	strcpy(p_gather_data->event_id, gSetting_system.gather_event_id);
	ret = 0;
	memcpy(&(p_gather_data->is_data_full), &ret, sizeof(int));
	strcpy(p_gather_data->dev_tag, gSetting_system.gather_DEV_TAG);

	/**********************************XML*********************************/
	doc = xmlNewDoc(NULL);

	root_node = xmlNewNode(NULL, BAD_CAST "CAR");

	xmlDocSetRootElement(doc, root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "VE_NAME", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO",
	BAD_CAST (ic_number));

	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO2", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "VE_CUSTOMS_NO", BAD_CAST "");

	xmlNewChild(root_node, NULL, BAD_CAST "VE_WT", BAD_CAST "");

	xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

	p_send = strstr((char*) xml_buf, "<CAR>");

	if (p_send == NULL)
	{
		return -1;
	}

	xml_len -= (p_send - (char*) xml_buf);

	p_gather_data->xml_data_len = xml_len + 1;
	memcpy(p_gather_data->xml_data, p_send, xml_len);
	p_gather_data->xml_data[xml_len] = 0;

	xmlFree(xml_buf);
	xmlFreeDoc(doc);

	xmlCleanupParser();

	xmlMemoryDump();
	/**********************************XML END*********************************/

	w_len = ((u8*) p_gather_data->xml_data - data_buf) + xml_len + 1;
	packet_head.packet_len = w_len - sizeof(net_packet_head_t) - 8;
	strcpy((char *) data_buf, "0XJZTECH");

	memcpy(data_buf + 8, &packet_head, sizeof(net_packet_head_t));
	strcpy((char*) (data_buf + w_len), "NJTECHJZ");
	w_len += 8;

//	LOG("upload gather data w_len = %d\n", w_len);
	ret = write(server_fd, data_buf, w_len);
	if (ret != w_len)
	{
		LOG_M("upload gather data error! ret =%d\n", ret);
	}

	log_add("RE:\t", 1);
	log_add(ic_number, 0);
	log_add("\n", 0);

	return 0;
}

static int net_system_CTRL_handle(u8 *buf, int len)
{
	NET_SYS_CTRL *p_ctrl;

	p_ctrl = (NET_SYS_CTRL*) buf;

	if (!strcmp(p_ctrl->event_id, gSetting_system.SYS_CTRL_R_start_event_ID))
	{
//		LOG("get a start read request!\n");
		log_add("ASK:\tStart read!\n", 1);
		RFID_request_count++;
	}
	else if (!strcmp(p_ctrl->event_id,
			gSetting_system.SYS_CTRL_R_stop_event_ID))
	{
//		LOG("get a stop read request!\n");
		log_add("ASK:\tStop read!\n", 1);
		RFID_request_count = RFID_UNIT.RFID_scan_count;
	}
	else
	{
		LOG("system ctrl id= %s\n", p_ctrl->event_id);
	}

	return 0;
}

//get a packet
//return value: 0:no data; >0: get packet -1:server close -2:error
int get_one_packet(int net_fd, u8* buf)
{
	int ret, r_len = 0;
	net_packet_head_t msg_head;
	char str_tmp[16];

	ret = server_read_timeout(net_fd, buf, sizeof(net_packet_head_t) + 8, 10);
	if (0 >= ret)
	{
		return ret;
	}
	else if (ret != (sizeof(net_packet_head_t) + 8))
	{
		printf("get str head len error!	%s\n", str_tmp);
		return -2;
	}

	memcpy(str_tmp, buf, 8);
	str_tmp[8] = 0;
	if (strcmp((char*) str_tmp, "0XJZTECH"))
	{
		printf("str head check error!\n");
		return -2;
	}
	memcpy(&msg_head, buf + 8, sizeof(net_packet_head_t));
	r_len = sizeof(net_packet_head_t) + 8;

	if (msg_head.packet_len > 0)
	{
		ret = server_read_timeout(net_fd, buf + r_len, msg_head.packet_len, 10);
		if (0 >= ret)
		{
			return ret;
		}
		else if (ret != msg_head.packet_len)
		{
			printf("get msg body len error!\n");
			return -2;
		}
		r_len += msg_head.packet_len;
	}

	ret = server_read_timeout(net_fd, buf + r_len, 8, 10);
	if (0 >= ret)
	{
		return ret;
	}
	else if (ret != 8)
	{
		printf("get str tail len error!\n");
		return -2;
	}

	memcpy(str_tmp, buf + r_len, 8);
	str_tmp[8] = 0;
	if (strcmp(str_tmp, "NJTECHJZ"))
	{
		printf("str tail check error!\n");
		return -2;
	}

	r_len += 8;

	return r_len;
}
