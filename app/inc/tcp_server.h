/***********************************************************************
�ļ����ƣ�TCP_Server.h
��    �ܣ�
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

extern char *BOARD_IP ;   			//������ip 
extern char *BOARD_NETMASK;   		//��������������
extern char *BOARD_WG;   		    //������������

void Task_TCP_server(void *pdata);

#endif



