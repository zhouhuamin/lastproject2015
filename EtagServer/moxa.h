#pragma once



typedef struct
{
	int baudrate;
	int data_bits;
	int stop_bits;
	char parity;
} comm_setting_t;


int comm_set_tcp_server_mode(int port);
int comm_settings_read(int port, comm_setting_t *p_setting);
int comm_setting_write(int port, comm_setting_t *p_setting);

int moxa_reset(void);
