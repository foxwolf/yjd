// #define DEBUG
#include "main.h"

// int UDP_server_socket_fd = -1;
int dbg_level=1;

static int tcp_server_accept_thread_fd = - 1;

struct netif if_wifi;
struct netif *p_netif = NULL; //һ��netif��Ӧһ�������豸
//����select�����ı���
fd_set rfds;
//socket������
mutex_t socket_mutex;

/***********************************************************************
�������ƣ�void ZQWL_System_Clock_Init()
��    �ܣ�ϵͳ���õ�ʱ�ӳ�ʼ����RCC_AHB1Periph_GPIOA/B/C/D��ʼ��/�����������ʱ�ӣ�ucos���ģ�NVIC_PriorityGroup_4
����������˿ں�
���������socket��
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void ZQWL_System_Clock_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); //�ⲿ�ж���Ҫ�õ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);	//��ʼ��IO��ʱ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC - 1);
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);		//��ʼ�������ʱ��
}

/***********************************************************************
�������ƣ�u32_t Auto_Get_IP(int time)
��    �ܣ��Զ���ȡIP
�����������ȡ��ʱʱ�� ��λs
�����������ȡ��IP u32_t  ����-1ʧ��
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
u32_t Auto_Get_IP(int time)
{
	int i = 0;
	struct ip_addr ipaddr;
	ipaddr.addr = 0;
	p_dbg_enter;
	netif_set_down(p_netif); //�ȹص�,dhcp_start��������
	netif_set_ipaddr(p_netif, &ipaddr); //����
	netif_set_netmask(p_netif, &ipaddr);
	netif_set_gw(p_netif, &ipaddr);
	dns_setserver(0, &ipaddr);
	dhcp_start(p_netif);
	
	for (i = 0; i < time; i++)
	{
		printf("%d", i);
		if (p_netif->ip_addr.addr)
			break;
		sleep(1000);
	}
	if (p_netif->ip_addr.addr)
	{
// 		show_tcpip_info();
		return(p_netif->ip_addr.addr);
	}	
	return 0;
}
/***********************************************************************
�������ƣ�int TCP_Link(char *ip,uint16_t port)
��    �ܣ����ӵ�ָ����IP�Ͷ˿ں�
���������ip��192.168.1.101  port 1030
���������wifiδ���ӣ�-1���ɹ�����socket��
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
int TCP_Link(char *ip,uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;
	
	p_dbg("���ӵ�:%s, �˿ں�:%d", ip, port);
	if (!is_wifi_connected())
	{
		p_err("wifi not connect");
		return - 1;
	}
	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	socket_c = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_c < 0)
	{
		p_err("get socket err:%d", errno);
		return  - 1;
	}
	//mutex_lock(socket_mutex);
	err = connect(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	//mutex_unlock(socket_mutex);
	if (err ==  - 1)
	{
		p_err("link err:%d\n", errno);
		close_socket(socket_c);
		return  - 1;
	}

	//����keepalive�����Զ�̶η������رգ����ǿ��Ծ����������ͷ��Ѿ��Ͽ�������
	err = 1;
	if (setsockopt(socket_c, SOL_SOCKET, SO_KEEPALIVE, &err, sizeof(int)) ==  - 1)
		p_err("SO_KEEPALIVE err\n");

	p_dbg("link ok\n");
	return socket_c;
}

/***********************************************************************
�������ƣ�void TCP_Unlink(int sock)
��    �ܣ����ӵ�ָ��socket�ŵ�����
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
int TCP_Unlink(int sock)
{
	int ret;
	p_dbg_enter;
	if (sock !=  - 1)
	{
		ret = close_socket(sock);
		sock =  - 1;
		return ret;
	}
	return - 1;
	p_dbg_exit;
}
/***********************************************************************
�������ƣ�void TCP_Unlink(int sock)
��    �ܣ�����һ��TCP���ӣ�����ʼ����
���������
���������socket��
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
int Create_TCP_Server(uint16_t port)
{

	int socket_s =  - 1, err = 0;

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	p_dbg_enter;

	p_dbg("����������, �˿ں�:%d", port);
// 	test_close_tcp_server();

	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htons(INADDR_ANY);

	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s ==  - 1)
	{
		goto err;
	}
	err = bind(socket_s, (struct sockaddr*) &serv, sizeof(struct sockaddr_in));
	if (err ==  - 1)
	{
		goto err;
	} err = listen(socket_s, 4);
	if (err ==  - 1)
	{
		goto err;
	}

// 	tcp_server_socket_fd = socket_s;

	tcp_server_accept_thread_fd = thread_create(TCP_Accept_Task, (void*)socket_s, TASK_TCP_ACCEPT_PRIO, 0, TASK_ACCEPT_STACK_SIZE, "tcp_accept_task");

	
	return socket_s;
	err: if (err < 0)
		p_err("err:%d", err);
	if (socket_s !=  - 1)
		close_socket(socket_s);
	p_dbg_exit;
	return -1;
	
}

/***********************************************************************
�������ƣ�void TCP_Accept_Task(void *server_fd)
��    �ܣ���ʼ����
���������
���������socket��
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
void TCP_Accept_Task(void *server_fd)
{
	int sockaddr_len, new_socket, opt;
	struct sockaddr_in addr;

	sockaddr_len = sizeof(struct sockaddr);

	while (1)
	{
		p_dbg("waiting for remote connect");
		new_socket = accept((int)server_fd, (struct sockaddr*) &addr, (socklen_t*) &sockaddr_len);
		if (new_socket ==  - 1)
		{
			p_err("accept err");
			break;
		} p_dbg("accept a new client");
// 		remote_socket_fd = new_socket;
		opt = 1;
		if (setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int)) ==  - 1)
			p_err("SO_KEEPALIVE err\n");

	}
	thread_exit(tcp_server_accept_thread_fd);
}

/***********************************************************************
�������ƣ�int TCP_Send_Data(int s,char *data)
��    �ܣ���ָ����socket���Ϸ�������
���������
������������͵����ݳ���
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/

int TCP_Send_Data(int s,char *data,int len,int flags)
{
	int ret;// len = strlen(data);
	p_dbg_enter;
	if (s !=  - 1)
	{
		mutex_lock(socket_mutex);
		ret = send(s, data, len, flags);
		mutex_unlock(socket_mutex);
// 		if (ret != len)
// 		{
// 			p_err("send data err:%d", ret);
// 			close_socket(s);
// 			s =  - 1;
// 		}
		return ret;
	}
	return - 1;
// 	ADD_MONITOR_VALUE(tcp_totol_send, len);
	p_dbg_exit;
}

/*
 * @brief �ر�һ��socket
 *ע�������ִ��mutex_lock(socket_mutex);Ӧ���з�ֹǶ��
 * 
 */
int close_socket(uint32_t socket_num)
{
	struct lwip_sock *sock;
	int ret;
	p_dbg_enter;
	sock = get_socket(socket_num);

	if (!sock || !sock->conn)
	{
		p_err("close_socket err1\n");
		return  - 1;
	} if (!((sock->conn->state != NETCONN_CONNECT) || (NETCONN_FLAG_IN_NONBLOCKING_CONNECT &(sock->conn->flags))))
	{
		p_err("close_socket err2\n");
		return  - 1;
	}
	mutex_lock(socket_mutex);
	//shutdown(socket_num, SHUT_RDWR);
	ret = close(socket_num);
	if (ret ==  - 1)
		p_err("close_socket err4:%d\n", ret);
	if (FD_ISSET(socket_num, &rfds))
		FD_CLR(socket_num, &rfds);
	mutex_unlock(socket_mutex);
	p_dbg_exit;
	return ret;
}

/***********************************************************************
�������ƣ�uint32_t DNS(char *hostname)
��    �ܣ�dns����
�������������
����������ɹ�����IP uint32_t ʧ�ܷ���-1
��дʱ�䣺
�� д �ˣ�
ע    �⣺
һ��ϸ�ڣ�DNS���õ����������ķ�����Ĭ����208.67.222.222������һ�£�������resolver1.opendns.com��
ͬ����Ҳ������dns_setserver����DNS������ip��ַ������������������һ���������ţ�һ���Ƿ�������ip��ַ��
�������ǲ��ܴ���DNS_MAX_SERVERS�����ġ�
***********************************************************************/
uint32_t DNS(char *hostname)
{

	char err_t;
	ip_addr_t ip_addr;
	p_dbg_enter;

	/**
	 * Execute a DNS query, only one IP address is returned
	 *
	 * @param name a string representation of the DNS host name to query
	 * @param addr a preallocated ip_addr_t where to store the resolved IP address
	 * @return ERR_OK: resolving succeeded
	 *         ERR_MEM: memory error, try again later
	 *         ERR_ARG: dns client not initialized or invalid hostname
	 *         ERR_VAL: dns server response was invalid
	 dns_gethostbyname("www.openedv.com",&DNS_Addr,dns_serverFound,NULL);	
	*/
	mutex_lock(socket_mutex);
// 	err_t = dns_gethostbyname(hostname,&ip_addr,0,NULL);
	err_t = netconn_gethostbyname(hostname, &ip_addr);
	mutex_unlock(socket_mutex);
	if (err_t == ERR_OK)
	{
// 		addr = ip_addr.addr;
// 		p_dbg("get %s, ipaddr:: %d.%d.%d.%d\n", hostname, ip4_addr1(&addr), ip4_addr2(&addr), ip4_addr3(&addr), ip4_addr4(&addr));
		return ip_addr.addr;
	}
	p_dbg_exit;
	return err_t;
}

/***********************************************************************
�������ƣ�int UDP_Create_Server(uint16_t port)
��    �ܣ�
���������
���������
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
int UDP_Create_Server(uint16_t port)
{
	int socket_c, err;
	struct sockaddr_in addr;

	p_dbg("enter %s\n", __FUNCTION__);
	addr.sin_family = AF_INET;
	addr.sin_len = sizeof(struct sockaddr_in);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htons(INADDR_ANY);

	socket_c = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_c < 0)
	{
		p_err("get socket err:%d", errno);
		return  - 1;
	} 
	mutex_lock(socket_mutex);
	err = bind(socket_c, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));
	p_dbg("bind ret:%d", err);
	if (err ==  - 1)
	{
		close(socket_c);
		mutex_unlock(socket_mutex);
		return  - 1;

	}
#if 0	//���ϣ�����չ㲥����,����1
	err = setsockopt(socket_c, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n));
	p_dbg("SO_BROADCAST ret:%d", err);
	if(err == -1)
	{
	close(socket_c);
	mutex_unlock(socket_mutex);
	return -1;
	}
#endif
	mutex_unlock(socket_mutex);
	p_dbg("server ok:%d", socket_c);
	return socket_c;
}
/***********************************************************************
�������ƣ�int UDP_Send_Data(int s,char *data)
��    �ܣ���ָ����socket���Ϸ�������
���������
������������͵����ݳ���
��дʱ�䣺
�� д �ˣ�
ע    �⣺
***********************************************************************/
int UDP_Sendto_Data(int socket,char *data,int len,struct sockaddr remote_addr,int flags)
{
	int   ret;
// 	struct lwip_sock *sock;
	p_dbg("enter %s ,%d\n", __FUNCTION__, socket);

// 	sock = get_socket(socket);
// 	if (!sock || !sock->conn)
// 	{
// 		return 0;
// 	}

// 	if (sock->conn->type != NETCONN_UDP)
// 	{
// 		p_err("is not udp socket");
// 		return 0;
// 	}
	mutex_lock(socket_mutex);
	ret = sendto(socket, (u8*)data, len, flags, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr));
	mutex_unlock(socket_mutex);
	return ret;
}

int Init_Udhcpd(void)
{
	int  socket_s, err, n = 1;
	struct sockaddr_in serv;
	p_dbg_enter;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	memset(&server_config, 0, sizeof(struct server_config_t));
	memcpy(server_config.arp, p_netif->hwaddr, 6);
	
	dhcp_packet = 0;
	
	server_config.auto_time = FALSE;
	server_config.conflict_time = 3600;
	server_config.decline_time = 3600;
	server_config.start = inet_addr(DHCP_START_IP);
	server_config.end = inet_addr(DHCP_END_IP);
	server_config.lease = LEASE_TIME;
	server_config.max_leases = 10;
	server_config.min_lease = 20;
	server_config.offer_time = 60;
	server_config.siaddr = inet_addr("0.0.0.0");
	server_config.sname = "ZQWL";

	serv.sin_family = AF_INET;
	serv.sin_port = htons(DHCP_SERVER_PORT);	
	serv.sin_addr.s_addr = htons(INADDR_ANY);
	
	mutex_lock(socket_mutex);		
	socket_s = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_s == -1)
	{
		mutex_unlock(socket_mutex);
		p_err("1");
		return -1;
	}
/*
	if (setsockopt(socket_s, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1) {
		mutex_unlock(socket_mutex);
		p_err("2");
		return -1;
	}*/
	if (setsockopt(socket_s, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n)) == -1) {
		mutex_unlock(socket_mutex);
		p_err("3");
		return -1;
	}
	
	err = bind(socket_s, (struct sockaddr*)&serv, sizeof(struct sockaddr_in));
	if(err == -1)
	{
		mutex_unlock(socket_mutex);
		p_err("4");
		return -1;
	}

	server_config.inited = TRUE;
	mutex_unlock(socket_mutex);
	return 0;
}

void Enable_Dhcp_Server(void)
{
	p_dbg_enter;
	if(server_config.inited)
		server_config.open = TRUE;
}

