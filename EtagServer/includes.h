#pragma once

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <stdarg.h>
#include 	<unistd.h>
#include 	<fcntl.h>
#include 	<linux/fb.h>
#include 	<sys/mman.h>
#include <termio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include "moxa.h"

//#include "common.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef char s8;
typedef short s16;
typedef int s32;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

typedef unsigned short int color_type;
typedef enum
{
	FALSE = 0, TRUE,
} BOOL;

#define SERVER_IP_ADDRESS_STR  ("127.0.0.1")
#define SERVER_NET_PORT 		(8888)

#define LOG_FILE_NAME       	"log"


#define REGISTER_DEV_TAG			("CAR")
#define REGISTER_DEV_ID				("11000000")
#define SYS_CTRL_R_START_EVENT_ID	("1100000000")
#define SYS_CTRL_R_STOP_EVENT_ID	("1100000009")

#define GATHER_DATA_EVENT_ID		("1100000002")
#define GATHER_DATA_DEV_TAG			("CAR")


#define _PIG_DEBUG_         1

#if _PIG_DEBUG_

#define LOG( ... ) printf(  __VA_ARGS__ )
#define LOG_M(format, ... ) printf( format"\n", __VA_ARGS__ )
#define DBG( ... ) printf(  __VA_ARGS__ )

#else

#define LOG( ... )
#define LOG_M(format, ... )
#define DBG( ... )
#endif


#define NET_TO_RS232_ENABLE    	1

#if NET_TO_RS232_ENABLE
#define NET_COMM_IP_ADDRESS		("192.168.1.254")
#define NET_COMM_PORT    		(4001)


#endif
