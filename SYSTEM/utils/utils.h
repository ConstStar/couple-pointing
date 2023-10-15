#ifndef __UTILS_H
#define __UTILS_H	

#include "gps.h"

// 计算校验和
void get_check_sum(const char *str,char *code);

// 计算校验和是否正确
// 校验正确返回 0   校验错误返回 -1
int check_sum(const char *str,const char *code);

// UTC转成北京时间
void uct_to_beijing(uint8_t *year,uint8_t *month,uint8_t *day,uint8_t *hour);

// 获取两地间的距离 单位km
double get_distance(double n, double w, double n_ta, double w_ta);

// 获取两地偏角
double get_angle(double n, double w, double n_ta, double w_ta);

// 从消息内容中提取GPS信息
// 返回值 0:提取成功 -1:提取失败
int gps_from_message(char* message,GPS *gps);

// 获取GPS两点直接的位置和方向
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle);

// 将GPS信息转成消息内容
void gps_to_message(GPS gps,char *data_buf);

#endif  
