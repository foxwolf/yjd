/***********************************************************************
�ļ����ƣ�UDP_Server.C
��    �ܣ�
��дʱ�䣺
�� д �ˣ�������Ƕ���������Ӽ����Ŷ�
ע    �⣺
***********************************************************************/
#include "main.h"
/***************������ip����*************************/

char *BOARD_IP  	= "192.168.1.200";   		//������ip 
char *BOARD_NETMASK = "255.255.255.0";   		//��������������
char *BOARD_WG		= "192.168.1.1";   		    //������������

#define UDP_LOCAL_PORT			1030			//��UDP���ض˿ں�

int server_sock_fd = -1;

OS_EVENT  *sem_udp_rec_flag = 0;				//udp������һ�������ź�������

char udp_recv_buff[2048];
int udp_recv_len = 0;

struct sockaddr udp_remote_client;

/***********************************************************************
�������ƣ�void ZQWL_W8782_IRQ_Handler(int socket, uint8_t *buff, int size)
��    �ܣ��������ݴ���������������������ʱ���ú������Զ����ã�������������ݾ������ﴦ��
���������sΪsocket�ţ�buff����ָ�룬size�����ݳ��� remote_addr Զ�̵�ַ��Ϣ
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void ZQWL_W8782_IRQ_Handler(int s, uint8_t *buff, int size,struct sockaddr remote_addr)
{

	struct lwip_sock *sock;
	sock = get_socket(s);
	if(s == server_sock_fd)
	{
		if (sock -> conn -> type == NETCONN_UDP)//��udpЭ��
		{
			memcpy(udp_recv_buff,buff,size);	//��������
			udp_recv_len = size;
			udp_remote_client = remote_addr;
			OSSemPost(sem_udp_rec_flag);  		//�׳�һ���ź�����ʾudp�Ѿ��������һ֡����
		}
	}
}
/***********************************************************************
�������ƣ�void Task_UDP_Server(void *pdata)
��    �ܣ�UDP�������������ڴ˿���ʵ�־��幦�ܣ����������ǽ��յ�������ԭ�����ء�
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void Task_UDP_Server(void *pdata)
{
	unsigned  char  os_err;
	
	server_sock_fd = UDP_Create_Server(UDP_LOCAL_PORT);//����UDP������
	if(server_sock_fd == -1)
	{
		printf("UDP_Create_Server faild!\n");
		while(1);
	}	
	printf("UDP_Create_Server ok!\n");  //	
	sem_udp_rec_flag = OSSemCreate(0); 						//����һ���ź���,
	while(1)
	{
		OSSemPend(sem_udp_rec_flag,0,&os_err);				//�����ȴ�sem_udp_rec_flag�ź�����Ч
		UDP_Sendto_Data(server_sock_fd,udp_recv_buff,udp_recv_len,udp_remote_client,MSG_DONTWAIT);//������ԭ������
	}
}
