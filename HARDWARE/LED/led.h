#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

// 初始化状态指示灯
void led_init(void);

// 根据角度来设置让哪个灯亮
void led_set_light_angle(uint16_t angle);
		 				    
#endif
