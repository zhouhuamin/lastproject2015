#pragma once


#define SYSTEM_SETTING_CML_FILE_NAME   "config.xml"

typedef struct
{
	char server_ip[20];
	int server_port;
	int keeplive_gap;
	int RFID_scan_gap;
	int com_comx;
	comm_setting_t com_setting;
	char com_ip[20];
	int com_port;
	int ic_timeout;
	int RFID_scan_duration_time;
	int RFID_label_type;  //标签类型 0: iso-180006C  1:iso-180006B
	char register_DEV_TAG[32];
	char register_DEV_ID[32];
	char SYS_CTRL_R_start_event_ID[32];
	char SYS_CTRL_R_stop_event_ID[32];
	char gather_event_id[32];
	char gather_DEV_TAG[32];
} sSystem_setting;

typedef struct
{
	int TK900_connect_flag;  //reader connect flag 0: no connection 1:connected
	int dev_register_flag;   //device register flag
	int work_flag;           //0:wait 1:working
}sRealdata_system;

int parame_init(void);
int set_parame_default(void);
int load_setting_from_xml(sSystem_setting *p_setting);
int save_setting_to_xml(sSystem_setting *p_setting);



extern sSystem_setting gSetting_system;
extern sRealdata_system gReal_system;
