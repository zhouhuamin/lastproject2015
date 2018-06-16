#include "includes.h"
#include "common.h"
#include <sys/time.h>
#include <time.h>

char str_tmp[0x800];

u32 get_system_ms(void)
{
	static u32 first_in = 1;
	static struct timeval tm_s;
	struct timeval tm_cur;

	if (first_in)
	{
		first_in = 0;
		gettimeofday(&tm_s, 0);
	}

	gettimeofday(&tm_cur, 0);

	return ((tm_cur.tv_sec * 1000 + tm_cur.tv_usec / 1000)
			- (tm_s.tv_sec * 1000 + tm_s.tv_usec / 1000));
}

int log_write(char *info)
{
	FILE *fp;

	if ( NULL == info)
	{
		return -1;
	}
	fp = fopen(LOG_FILE_NAME, "w");
	if (fp < 0)
	{
		return -1;
	}

	fwrite(info, strlen(info), 1, fp);
	fclose(fp);

	return 0;
}

int log_add(char *info, int tim_flag)
{
	FILE *fp;
	time_t tim;
	struct tm *tim_info;

	if ( NULL == info)
	{
		return -1;
	}

	fp = fopen(LOG_FILE_NAME, "a");
	if (fp < 0)
	{
		return -1;
	}

	printf(info);
	if (tim_flag)
	{
		time(&tim);
		tim_info = localtime(&tim);
		sprintf(str_tmp, "[ %04d %02d/%02d %02d:%02d:%02d ] %s", tim_info->tm_year+1900,
				tim_info->tm_mon+1, tim_info->tm_mday, tim_info->tm_hour,
				tim_info->tm_min, tim_info->tm_sec, info);
		fwrite(str_tmp, strlen(str_tmp), 1, fp);
	}
	else
	{
		fwrite(info, strlen(info), 1, fp);
	}
	fclose(fp);
	return 0;
}
