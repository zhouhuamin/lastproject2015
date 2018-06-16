#include "includes.h"
#include "moxa.h"
#include "reader1000api.h"
#include "errno.h"
#include "sys/select.h"

#define TELNET_QNA(S,F,L) ret = telnet_QNA(telnet_fd, (u8*)(S), data_buf, sizeof(data_buf));\
if ((ret <= 0) || (NULL == u8_str(data_buf, F, ret)))\
	{\
		LOG(L); \
		telnet_close(telnet_fd);\
return -1;\
}\

static u8 data_buf[0x800];
//static char sub_str[200];

static int telnet_open(void);
static int telnet_close(int telnet_fd);
static int telnet_read(int telnet_fd, u8 *buf, int max_len, int timeout);
static int telnet_write(int telnet_fd, u8 *buf);
static int telnet_QNA(int telnet_fd, u8 *w_buf, u8 * r_buf, int max_r_len);
static char *u8_chr(u8 *buf, char chr, int buf_len);
static char *u8_str(u8 *buf, char * sub, int buf_len);
static char get_baudrate_index(int baudrate);
static int get_data_bits_index(int data_bits);
static int get_stop_bits_index(char stop_bits);
static int get_parity_index(char parity);

int comm_settings_read(int port, comm_setting_t *p_setting)
{
	int telnet_fd;
	int ret = 0;
	char w_buf[10];
	char *p_sub;

	if (port <= 0 || port > 4)
	{
		LOG_M("PORT ID error port = %d!\n", port);
		return -1;
	}

	telnet_fd = telnet_open();
	if (telnet_fd < 0)
	{
		LOG("telnet open error! %d\n", telnet_fd);
		return -1;
	}

	/*------------------------------------get first packet-------------------------------------------*/
	ret = telnet_read(telnet_fd, data_buf, sizeof(data_buf), 100);
	if ((ret <= 0) || (NULL == u8_str(data_buf, "<< Main Menu >>", ret)))
	{
		LOG("telnet first packet read error! %d\n", ret);
		telnet_close(telnet_fd);
		return -2;
	}

	TELNET_QNA("3", "<< Main Menu->Serial settings >>",
			"select serial settings error!");

	sprintf(w_buf, "%d", port);
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"select serial port error!\n");

	TELNET_QNA("2", "Baud rate", "select baud rate error!\n");
	p_sub = u8_chr(data_buf, '(', ret) + 1;
	sscanf(p_sub, "%d", &(p_setting->baudrate));
//	LOG_M("baud rate = %d", p_setting->baudrate);

	TELNET_QNA("q", "<< Main Menu->Serial settings->Port", "back prev error!\n");

	TELNET_QNA("3", "Data bits", "select Data bits error!\n");
	p_sub = u8_chr(data_buf, '(', ret) + 1;
	sscanf(p_sub, "%d", &(p_setting->data_bits));
//	LOG_M("data bits = %d", p_setting->data_bits);

	TELNET_QNA("q", "<< Main Menu->Serial settings->Port", "back prev error!\n");

	TELNET_QNA("4", "Stop bits", "select Stop bits error!\n");
	p_sub = u8_chr(data_buf, '(', ret) + 1;
	sscanf(p_sub, "%d", &(p_setting->stop_bits));
//	LOG_M("Stop bits = %d", p_setting->stop_bits);

	TELNET_QNA("q", "<< Main Menu->Serial settings->Port", "back prev error!\n");

	TELNET_QNA("5", "Parity", "select Parity error!\n");
	p_sub = u8_chr(data_buf, '(', ret) + 1;
	sscanf(p_sub, "%c", &(p_setting->parity));
//	LOG_M("Parity = %c", p_setting->parity);

	telnet_close(telnet_fd);
	return 0;
}

int comm_setting_write(int port, comm_setting_t *p_setting)
{
	int telnet_fd;
	int ret = 0;
	char w_buf[10];

	if (port <= 0 || port > 4)
	{
		LOG_M("PORT ID error port = %d!\n", port);
		return -1;
	}

	telnet_fd = telnet_open();
	if (telnet_fd < 0)
	{
		LOG("telnet open error!\n");
		return -1;
	}

	/*------------------------------------get first packet-------------------------------------------*/
	ret = telnet_read(telnet_fd, data_buf, sizeof(data_buf), 100);
	if ((ret <= 0) || (NULL == u8_str(data_buf, "<< Main Menu >>", ret)))
	{
		LOG("telnet first packet read error! %d\n", ret);
		telnet_close(telnet_fd);
		return -2;
	}

	TELNET_QNA("3", "<< Main Menu->Serial settings >>",
			"select serial settings error!");

	sprintf(w_buf, "%d", port);
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"select serial port error!\n");

	TELNET_QNA("2", "Baud rate", "select baud rate error!\n");
	sprintf(w_buf, "%c", get_baudrate_index(p_setting->baudrate));
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"set baud rate error!\n");

	TELNET_QNA("3", "Data bits", "select Data bits error!\n");
	sprintf(w_buf, "%d", get_data_bits_index(p_setting->data_bits));
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"set data bits error!\n");

	TELNET_QNA("4", "Stop bits", "select Stop bits error!\n");
	sprintf(w_buf, "%d", get_stop_bits_index(p_setting->stop_bits));
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"set stop bits error!\n");

	TELNET_QNA("5", "Parity", "select Parity error!\n");
	sprintf(w_buf, "%d", get_parity_index(p_setting->parity));
	TELNET_QNA(w_buf, "<< Main Menu->Serial settings->Port",
			"set parity error!\n");

	TELNET_QNA("m", "<< Main Menu >>", "back Main Menu error!\n");
	TELNET_QNA("s", "Save change?", "Save change error!\n");
//	TELNET_QNA("y", "Save change?", "Save change verify error!\n");
	ret = telnet_QNA(telnet_fd, (u8*) ("y"), data_buf, sizeof(data_buf));
	if (ret == 0)
	{
		LOG("telnet is disconnect!\n");
//		telnet_close(telnet_fd);
		return 0;
	}
	else if (ret < 0)
	{
		telnet_close(telnet_fd);
		return -1;
	}

	telnet_close(telnet_fd);

	return 0;
}

int comm_set_tcp_server_mode(int port)
{
	int telnet_fd;
	int ret = 0;
	char w_buf[10];
	char mode_str[100];
//	char *p_sub;

	if (port <= 0 || port > 4)
	{
		LOG_M("PORT ID error port = %d!\n", port);
		return -1;
	}

	telnet_fd = telnet_open();
	if (telnet_fd < 0)
	{
		LOG("telnet open error!\n");
		return -1;
	}

	/*------------------------------------get first packet-------------------------------------------*/
	ret = telnet_read(telnet_fd, data_buf, sizeof(data_buf), 100);
	if ((ret <= 0) || (NULL == u8_str(data_buf, "<< Main Menu >>", ret)))
	{
		LOG("telnet first packet read error! %d\n", ret);
		telnet_close(telnet_fd);
		return -2;
	}

	TELNET_QNA("4", "<< Main Menu->Operating settings >>",
			"select serial mode error!");

	sprintf(w_buf, "%d", port);
	TELNET_QNA(w_buf, "<< Main Menu->Operating settings->Port",
			"select serial port error!\n");

	TELNET_QNA("1", "Operating mode", "select operating mode error!\n");
//	p_sub = u8_chr(data_buf, '(', ret) + 1;
//	sscanf((char*)data_buf, "%s", mode_str);
	if ( NULL == u8_str(data_buf, "(TCP Server Mode)", ret))
	{
		LOG_M("cur mode is no tcp server ->%s!\n", mode_str);
		TELNET_QNA("1", "<< Main Menu->Serial settings->Port",
				"set oper mode error!\n");
	}
	else
	{
		LOG("mode is checked!\n");
	}

	telnet_close(telnet_fd);

	return 0;
}

int moxa_reset(void)
{
	int telnet_fd;
	int ret = 0;
	char w_buf[10];

	telnet_fd = telnet_open();
	if (telnet_fd < 0)
	{
		LOG("telnet open error!\n");
		return -1;
	}

	/*------------------------------------get first packet-------------------------------------------*/
	ret = telnet_read(telnet_fd, data_buf, sizeof(data_buf), 100);
	if ((ret <= 0) || (NULL == u8_str(data_buf, "<< Main Menu >>", ret)))
	{
		LOG("telnet first packet read error! %d\n", ret);
		telnet_close(telnet_fd);
		return -2;
	}

	TELNET_QNA("s", "Ready to restart", "select system reset error!");

	ret = telnet_QNA(telnet_fd, (u8*) ("y"), data_buf, sizeof(data_buf));
	if (ret == 0)
	{
		LOG("telnet is disconnect!\n");
		return 0;
	}
	else if (ret < 0)
	{
		telnet_close(telnet_fd);
		return -1;
	}

	telnet_close(telnet_fd);

	return 0;
}

static int telnet_open(void)
{
	int telnet_fd;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int flag;
	struct timeval tm =
	{ 2, 0 };
//	fd_set set;
//	int error = -1;
	int len = sizeof(tm);

	host = gethostbyname(NET_COMM_IP_ADDRESS);
	if ( NULL == host)
	{
		LOG("telnet get hostname error!\n");
		return -1;
	}

	telnet_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == telnet_fd)
	{
		LOG("telnet socket error:%s\a\n!\n", strerror(errno));
		return -2;
	}

	//设置非阻塞
	setsockopt(telnet_fd, SOL_SOCKET, SO_SNDTIMEO, &tm, len);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(23);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

	if (connect(telnet_fd, (struct sockaddr *) (&server_addr),
			sizeof(struct sockaddr_in)) == -1)
	{
		if (errno == EINPROGRESS)
		{
			close(telnet_fd);
			return -3;
		}
	}

	flag = fcntl(telnet_fd, F_GETFL, 0);
	fcntl(telnet_fd, F_SETFL, flag | O_NONBLOCK);

	return telnet_fd;
}

static int telnet_close(int telnet_fd)
{
	tcflush(telnet_fd, TCIOFLUSH);
	close(telnet_fd);
	return 0;
}

static int telnet_read(int telnet_fd, u8 *buf, int max_len, int time_out)
{
	int ret;
	int r_len = 0;

	while (1)
	{
		ret = read(telnet_fd, buf + r_len, max_len - r_len);

		if (ret > 0)
		{
			r_len += ret;
		}
		else if (0 == ret)
		{
			return 0;
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

static int telnet_write(int telnet_fd, u8 *buf)
{
	char w_buf[100];
	int len;

	if (0 == buf)
	{
		return -1;
	}
	strcpy(w_buf, (char*) buf);
	len = strlen(w_buf);

	w_buf[len++] = '\r';

	return write(telnet_fd, w_buf, len);;
}

static int telnet_QNA(int telnet_fd, u8 *w_buf, u8 * r_buf, int max_r_len)
{
	int ret = 0;
	int time_out = 15;

	tcflush(telnet_fd, TCIOFLUSH);
	ret = telnet_write(telnet_fd, w_buf);
	if (ret <= 0)
	{
		LOG("telnet write error!\n");
		return -1;
	}

	ret = telnet_read(telnet_fd, r_buf, max_r_len, time_out);
	return ret;
}

static char *u8_chr(u8 *buf, char chr, int buf_len)
{
	char *p_sub;
	char *p_start;
	int len = 0;

	p_sub = (char *) buf;
	while (1)
	{
		p_start = strchr(p_sub, chr);
		if ( NULL != p_start)  //get
		{
			return p_start;
		}
		len = strlen((char *) p_sub);
		{
			len++;
		}
		p_sub += len; //jump the net string
		if (((u8*) p_sub - buf) >= buf_len)
		{
			break;
		}
	}

	return NULL;
}

static char *u8_str(u8 *buf, char * sub, int buf_len)
{
	char *p_sub;
	char *p_start;
	int len = 0;

	p_sub = (char*) buf;
	while (1)
	{
		p_start = strstr(p_sub, sub);
		if ( NULL != p_start)  //get
		{
			return p_start;
		}

		len = strlen((char *) p_sub);
		if (len == 0)
		{
			len++;
		}
		p_sub += len; //jump the net string
		if (((u8*) p_sub - buf) >= buf_len)
		{
			break;
		}
	}

	return NULL;
}

static char get_baudrate_index(int baudrate)
{
	switch (baudrate)
	{
	case 300:
		return '5';
		break;
	case 600:
		return '6';
		break;
	case 1200:
		return '7';
		break;
	case 2400:
		return '9';
		break;
	case 4800:
		return 'a';
		break;
	case 9600:
		return 'c';
		break;
	case 19200:
		return 'd';
		break;
	case 38400:
		return 'e';
		break;
	case 57600:
		return 'f';
		break;
	case 115200:
		return 'g';
		break;
	case 230400:
		return 'h';
		break;
	case 460800:
		return 'i';
		break;
	case 921600:
		return 'j';
		break;
	default:
		return 'c';
		break;
	}
	return 'c';
}

static int get_data_bits_index(int data_bits)
{
	switch (data_bits)
	{
	case 5:
		return 0;
		break;
	case 6:
		return 1;
		break;
	case 7:
		return 2;
		break;
	case 8:
		return 3;
		break;

	default:
		break;
	}
	return '0';
}

static int get_stop_bits_index(char stop_bits)
{
	switch (stop_bits)
	{
	case 1:
		return 0;
		break;
	case 15:
		return 1;
		break;
	case 2:
		return 2;
		break;
	default:
		break;
	}
	return '0';
}

static int get_parity_index(char parity)
{
	switch (parity)
	{
	case 'N':
	case 'n':
		return 0;
		break;
	case 'O':
	case 'o':
		return 1;
		break;
	case 'E':
	case 'e':
		return 2;
		break;
	default:
		break;
	}
	return 0;
}
