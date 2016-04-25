/***********************************************************************
�ļ����ƣ�wifi_config.h
��    �ܣ�
��дʱ�䣺
�� д �ˣ�������Ƕ���������Ӽ����Ŷ�
ע    �⣺
***********************************************************************/
#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_



// #define DEBUG_LEVEL1
//#define DEBUG_LEVEL2

#ifdef STA_SUPPORT	
extern struct net_device *sta_wifi_dev ;
#endif
#ifdef UAP_SUPPORT	
extern struct net_device *ap_wifi_dev;
#endif
#ifdef WIFI_DIRECT_SUPPORT
extern struct net_device *direct_wifi_dev;
#endif
extern struct net_device *wifi_dev;

void Fix_ZQWL_Params(void);
int Power_Mode(char mode);
void print_scan_result(struct scan_result_data *dat);

#endif
