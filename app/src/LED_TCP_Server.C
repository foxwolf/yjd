/***********************************************************************
�ļ����ƣ�LED_TCP_Server.C
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

#define TCP_LOCAL_PORT			1030			//��TCP�������˿ں�

int tcp_server_sock_fd = -1;					//TCP������socket���
int a_new_client_sock_fd = -1;					//TCP�ͻ���socket���

char tcp_recv_flag = 0;
OS_EVENT  *sem_tcp_rec_flag = 0;				//tcp������һ�������ź�������

char tcp_recv_buff[2048];						//tcp���ݽ��ջ�����
int tcp_recv_len = 0;							//tcp���ݽ��ճ���


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
	if(sock->conn->pcb.tcp->local_port == TCP_LOCAL_PORT)//�뿪����TCP�˿ں�һ��
	{
		if (sock->conn->type == NETCONN_TCP)//��TCPЭ��
		{
			memcpy(tcp_recv_buff,buff,size);//��������
			tcp_recv_flag = 1;
			tcp_recv_len = size;
			a_new_client_sock_fd = s;
			OSSemPost(sem_tcp_rec_flag);  //�׳�һ���ź�����ʾtcp�Ѿ��������һ֡����
		}
	}
}
/***********************************************************************
�������ƣ�void Task_TCP_server(void *pdata)
��    �ܣ�TCP�������������ڴ˿���ʵ�־��幦�ܣ����������ǽ��յ�������ԭ�����ء�
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void Task_TCP_server(void *pdata)
{
	unsigned  char  os_err;
	
	tcp_server_sock_fd = Create_TCP_Server(TCP_LOCAL_PORT);//��ʼ��һ��TCP����socket �˿�ΪTCP_LOCAL_PORT����ʼ����
	sem_tcp_rec_flag = OSSemCreate(0); 						//����һ���ź���,
	while(1)
	{
		OSSemPend(sem_tcp_rec_flag,0,&os_err);				//�����ȴ�sem_tcp_rec_flag�ź�����Ч
		LED_Ctr();
	}
}

LED_Ctr();