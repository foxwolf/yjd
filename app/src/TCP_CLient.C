/***********************************************************************
�ļ����ƣ�TCP_CLient.C
��    �ܣ�
��дʱ�䣺
�� д �ˣ�������Ƕ���������Ӽ����Ŷ�
ע    �⣺
***********************************************************************/
#include "main.h"
/***************������ip����*************************/

char *BOARD_IP  	= "192.168.5.200";   		//������ip 
char *BOARD_NETMASK = "255.255.255.0";   		//��������������
char *BOARD_WG		= "192.168.5.1";   		    //������������

#define TCP_SERVER_PORT			16882			//��TCP�������˿ںţ�Զ�ˣ�
//#define TCP_SERVER_IP			"119.90.35.78"	//��TCP������IP��Զ�ˣ�
//#define TCP_SERVER_IP			"222.35.226.137"	//��TCP������IP��Զ�ˣ�
#define TCP_SERVER_IP			"192.168.5.159"

int client_sock_fd = -1;

char tcp_recv_flag = 0;
OS_EVENT  *sem_tcp_rec_flag = 0;				//tcp������һ�������ź�������

char tcp_recv_buff[2048];
int tcp_recv_len = 0;

extern int mcuID[3];


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
	
	if(s == client_sock_fd)
	{
		if (sock -> conn -> type == NETCONN_TCP)//��TCPЭ��
		{
			memcpy(tcp_recv_buff,buff,size);//��������
			tcp_recv_flag = 1;
			tcp_recv_len = size;
			TCP_Send_Data(client_sock_fd,tcp_recv_buff,tcp_recv_len,MSG_DONTWAIT);//ԭ����������
		}
	}
}

int send_ex(int sock, void * buffer, unsigned long len)
{
	int sendlen = 0, total_len = (int)len;

	while (total_len > 0)
	{
		sendlen = TCP_Send_Data(sock, buffer, total_len,MSG_DONTWAIT);//TCP�ͻ����������������������
		if (YJ_FAILURE == sendlen)
		{
			return YJ_FAILURE;
		}

		total_len -= sendlen;

		buffer = (void *)((unsigned long)buffer + sendlen);
	}

	return YJ_SUCCESS;
}

/***********************************************************************
�������ƣ�void Task_TCP_Client(void *pdata)
��    �ܣ�TCP�ͻ��˵�����ÿ��500ms���������������һ�����ݣ�������һ���������״̬������
		  ��������Ծ����֪���������Ƿ�Ͽ�����
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void Task_TCP_Client(void *pdata)
{
	struct lwip_sock *sock;
	INT8U 			err;
	parallet_msg_t	*pblock = NULL;
	OS_Q_DATA  		qInfo;
	int				i;
	INT8U	u8Buf[BUF_SIZE+32];
	INT16U	len;
	int 	ret;
	
	//client_sock_fd = TCP_Link(TCP_SERVER_IP,TCP_SERVER_PORT);//���������������
	//while(client_sock_fd == -1)//һֱ���� ֱ���ɹ�
	//{
	//	printf("TCP Linking...\n");  //
	//	client_sock_fd = TCP_Link(TCP_SERVER_IP,TCP_SERVER_PORT);//���������������
	//	OSTimeDlyHMSM(0, 0, 0, 500);//
	//}	
	//printf("TCP_Link ok!\n");  //	
	//����һ�������������client_sock_fd��״̬
	//thread_create(Task_Client_Sock_Stats, 0, TASK_CLIENT_SOCK_STATS_PRIO, 0, TASK_CLIENT_SOCK_STATS_STACK_SIZE, "Task_Client_Sock_Stats");
	while(1)
	{
		err = OSQQuery(hParalletMsgQueue, &qInfo);
		if (OS_NO_ERR == err)
		{
			if (qInfo.OSNMsgs > 0)
			{
				pblock = (parallet_msg_t *)OSQAccept(hParalletMsgQueue, NULL);
				if (pblock != NULL)
				{
					len = pblock->len;
					u8Buf[0]='$';
					u8Buf[1]='B';
					u8Buf[2]='Y';
					u8Buf[3]='P';
					u8Buf[4]='$';
					u8Buf[5]=0x01;
					u8Buf[6]=0x0C;
					u8Buf[7]=0xE9;
					u8Buf[8]=0x00;
					u8Buf[9]=0x00;
					*((YJ_U32*)(&u8Buf[10])) = ntohl(len+12);
					*((YJ_U32*)(&u8Buf[14]))=ntohl(mcuID[0]);
					*((YJ_U32*)(&u8Buf[18]))=ntohl(mcuID[1]);
					*((YJ_U32*)(&u8Buf[22]))=ntohl(mcuID[2]);
					for (i=0; i< len; i++)
					{
						//printf("%c",pblock->u8Buf[i]);
						u8Buf[26+i] = pblock->u8Buf[i];
					}
					OSMemPut(g_hParalletRxMem, pblock);
					client_sock_fd = TCP_Link(TCP_SERVER_IP,TCP_SERVER_PORT);//���������������
					while(client_sock_fd == -1)//һֱ���� ֱ���ɹ�
					{
						printf("TCP Linking...\n");  //
						client_sock_fd = TCP_Link(TCP_SERVER_IP,TCP_SERVER_PORT);//���������������
						OSTimeDlyHMSM(0, 0, 0, 500);//
					}
					sock = get_socket(client_sock_fd);
					if(sock->conn->pcb.tcp->state == ESTABLISHED)//�Ѿ�������
					{
						ret = send_ex(client_sock_fd, u8Buf, len+26);
						if (YJ_FAILURE == ret)
						{
							printf("???\n");
						}
						TCP_Unlink(client_sock_fd);
					}
				}
			}
		}
		OSTimeDly(1);
	}
}

/***********************************************************************
�������ƣ�void Task_Client_Sock_Stats(void *pdata)
��    �ܣ�����client_sock_fd���ӵ�״̬����������������ӣ�ÿ����һ��
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void Task_Client_Sock_Stats(void *pdata)
{
	struct lwip_sock *sock;
	while(1)
	{
		sock = get_socket(client_sock_fd);
		if(sock->conn->pcb.tcp->state != ESTABLISHED)//��û������
		{
			printf("TCP Link faild!\n");  //
			printf("TCP Link...!\n");  //
			client_sock_fd = TCP_Link(TCP_SERVER_IP,TCP_SERVER_PORT);//���������������
			if(client_sock_fd != -1)
			{
				printf("TCP Link ok!\n");  //
			}
		}
		OSTimeDlyHMSM(0, 0, 1, 0);//����1s
	}
}
