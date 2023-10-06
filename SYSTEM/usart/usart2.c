#include "stm32f10x.h"                  // Device header
#include "sys.h"
#include "usart2.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

char gps_buffer[RX_DATA_SIZE];

uint8_t usart2_buf[RX_DATA_SIZE];
uint8_t usart2_rx_count = 0;

void usart2_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2, ENABLE);
}

//void usart2_send_byte(uint8_t byte)
//{
//	USART_SendData(USART2, byte);
//	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
//}

//void usart2_send_array(uint8_t *array, uint16_t length)
//{
//	uint16_t i;
//	for (i = 0; i < length; i ++)
//	{
//		usart2_send_byte(array[i]);
//	}
//}

//void usart2_send_string(char *string)
//{
//	uint8_t i;
//	for (i = 0; string[i] != '\0'; i ++)
//	{
//		usart2_send_byte(string[i]);
//	}
//}

uint32_t usart2_pow(uint32_t x, uint32_t y)
{
	uint32_t result = 1;
	while (y --)
	{
		result *= x;
	}
	return result;
}

//void usart2_send_number(uint32_t number, uint8_t length)
//{
//	uint8_t i;
//	for (i = 0; i < length; i ++)
//	{
//		usart2_send_byte(number / usart2_pow(10, length - i - 1) % 10 + '0');
//	}
//}

//void usart2_printf(char *format, ...)
//{
//	char String[100];
//	va_list arg;
//	va_start(arg, format);
//	vsprintf(String, format, arg);
//	va_end(arg);
//	usart2_send_string(String);
//}

uint8_t usart2_get_rx_count(void)
{
	return usart2_rx_count;
}

const uint8_t* usart2_get_rx_data(void)
{
	return usart2_buf;
}


void USART2_IRQHandler(void)
{
	uint8_t recv_data = 0;
	
#ifdef SYSTEM_SUPPORT_OS	 	
	OSIntEnter();    
#endif
	
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		recv_data = USART_ReceiveData(USART2);
		
		if(usart2_rx_count >= RX_DATA_SIZE)
		{
			usart2_rx_count = 0;
		}
		
		
		
		if(recv_data == '\r')
		{
			// 忽略
		}
		else if(recv_data == '$')
		{
			usart2_rx_count = 0;	
		}
		else if(recv_data == '\n')	
		{
			// 如果收到的是"GPRMC/GNRMC" 就保存下来
			if(usart2_buf[3] == 'M' && usart2_buf[4] == 'C')
			{
				memcpy(gps_buffer, usart2_buf, usart2_rx_count); 
				gps_buffer[usart2_rx_count] = '\0';
			}
		}
		else
		{
			//  如果指令长度符合条件就记录下来，否则忽略本次内容 因为需要多存一个结束符所以要-1
			if(usart2_rx_count<RX_DATA_SIZE-1)
			{
				usart2_buf[usart2_rx_count] = recv_data;	
				usart2_rx_count++;
			}				
		}
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	
#ifdef SYSTEM_SUPPORT_OS	 
	OSIntExit();  											 
#endif
	
}
