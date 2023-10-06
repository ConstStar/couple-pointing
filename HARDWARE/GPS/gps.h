#ifndef __GPS_H
#define __GPS_H

typedef struct 
{
	uint32_t n[2];			// 维度
	uint32_t w[2];			// 经度
	uint32_t utc_time;		// UTC时间
	uint32_t utc_date;		// UTC日期
	uint32_t angle;			// 旋转角度
} GPS;

// 初始化GPS
void gps_init(void);

// 从消息内容中提取GPS信息
// 返回值 0:提取成功 -1:提取失败
int gps_from_message(char* message,GPS *gps);

// 获取两点直接的位置和方向
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle);

// 将GPS信息转成消息内容
void gps_to_message(GPS gps,char *data_buf);

// 获取GPS信息
// 返回值 0:提取成功 -1:提取失败
int gps_get_info(GPS* gps);

#endif
