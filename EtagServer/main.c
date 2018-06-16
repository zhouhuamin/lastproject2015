#include "includes.h"
#include <stdio.h>
#include "reader1000api.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <errno.h>
#include "packet_format.h"
#include "moxa.h"
#include "pthread.h"
#include "config.h"
#include "common.h"
#include "msg.h"

u16 hv, sv;
u8 EPCID[600];
pthread_t tid_RFID;

RFID_UNIT_t RFID_UNIT =
{ 0, 0 };

volatile u32 RFID_request_count = 0;

int connect_server(const char *host_name, int port_id);
void *pthread_RFID_handle(void *arg);
int fd_com;

int main(void)
{
	int ret;
	int server_fd;

	LOG("parame init...\n");
	log_write("LOG:\tparam init...\n");
	do
	{
		ret = parame_init();
		if (ret < 0)
		{
			printf("parame init error!\n");
		}
		else
		{
			break;
		}
		sleep(1);
	} while (ret < 0);
	log_add("LOG:\tparam init OK!\n", 1);

	log_add("LOG:\topen the reader...\n", 1);

	//连接读卡器
	while (1)
	{
		ret = ConnectScanner(&fd_com, "tty0", 57600);
		if (ret < 0)
		{
			printf("open reader port error! %d\n", ret);
		}
		else
		{
			break;
		}
		sleep(2);
	}
	log_add("LOG:\topen the reader OK!\n", 1);

	log_add("LOG:\tconnect the reader...\n", 1);
	while (0 == gReal_system.TK900_connect_flag)
	{
		ret = GetReaderVersion(fd_com, &hv, &sv, 0);
		if (ret != 0)
		{
			printf("connect read error! %d\n", ret);
		}
		else
		{
			gReal_system.TK900_connect_flag = 1;
			break;
		}

		sleep(2);
	}
	log_add("LOG:\tconnect the reader OK!\n", 1);

	log_add("LOG:\tconnect the server...\n", 1);
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
			break;
		}
		sleep(2);
	}
	log_add("LOG:\tconnect the server OK!\n", 1);

	log_add("LOG:\tregister the device...\n", 1);
	while (1)
	{
		net_dev_register_handle(server_fd);
		if (1 == gReal_system.dev_register_flag)
		{
			break;
		}
		sleep(2);
	}
	log_add("LOG:\tregister the device OK\n", 1);

	pthread_create(&tid_RFID, NULL, pthread_RFID_handle, NULL);
	log_add("LOG:\tcreate the RFID pthread OK!\n", 1);

	net_handle(server_fd);

	close(fd_com);

	return 0;
}

void *pthread_RFID_handle(void *arg)
{
	int ret;
	int n_count;
	int r_len;
	u8 EPCID[200];
	int i;
	u8 *p_start;
	int fd_tmp;
	int retry_cnt = 0;

	while (1)
	{
		if (RFID_request_count != RFID_UNIT.RFID_scan_count)
		{
			n_count = 0;

			if (gSetting_system.RFID_label_type)
			{
				ret = ISO6B_ListID(fd_com, EPCID, &n_count, 0);
			}
			else
			{
				ret = EPC1G2_ListTagID(fd_com, 1, 0, 0, NULL, EPCID, &n_count,
						&r_len, 0);
			}

			if (0 == ret)
			{
				if (n_count > 0)
				{
					p_start = EPCID;
					LOG("read %d label!\n", n_count);
					for (i = 0; i < n_count; i++)
					{
						if (gSetting_system.RFID_label_type)
						{
							RFID_msg_send(0, 0, &p_start[0+8*i], 8);
						}
						else
						{
							RFID_msg_send(0, 0, &p_start[1], p_start[0] * 2);
							p_start += (p_start[0] * 2 + 1);
						}
					}

					if (0 == RFID_UNIT.RFID_label_count) // first scan label
					{
						RFID_UNIT.RFID_scan_ok_start_systick = get_system_ms();
					}
					RFID_UNIT.RFID_label_count += n_count;
				}
			}
			else if (-99 == ret)
			{
				log_add("ERR:\tthe reader's connection lost \n", 1);
				retry_cnt = 0;
				//连接读卡器
				while (1)
				{
					ret = ConnectScanner(&fd_tmp, "tty0", 57600);
					if (ret >= 0)
					{
						gReal_system.TK900_connect_flag = 1;
						close(fd_com);
						fd_com = fd_tmp;
						log_add("MSG:\tconnect the reader with new fd!\n", 1);
						break;
					}
					else
					{
						ret = GetReaderVersion(fd_com, &hv, &sv, 0);
						if (ret == 0)
						{
							gReal_system.TK900_connect_flag = 1;
							log_add("MSG:\tconnect the reader with old fd!\n",
									1);
							break;
						}
						else
						{
							retry_cnt++;

							if (retry_cnt >= 2)
							{
								moxa_reset();
								log_add("MSG:\treset the moxa!\n", 1);
							}
						}
					}

					sleep(1);
				}
			}
			else if (2 == ret)  //no label
			{

			}
			else  //协议报文交互出了问题
			{
				LOG("the reader no respond! ret = %d\n", ret);
			}

			if (RFID_UNIT.RFID_label_count != 0)
			{
				if ((get_system_ms() - RFID_UNIT.RFID_scan_ok_start_systick)
						>= gSetting_system.RFID_scan_duration_time) //time out
				{
					RFID_UNIT.RFID_scan_count = RFID_request_count;
					RFID_UNIT.RFID_label_count = 0;
				}
			}
		}

		//
		if (gSetting_system.RFID_scan_gap < 60)
		{
			usleep(60 * 1000);
		}
		else
		{
			usleep(gSetting_system.RFID_scan_gap * 1000);
		}
	}

	return 0;
}

