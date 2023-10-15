#include "led.h"

#define LED_LEN 8

// 指示灯
const uint16_t led_pin_list[LED_LEN] = {GPIO_Pin_5,GPIO_Pin_9,GPIO_Pin_6,GPIO_Pin_12,GPIO_Pin_7,GPIO_Pin_13,GPIO_Pin_8,GPIO_Pin_14};


// 初始化状态指示灯
void led_init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;					//输出模式 推挽输出
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;					//输出速率
	
	//引脚编号
	for(int i=0;i<LED_LEN;++i)
	{
		GPIO_InitStructure.GPIO_Pin|=led_pin_list[i];
	}
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//默认不亮
	for(int i=0;i<LED_LEN;++i)
	{
		GPIO_SetBits(GPIOB,led_pin_list[i]);
	}
}
 


// 根据角度来设置让哪个灯亮
void led_set_light_angle(uint16_t angle)
{
	// 让某一个区间内亮灯
	// 比如LED1的亮灯区间为[338:23)
	uint8_t index = (angle + 22)%360/45;
	
	// 只让某一个灯亮
	for(uint8_t i=0;i<LED_LEN;++i)
	{
		if(index == i)
			GPIO_ResetBits(GPIOB,led_pin_list[i]);
		else
			GPIO_SetBits(GPIOB,led_pin_list[i]);
	}
}
