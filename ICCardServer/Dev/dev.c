#include "includes.h"
#include "dev.h"
#include "packet_format.h"
#include "starup_handle.h"
#include "ic_card.h"
#include "signal.h"

extern int gFd_dev;
static net_packet_head_t packet_head;
static u8 data_buf[0x800];
static char tmp_str[150];
card_block_t old_card_block, card_block;

void *pthread_dev_handle(void *arg)
{
	int ret;
	int connect_failed_cnt = 0;

	signal(SIGPIPE, SIG_IGN);
	while (1)
	{
		ret = ic_card_section_read(gFd_dev, 1, &card_block);
		if (ret == 0)
		{
			if (card_block.len > 0)
			{
				if (0 != compare_ic_block(&old_card_block, &card_block))
				{
					memcpy(&old_card_block, &card_block, sizeof(card_block_t));
					dev_send_msg(msg_DEV_NEW_DATA, 0);
					ic_app_beep(gFd_dev, 30);
				}
			}
			connect_failed_cnt = 0;
		}
		else if( ret == -99)
		{
			LOG("read ic error!\n");
			connect_failed_cnt++;
			if (connect_failed_cnt >= 15)
			{
				connect_failed_cnt = 0;
				dev_retry_connect_handle(&gFd_dev);
			}
		}
		usleep(gSetting_system.DEV_scan_gap * 1000);
		usleep(1000);

	}

	return 0;
}

int IC_upload_gather_data(int server_fd, char *ic_number)
{
	NET_gather_data *p_gather_data;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int w_len;
	xmlChar * xml_buf;
	char *p_send;
	int xml_len;
	int ret;
	char str_tmp[100];

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

	root_node = xmlNewNode(NULL, BAD_CAST "IC");

	xmlDocSetRootElement(doc, root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "DR_IC_NO", BAD_CAST (ic_number));

	xmlNewChild(root_node, NULL, BAD_CAST "IC_DR_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_CO_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_BILL_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_GROSS_WT", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_VE_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_VE_NAME", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_CONTA_ID", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_ESEAL_ID", BAD_CAST "");

	xmlNewChild(root_node, NULL, BAD_CAST "IC_BUSS_TYPE", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_EX_DATA", BAD_CAST "");

	xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

	p_send = strstr((char*) xml_buf, "<IC>");

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

	sprintf(str_tmp, "RE:\t%s\n", ic_number);
	log_send(str_tmp);

	return 0;
}

int IC_upload_arrived_data(int server_fd, char *ic_number)
{
	NET_gather_data *p_gather_data;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int w_len;
	xmlChar * xml_buf;
	char *p_send;
	int xml_len;
	int ret;
	char str_tmp[100];

	packet_head.msg_type = MSG_TYPE_GATHER_DATA;
	strcpy(packet_head.szVersion, "1.0");

	p_gather_data = (NET_gather_data *) (data_buf + sizeof(net_packet_head_t)
			+ 8);

	strcpy(p_gather_data->event_id, gSetting_system.arrived_DEV_event_id);
	ret = 0;
	memcpy(&(p_gather_data->is_data_full), &ret, sizeof(int));
	strcpy(p_gather_data->dev_tag, gSetting_system.gather_DEV_TAG);

	/**********************************XML*********************************/
	doc = xmlNewDoc(NULL);

	root_node = xmlNewNode(NULL, BAD_CAST "IC");

	xmlDocSetRootElement(doc, root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "DR_IC_NO", BAD_CAST (ic_number));

	xmlNewChild(root_node, NULL, BAD_CAST "IC_DR_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_CO_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_BILL_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_GROSS_WT", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_VE_CUSTOMS_NO", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_VE_NAME", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_CONTA_ID", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_ESEAL_ID", BAD_CAST "");

	xmlNewChild(root_node, NULL, BAD_CAST "IC_BUSS_TYPE", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "IC_EX_DATA", BAD_CAST "");

	xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

	p_send = strstr((char*) xml_buf, "<IC>");

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

	ret = write(server_fd, data_buf, w_len);
	if (ret != w_len)
	{
		LOG_M("upload gather data error! ret =%d\n", ret);
	}

	sprintf(str_tmp, "ARRIVED:\t%s\n", ic_number);
	log_send(str_tmp);

	return 0;
}

