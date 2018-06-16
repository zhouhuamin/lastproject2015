#include "includes.h"
#include "port.h"

int param_init_handle(void)
{
	int ret;

	log_send("parame init...");

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
	log_send("LOG:\tparam init OK!\n");

	return 0;
}

int dev_open_handle(int *p_fd_com)
{
	int ret;
	log_send("LOG:\topen the reader...\n");

	//连接读卡�?	
	while (1)
	{
		ret = dev_open(p_fd_com);
		if (ret < 0)
		{
			printf("open reader port error! %d\n", ret);
		}
		else
		{
			gReal_system.dev_open_flag = 1;
			break;
		}
		sleep(2);
	}
	log_send("LOG:\topen the reader OK!\n");
	return 0;
}
int dev_connect_handle(int fd_dev)
{
	int ret;

	log_send("LOG:\tconnect the dev...\n");

	while (1) //
	{
		if (gReal_system.dev_open_flag)
		{
			ret = dev_connect(fd_dev);
			if (ret != 0)
			{
				printf("connect read error! %d\n", ret);
			}
			else
			{
				gReal_system.dev_connect_flag = 1;
				break;
			}
		}

		sleep(2);
	}
	log_send("LOG:\tconnect the dev OK!\n");
	return 0;
}

int server_connect_handle(int *pFd_ser)
{
	log_send("LOG:\tconnect the server...\n");
	//连接服务�?	while (1)
	{
		*pFd_ser = connect_server(gSetting_system.server_ip,
				gSetting_system.server_port);
		if (*pFd_ser <= 0)
		{
			printf("connect to server error!\n");
		}
		else
		{
			gReal_system.server_connect_flag = 1;
			break;
		}
		sleep(2);
	}
	log_send("LOG:\tconnect the server OK!\n");
	return 0;
}
int server_register_handle(int server_fd)
{
	int ret;

	log_send("LOG:\tregister the device...\n");

	while (1)
	{
		if (gReal_system.server_connect_flag)
		{
			ret = net_dev_register_handle(server_fd);
			if (ret < 0)
			{
				LOG("register dev error!\n");
			}
			else
			{
				gReal_system.server_register_flag = 1;
				break;
			}
		}

		sleep(2);
	}
	log_send("LOG:\tregister the device OK\n");
	return 0;
}

int dev_retry_connect_handle(int *pFd_dev)
{
	int fd_tmp;
	int ret;
	log_send("ERR:\tthe reader's connection lost \n");

	//连接读卡�?	while (1)
	{
		ret = dev_open(&fd_tmp);
		if (ret >= 0)
		{
			gReal_system.dev_open_flag = 1;
			close(*pFd_dev);
			*pFd_dev = fd_tmp;
			log_send("MSG:\tconnect the reader with new fd!\n");
			break;
		}
		else
		{
			ret = dev_connect(*pFd_dev);
			if (ret == 0)
			{
				gReal_system.dev_connect_flag = 1;
				log_send("MSG:\tconnect the reader with old fd!\n");
				break;
			}
		}
		sleep(1);
	}
	return 0;
}

int server_retry_connect_handle(int *pFd_ser)
{
	int ret;

	close(*pFd_ser);
	log_send("ERR:\tthe server's connection is lost!\n");
	log_send("MSG:\tRetry to connect to the server...\n");

	//连接服务�?	
	while (1)
	{
		*pFd_ser = connect_server(gSetting_system.server_ip,
				gSetting_system.server_port);
		if (*pFd_ser <= 0)
		{
			printf("connect to server error!\n");
		}
		else
		{
			log_send("MSG:\tconnected to the server!\n");
			ret = net_dev_register_handle(*pFd_ser);
			if (0 == ret)
			{
				log_send("MSG:\tregister to the server!\n");
				break;
			}
			else
			{
				close(*pFd_ser);
			}
		}
		sleep(2);
	}
	return 0;
}

