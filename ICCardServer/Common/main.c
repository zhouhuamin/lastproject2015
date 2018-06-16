#include "includes.h"
#include "packet_format.h"
#include "pthread.h"
#include "starup_handle.h"
#include "port.h"

pthread_t tid_DEV;

volatile u32 DEV_request_count = 0;
int gFd_dev = -1;


int main(void)
{
	int server_fd;

	param_init_handle();
	dev_open_handle(&gFd_dev);
	dev_connect_handle(gFd_dev);
	server_connect_handle(&server_fd);
	server_register_handle(server_fd);

	pthread_create(&tid_DEV, NULL, pthread_dev_handle, NULL);

	log_send("LOG:\tcreate the DEV pthread OK!\n");

	net_handle(server_fd);

	close(gFd_dev);

	return 0;
}

