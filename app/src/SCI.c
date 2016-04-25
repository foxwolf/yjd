
#include "main.H"
/***********************************************************************
�ļ����ƣ�SCI.C
��    �ܣ���ɶ�usart1��usart2�Ĳ���
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺��������ͨ���ж������ض����ַ���ȷ��һ֡�����Ƿ�����ġ�
USART1 ʱ�� : RCC_APB2PeriphClockCmd
USART1~6 ʱ�� :RCC_APB1PeriphClockCmd
***********************************************************************/

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);
	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}
	return ch;
}
volatile unsigned char RS232_REC_Flag = 0;
volatile unsigned char RS485_REC_Flag = 0;
volatile unsigned char RS232_buff[RS232_REC_BUFF_SIZE];//���ڽ�������
volatile unsigned char RS485_buff[RS485_REC_BUFF_SIZE];//���ڽ�������
volatile unsigned int RS232_rec_counter = 0;//����RS232���ռ���
volatile unsigned int RS485_rec_counter = 0;//����RS485���ռ���

void USART_Configuration(void)
{ 
	
	GPIO_InitTypeDef GPIO_InitStructure;//����GPIO_InitTypeDef���͵Ľṹ���ԱGPIO_InitStructure

	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	//ʹ����Ҫ�õ���GPIO�ܽ�ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD, ENABLE);
	//ʹ��USART1 ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	///��λ����1
	USART_DeInit(USART1);
	
	USART_StructInit(&USART_InitStructure);//����Ĭ��USART����
	USART_ClockStructInit(&USART_ClockInitStruct);//����Ĭ��USART����        
	//���ô���1�Ĺܽ� PA8 USART1_EN PA9 USART1_TX PA10 USART1_RX    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);        //�ܽ�PA9����ΪUSART1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;        
	GPIO_Init(GPIOA, &GPIO_InitStructure);                                                                                                                 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	
	USART_ClockInit(USART1,&USART_ClockInitStruct);


	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure); 

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);        ///////�����ж�ʹ��
	USART_ClearITPendingBit(USART1, USART_IT_TC);//����ж�TCλ
	USART_Cmd(USART1,ENABLE);//���ʹ�ܴ��
}

/***********************************************************************
�������ƣ�void USART1_IRQHandler(void) 
��    �ܣ����SCI�����ݵĽ��գ�������ʶ
���������
���������
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    ��  RS232�õ���USART1.
***********************************************************************/
void USART1_IRQHandler(void)  
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{	
		RS232_buff[RS232_rec_counter] = USART1->DR;//
		RS232_rec_counter ++;
/********��RS232_END_FLAG1��RS232_END_FLAG2������ַ���Ϊһ֡���ݵĽ�����ʶ************/
		if(RS232_rec_counter >= 2)	//ֻ�н��յ�2���������ϲ����ж�
		{
			if(RS232_buff[RS232_rec_counter - 2] == RS232_END_FLAG1 && RS232_buff[RS232_rec_counter - 1] == RS232_END_FLAG2) 	//֡��ʼ��־   
			{
				RS232_REC_Flag = 1;
			}
		}
		if(RS232_rec_counter > RS232_REC_BUFF_SIZE)//�������ջ�������С
		{
			RS232_rec_counter = 0;
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) 
	{
        USART_ClearITPendingBit(USART1, USART_IT_TXE);           /* Clear the USART transmit interrupt                  */
    }	
}

/***********************************************************************
�������ƣ�RS232_Send_Data(unsigned char *send_buff,unsigned int length)
��    �ܣ�RS232�����ַ���
���������send_buff:�����͵�����ָ�룻length�����͵����ݳ��ȣ��ַ�������
���������
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺
***********************************************************************/
void RS232_Send_Data(unsigned char *send_buff,unsigned int length)
{
 	unsigned int i = 0;
	for(i = 0;i < length;i ++)
	{			
		USART1->DR = send_buff[i];
		while((USART1->SR&0X40)==0);	
	}	
}

/***********************************************************************
�������ƣ�void RS485_Delay(uint32_t nCount)
��    �ܣ�RS485�շ���ʱ����
���������
���������
��дʱ�䣺2012.11.22
�� д �ˣ�
ע    �⣺
***********************************************************************/
// static void RS485_Delay(uint32_t nCount)
// { 
//   while(nCount > 0)
//   { 
//   	  nCount --;   
//   }
// }
