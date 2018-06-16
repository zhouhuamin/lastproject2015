#include "includes.h"
#include "config.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "packet_format.h"
#include <ctype.h>
#include "msg.h"

sSystem_setting gSetting_system;
sRealdata_system gReal_system;

static int ip_address_check(char *p_ip_str);
static int interger_check(char *p_str);
static int com_parity_check(char *p_str);

//参数初始化
int parame_init(void)
{
	int ret;
	comm_setting_t com_set_r;
	comm_setting_t *p_setting;

	ret = load_setting_from_xml(&gSetting_system);
	if (ret < 0)
	{
		LOG("load setting error!\n");

		set_parame_default();
		save_setting_to_xml(&gSetting_system);
	}

	p_setting = &(gSetting_system.com_setting);
	ret = comm_settings_read(gSetting_system.com_comx, &com_set_r);
	if (0 != ret)
	{
		LOG("read comm setting error!\n");
		return -1;
	}
	else
	{
		LOG_M("baudrate = %d\ndata_bits=%d\nstop_bits =%d\nparity=%c\n",
				com_set_r.baudrate, com_set_r.data_bits, com_set_r.stop_bits,
				com_set_r.parity);
	}

	if (p_setting->baudrate != com_set_r.baudrate
			|| p_setting->data_bits != com_set_r.data_bits
			|| p_setting->stop_bits != com_set_r.stop_bits
			|| toupper(p_setting->parity) != toupper(com_set_r.parity))

	{
		ret = comm_setting_write(gSetting_system.com_comx, p_setting);

		if (0 != ret)
		{
			LOG("comm set error!\n");
			return -1;
		}
		sleep(6);
	}
	else
	{
		LOG("parame is same!\n");
	}

	gReal_system.TK900_connect_flag = 0;
	gReal_system.dev_register_flag = 0;
	gReal_system.work_flag = 0;

	msg_queue_init();
	return 0;
}

//将参数设置到默认状态
int set_parame_default(void)
{
	strcpy(gSetting_system.server_ip, SERVER_IP_ADDRESS_STR);
	strcpy(gSetting_system.com_ip, NET_COMM_IP_ADDRESS);
	gSetting_system.server_port = SERVER_NET_PORT;
	gSetting_system.com_port = NET_COMM_PORT;
	gSetting_system.RFID_scan_gap = 200;
	gSetting_system.keeplive_gap = 5000;
	gSetting_system.com_comx = 1;
	gSetting_system.com_setting.baudrate = 57600;
	gSetting_system.com_setting.data_bits = 8;
	gSetting_system.com_setting.stop_bits = 1;
	gSetting_system.com_setting.parity = 'N';
	gSetting_system.RFID_scan_duration_time = 0;
	gSetting_system.RFID_label_type = 0;
	strcpy(gSetting_system.register_DEV_TAG, REGISTER_DEV_TAG);
	strcpy(gSetting_system.register_DEV_ID, REGISTER_DEV_ID);
	strcpy(gSetting_system.SYS_CTRL_R_start_event_ID,
	SYS_CTRL_R_START_EVENT_ID);
	strcpy(gSetting_system.SYS_CTRL_R_stop_event_ID, SYS_CTRL_R_STOP_EVENT_ID);

	strcpy(gSetting_system.gather_event_id, GATHER_DATA_EVENT_ID);
	strcpy(gSetting_system.gather_DEV_TAG, GATHER_DATA_DEV_TAG);

	return 0;
}

int load_setting_from_xml(sSystem_setting *p_setting)
{
	xmlDocPtr doc;
	xmlNodePtr root_node, cur_node;
	xmlChar *str_tmp;

	doc = xmlReadFile(SYSTEM_SETTING_CML_FILE_NAME, "utf-8", XML_PARSE_RECOVER);
	if ( NULL == doc)
	{
		LOG("open xml file error!\n");
		return -1;
	}

	root_node = xmlDocGetRootElement(doc);
	if ( NULL == root_node)
	{
		LOG("get the root node error!\n");
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(root_node->name, BAD_CAST "RFID_DEV"))
	{
		LOG("the root node name  error!\n");
		xmlFreeDoc(doc);
		return -2;
	}

	cur_node = root_node->children;
	while ( NULL != cur_node)
	{
		if (!xmlStrcmp(cur_node->name, BAD_CAST "SERVER_IP"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == ip_address_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s", gSetting_system.server_ip);
			}
			else
			{
				LOG("load server ip error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "SERVER_PORT"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d", &gSetting_system.server_port);
			}
			else
			{
				LOG("load server port error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "KEEPLIVE_GAP"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d", &gSetting_system.keeplive_gap);
			}
			else
			{
				LOG("load keeplive gap error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_SCAN_GAP"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d", &gSetting_system.RFID_scan_gap);
			}
			else
			{
				LOG("load RFID scan gap error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_COMX"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d", &gSetting_system.com_comx);
			}
			else
			{
				LOG("load RFID COMX error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_BAUDRATE"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d",
						&gSetting_system.com_setting.baudrate);
			}
			else
			{
				LOG("load RFID BAUDRATE error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_DATA_BITS"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d",
						&gSetting_system.com_setting.data_bits);
			}
			else
			{
				LOG("load RFID DATA bits error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_STOP_BITS"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d",
						&gSetting_system.com_setting.stop_bits);
			}
			else
			{
				LOG("load rfid stop bits error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_PARITY"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == com_parity_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%c",
						&gSetting_system.com_setting.parity);
			}
			else
			{
				LOG("load comm parity error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_COM_IP"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == ip_address_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s", gSetting_system.com_ip);
			}
			else
			{
				LOG("load rfid com ip port error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_COM_PORT"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d", &gSetting_system.com_port);
			}
			else
			{
				LOG("load rfid com port port error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_SCAN_DURATION_TIME"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%d",
						&gSetting_system.RFID_scan_duration_time);
			}
			else
			{
				LOG("load scan duration time error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "RFID_LABEL_TYPE"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == strcmp((char*) str_tmp, "ISO1800-6B"))
			{
				gSetting_system.RFID_label_type = 1;
			}
			else
			{
				gSetting_system.RFID_label_type = 0;
			}
			xmlFree(str_tmp);
		}

		else if (!xmlStrcmp(cur_node->name, BAD_CAST "REGISTER_DEV_TAG"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 != str_tmp)
			{
				sscanf((char *) str_tmp, "%s",
						gSetting_system.register_DEV_TAG);
			}
			else
			{
				LOG("load reg dev tag port error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "REGISTER_DEV_ID"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s", gSetting_system.register_DEV_ID);
			}
			else
			{
				LOG("load reg dev id error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name,
		BAD_CAST "SYS_CTRL_R_START_EVENT_ID"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s",
						gSetting_system.SYS_CTRL_R_start_event_ID);
			}
			else
			{
				LOG("load SYS_CTRL_R_START_EVENT_ID error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name,
		BAD_CAST "SYS_CTRL_R_STOP_EVENT_ID"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s",
						gSetting_system.SYS_CTRL_R_stop_event_ID);
			}
			else
			{
				LOG("load SYS_CTRL_R_STOP_EVENT_ID error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "GATHER_DATA_EVENT_ID"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 == interger_check((char *) str_tmp))
			{
				sscanf((char *) str_tmp, "%s", gSetting_system.gather_event_id);
			}
			else
			{
				LOG("load GATHER_DATA_EVENT_ID error!\n");
			}
			xmlFree(str_tmp);
		}
		else if (!xmlStrcmp(cur_node->name, BAD_CAST "GATHER_DATA_DEV_TAG"))
		{
			str_tmp = xmlNodeGetContent(cur_node);
			if (0 != str_tmp)
			{
				sscanf((char *) str_tmp, "%s", gSetting_system.gather_DEV_TAG);
			}
			else
			{
				LOG("load GATHER_DATA_DEV_TAG error!\n");
			}
			xmlFree(str_tmp);
		}

		cur_node = cur_node->next;
	}
	xmlFreeDoc(doc);
	return 0;
}

int save_setting_to_xml(sSystem_setting *p_setting)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int ret;
	char str_tmp[50];

	/**********************************XML*********************************/
	doc = xmlNewDoc(BAD_CAST "1.0");

	root_node = xmlNewNode(NULL, BAD_CAST "RFID_DEV");

	xmlDocSetRootElement(doc, root_node);

	sprintf(str_tmp, "%s", gSetting_system.server_ip);
	xmlNewChild(root_node, NULL, BAD_CAST "SERVER_IP", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.server_port);
	xmlNewChild(root_node, NULL, BAD_CAST "SERVER_PORT", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.keeplive_gap);
	xmlNewChild(root_node, NULL, BAD_CAST "KEEPLIVE_GAP", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.RFID_scan_gap);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_SCAN_GAP", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.com_comx);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_COMX", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.com_setting.baudrate);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_BAUDRATE", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.com_setting.data_bits);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_DATA_BITS", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.com_setting.stop_bits);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_STOP_BITS", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%c", gSetting_system.com_setting.parity);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_PARITY", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.com_ip);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_COM_IP", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.com_port);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_COM_PORT", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", gSetting_system.RFID_scan_duration_time);
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_SCAN_DURATION_TIME",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s",
			(gSetting_system.RFID_label_type != 0) ?
					("ISO1800-6B") : ("ISO1800-6C"));
	xmlNewChild(root_node, NULL, BAD_CAST "RFID_LABEL_TYPE",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.register_DEV_TAG);
	xmlNewChild(root_node, NULL, BAD_CAST "REGISTER_DEV_TAG",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.register_DEV_ID);
	xmlNewChild(root_node, NULL, BAD_CAST "REGISTER_DEV_ID",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.SYS_CTRL_R_start_event_ID);
	xmlNewChild(root_node, NULL, BAD_CAST "SYS_CTRL_R_START_EVENT_ID",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.SYS_CTRL_R_stop_event_ID);
	xmlNewChild(root_node, NULL, BAD_CAST "SYS_CTRL_R_STOP_EVENT_ID",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.gather_event_id);
	xmlNewChild(root_node, NULL, BAD_CAST "GATHER_DATA_EVENT_ID",
	BAD_CAST (str_tmp));

	sprintf(str_tmp, "%s", gSetting_system.gather_DEV_TAG);
	xmlNewChild(root_node, NULL, BAD_CAST "GATHER_DATA_DEV_TAG",
	BAD_CAST (str_tmp));

	ret = xmlSaveFormatFileEnc(SYSTEM_SETTING_CML_FILE_NAME, doc, "utf-8",
			XML_PARSE_RECOVER);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	xmlMemoryDump();
	/**********************************XML END*********************************/

	return ret;
}

static int ip_address_check(char *p_ip_str)
{
	int len;
	int i;
	int dot_number = 0;

	if ( NULL == p_ip_str)
	{
		return -1;
	}

	len = strlen(p_ip_str);

	if (len < 8 || len > 16) //1.1.1.1  and 192.168.128.254
	{
		return -2;
	}

	for (i = 0; i < len; i++)
	{
		if (p_ip_str[i] == '.')
		{
			dot_number++;
			continue;
		}
		if ((p_ip_str[i] < '0') || (p_ip_str[i] > '9')) //invalid char
		{
			return -3;
		}
	}

	if (3 != dot_number) //format error
	{
		return -4;
	}

	return 0;
}

static int interger_check(char *p_str)
{
	int len;
	int i;

	if ( NULL == p_str)
	{
		return -1;
	}

	len = strlen(p_str);

	for (i = 0; i < len; i++)
	{
		if ((p_str[i] < '0') || (p_str[i] > '9')) //invalid char
		{
			return -2;
		}
	}

	return 0;
}

static int com_parity_check(char *p_str)
{
	int len;

	if ( NULL == p_str)
	{
		return -1;
	}

	len = strlen(p_str);
	if (1 != len)
	{
		LOG_M("len = %d\n", len);
		return -2;
	}

	if ((p_str[0] == 'N') || (p_str[0] == 'n') || (p_str[0] == 'O')
			|| (p_str[0] == 'o') || (p_str[0] == 'E') || (p_str[0] == 'e'))
	{

	}
	else
	{
		return -3;
	}

	return 0;
}

