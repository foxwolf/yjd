#include "exti.h"
#include "debug.h"

#define DEBUG
extern int dbg_level;

void EXTI9_5_IRQHandler(void)
{
	YJ_U16	t0;
	int i;

	if (EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		t0 = parallel_read();
		if (!(t0 & 0x100))
		{
			pblock->u8Buf[pblock->len++] = t0;
			pblock->fst = os_time_get();
		}
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	//OSSemPost(g_sem_flag);
}

void exti_init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;

	p_info("%s comming", __FUNCTION__);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource8);

	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//����

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);//����
}

