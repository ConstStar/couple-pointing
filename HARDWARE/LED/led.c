#include "led.h"

#define LED_LEN 8

// ָʾ��
const uint16_t led_pin_list[LED_LEN] = {GPIO_Pin_5,GPIO_Pin_9,GPIO_Pin_6,GPIO_Pin_12,GPIO_Pin_7,GPIO_Pin_13,GPIO_Pin_8,GPIO_Pin_14};


// ��ʼ��״ָ̬ʾ��
void led_init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;					//���ģʽ �������
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;					//�������
	
	//���ű��
	for(int i=0;i<LED_LEN;++i)
	{
		GPIO_InitStructure.GPIO_Pin|=led_pin_list[i];
	}
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//Ĭ�ϲ���
	for(int i=0;i<LED_LEN;++i)
	{
		GPIO_SetBits(GPIOB,led_pin_list[i]);
	}
}
 


// ���ݽǶ����������ĸ�����
void led_set_light_angle(uint16_t angle)
{
	// ��ĳһ������������
	// ����LED1����������Ϊ[338:23)
	uint8_t index = (angle + 22)%360/45;
	
	// ֻ��ĳһ������
	for(uint8_t i=0;i<LED_LEN;++i)
	{
		if(index == i)
			GPIO_ResetBits(GPIOB,led_pin_list[i]);
		else
			GPIO_SetBits(GPIOB,led_pin_list[i]);
	}
}
