#ifndef __USART2_H
#define __USART2_H

#include <stdio.h>

#define RX_DATA_SIZE 200

void usart2_init(void);
//void usart2_send_byte(uint8_t byte);
//void usart2_send_array(uint8_t *array, uint16_t length);
//void usart2_send_string(char *string);
//void usart2_send_number(uint32_t number, uint8_t length);
//void usart2_printf(char *format, ...);

uint8_t usart2_get_rx_count(void);
const uint8_t* usart2_get_rx_data(void);

#endif
