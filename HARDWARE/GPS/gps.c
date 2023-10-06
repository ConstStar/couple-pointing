#include "sys.h"

#include <string.h>
#include <stdio.h>

#include "usart2.h"
#include "gps.h"

#include "utils.h"


// 我的gps数据
extern char gps_buffer[RX_DATA_SIZE];

// 初始化GPS
void gps_init(void)
{
	usart2_init();
}

// 从消息内容中提取GPS信息
// 返回值 0:提取成功 -1:提取失败
int gps_from_message(char* message,GPS *gps)
{
	char gps_buffer_temp[RX_DATA_SIZE];	//临时GPS信息
	char *p=NULL;
	
	if(message[0] == '\0')
		return -1;
	
	strcpy(gps_buffer_temp,message);
	message[0] = '\0';	// 清空
	
	// 分割校验内容 和校验和
	p = strtok(gps_buffer_temp, "*");
	if(p == NULL)
		return -1;
	
	GPS gps_info_temp;	// 临时储存的位置信息
	gps_info_temp.n[0] = 0;
	gps_info_temp.n[1] = 0;
	gps_info_temp.w[0] = 0;
	gps_info_temp.w[1] = 0;
	gps_info_temp.utc_date = 0;
	gps_info_temp.utc_time = 0;
	gps_info_temp.angle = 0;

	// 校验不通过
	if(check_sum(p,strtok(NULL,"*")) != 0)
		return -1;

	p = strtok(gps_buffer_temp, ",");
	if(p == NULL)
		return -1;
	
	sscanf(p,"%x",&gps_info_temp.utc_time);
	
	p = strtok(NULL, ",");
	sscanf(p,"%x",&gps_info_temp.n[0]);
	
	p = strtok(NULL, ",");
	sscanf(p,"%x",&gps_info_temp.n[1]);
	
	p = strtok(NULL, ",");
	sscanf(p,"%x",&gps_info_temp.w[0]);
	
	p = strtok(NULL, ",");
	sscanf(p,"%x",&gps_info_temp.w[1]);
	
	p = strtok(NULL, ",");
	sscanf(p,"%x",&gps_info_temp.utc_date);
	
	// 如果位置信息不合理 则忽略
	if(gps_info_temp.n[0]<=0||gps_info_temp.n[1]<=0||gps_info_temp.w[0]<=0||gps_info_temp.w[1]<=0)
		return -1;
	
	*(gps) = gps_info_temp;
	
	return 0;
}

// 获取两点直接的位置和方向
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle)
{
	// 我的经纬度信息
	float gps_n;
	float gps_w;
	
	// 对方的经纬度信息
	float gps_n_ta;
	float gps_w_ta;
	
	gps_n = gps_a.n[0]/100+(gps_a.n[0]%100+gps_a.n[1]/10000.0)/60.0;
	gps_w = gps_a.w[0]/100+(gps_a.w[0]%100+gps_a.w[1]/10000.0)/60.0;
	
	gps_n_ta = gps_b.n[0]/100+(gps_b.n[0]%100+gps_b.n[1]/10000.0)/60.0;
	gps_w_ta = gps_b.w[0]/100+(gps_b.w[0]%100+gps_b.w[1]/10000.0)/60.0;
	
	(*distance) = get_distance(gps_n,gps_w,gps_n_ta,gps_w_ta);
	(*angle) = get_angle(gps_n,gps_w,gps_n_ta,gps_w_ta);
}

// 将GPS信息转成消息内容
void gps_to_message(GPS gps,char *data_buf)
{
	char data_code[3];
	sprintf(data_buf,"%x,%x,%x,%x,%x,%x",gps.utc_time,gps.n[0],gps.n[1],gps.w[0],gps.w[1],gps.utc_date);
	get_check_sum(data_buf,data_code);
	strcat(data_buf,"*");
	strcat(data_buf,data_code);
}


// 获取GPS信息
// 返回值 0:提取成功 -1:提取失败
int gps_get_info(GPS* gps)
{
	char gps_buffer_temp[RX_DATA_SIZE];
	char *p=NULL;
	
	strcpy(gps_buffer_temp,gps_buffer);
	gps_buffer[0] = '\0';	// 清空
	
	// 分割校验内容 和校验和
	p = strtok(gps_buffer_temp, "*");
	if(p == NULL)
		return -1;
	
	GPS gps_info_temp;	// 临时储存的位置信息
	gps_info_temp.n[0] = 0;
	gps_info_temp.n[1] = 0;
	gps_info_temp.w[0] = 0;
	gps_info_temp.w[1] = 0;
	gps_info_temp.utc_date = 0;
	gps_info_temp.utc_time = 0;
	gps_info_temp.angle = 0;

	// 校验不通过
	if(check_sum(p,strtok(NULL,"*")) != 0)
		return -1;
	
	
	p = gps_buffer_temp;
	
	// 定位点的UTC时间
	p = strstr(p, ",");
	sscanf(p+1,"%d",&gps_info_temp.utc_time);
	
	// 定位点的状态
	p = strstr(p+1, ",");
	
	// 纬度
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// 如果没有内容则忽略
		return -1;	
	sscanf(p+1,"%d.%d",gps_info_temp.n,gps_info_temp.n+1);
	
	// 纬度方向
	p = strstr(p+1, ",");
	
	// 经度
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// 如果没有内容则忽略
		return -1;	
	sscanf(p+1,"%d.%d",gps_info_temp.w,gps_info_temp.w+1);
	
	
	// 经度方向
	p = strstr(p+1, ",");
	
	// 对地航速
	p = strstr(p+1, ",");
	
	// 对地航向
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// 如果没有内容则忽略
		return -1;	
	sscanf(p+1,"%d",&gps_info_temp.angle);
	
	// UTC日期
	p = strstr(p+1, ",");
	sscanf(p+1,"%d",&gps_info_temp.utc_date);
	
//			// 磁偏角
//			p = strtok(NULL, ",");
//			
//			// 磁偏角方向
//			p = strtok(NULL, ",");
	
	
	// 如果位置信息为空或者不合理 则忽略
	if(gps_info_temp.n[0]<=0||gps_info_temp.n[1]<=0||gps_info_temp.w[0]<=0||gps_info_temp.w[1]<=0)
		return -1;
	
	*(gps) = gps_info_temp;
	
	return 0;
}


