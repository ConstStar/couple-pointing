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


