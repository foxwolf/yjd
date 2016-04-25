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

#define UDP_LOCAL_PORT			5000			//��UDP���ض˿ں�
#define UDP_REMOTE_PORT			1030			//��UDPԶ�˶˿ں�
#define	UDP_REMOTE_IP			"192.168.1.107"	//Զ������IP

int client_sock_fd = -1;
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
	if(s == client_sock_fd)
	{
		if (sock -> conn -> type == NETCONN_UDP)//��udpЭ��
		{
			memcpy(udp_recv_buff,buff,size);	//��������
			udp_recv_len = size;
			udp_remote_client = remote_addr;
			UDP_Sendto_Data(client_sock_fd,udp_recv_buff,udp_recv_len,udp_remote_client,MSG_DONTWAIT);//������ԭ������
		}
	}
}
/***********************************************************************
�������ƣ�void Task_UDP_Client(void *pdata)
��    �ܣ�UDP�ͻ��˵�����ÿ��500ms���������������һ������
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void Task_UDP_Client(void *pdata)
{
	int ret;
	struct sockaddr_in addr;
	struct sockaddr  remote_addr;
	
	client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);	//����һ��UDP socket�ţ����ø�socket��Ŀ������������
	if (client_sock_fd < 0)								//�����ȡʧ�ܣ���ӡ�����־
	{
		printf("get socket err:%d\n", errno);				
	} 
	//���°󶨱��ض˿�
	addr.sin_family = AF_INET;					//Э������  IPV4  internet Э��
	addr.sin_len = sizeof(struct sockaddr_in);	
	addr.sin_port = htons(UDP_LOCAL_PORT);		//���˿ں�ת�������ֽ�˳��
	addr.sin_addr.s_addr = htons(INADDR_ANY);	//��������IP����������
	ret = bind(client_sock_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));//��client_sock_fd
	{
		if(ret < 0)//�����ʧ��
		{
			printf("bind socket err\n");		
		}
	}
	
	//��������Ŀ��������ַ��Ϣ
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(UDP_REMOTE_PORT);
	addr.sin_addr.s_addr = inet_addr(UDP_REMOTE_IP);	
	memcpy(&remote_addr, &addr, sizeof(struct sockaddr_in));//װ�ص�ַ��Ϣ��remote_addr
	
	while(1)
	{
		UDP_Sendto_Data(client_sock_fd,"UDP �ͻ������ݣ�\n",sizeof("UDP �ͻ������ݣ�\n"),remote_addr,0);//ע����Ҫ�رձʼǱ��ķ���ǽ�����ܽ��յ����ݰ�
		OSTimeDlyHMSM(0, 0, 0, 500);						//����500ms�Ա�������������
	}
}
