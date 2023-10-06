#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utils.h"

const double PI = 3.141592654;
const double EARTH_RADIUS = 6378.137; 	//地球半径

// 计算校验和
void get_check_sum(const char *str,char *code)
{
	int i=0;
	if(str == NULL || code == NULL)
		return;

	unsigned char data=0;
	//计算校验和
	for(i=0;str[i]!='\0';++i)
	{
		data^=str[i];
	}
   
	sprintf(code,"%02X",data);
}

// 计算校验和是否正确
// 校验正确返回 0   校验错误返回 -1
int check_sum(const char *str,const char *code)
{
	char buf[5];
	int i=0;
	if(str == NULL || code == NULL)
		return -1;

	unsigned char data=0;
	//计算校验和
	for(i=0;str[i]!='\0';++i)
	{
		data^=str[i];
	}
   
	sprintf(buf,"%02X",data);
	if(strcmp(buf,code)==0)
		return 0;
	
	sprintf(buf,"%02x",data);
	if(strcmp(buf,code)==0)
		return 0;
  
    return -1;
}


// UTC转成北京时间
void uct_to_beijing(uint8_t *year,uint8_t *month,uint8_t *day,uint8_t *hour)
{
	(*hour)+=8;
	
	// 判断时间是否越过
	if((*hour)+8>=24)
	{
		(*hour)%=24;
		(*day)++;
	}

	// 判断天数是否越过
	if((*month) == 2)
	{
		
		// 判断闰年平年
		if((*year)%4==0&&(*day)>29)
		{
			(*day) = 1;
			(*month)++;
		}
		else if((*year)%4!=0&&(*day)>28)
		{
			(*day) = 1;
			(*month)++;
		}
	}
	else if((*month)==1||(*month)==3||(*month)==5||(*month)==7||(*month)==8||(*month)==10||(*month)==12)
	{
		if((*day)>31)
		{
			(*day) = 1;
			(*month)++;
		}
	}
	else
	{
		if((*day)>30)
		{
			(*day) = 1;
			(*month)++;
		}
	}

	// 判断月数是否越过
	if((*month)>12)
	{
		(*year)++;
	}
	
}


double rad(double d)
{
    return d * PI / 180.0;
}



// 获取两地间的距离 单位km
double get_distance(double n, double w, double n_ta, double w_ta)
{
    double a = rad(n) - rad(n_ta);
    double b = rad(w) - rad(w_ta);
    double s = 2 * asin(sqrt(pow(sin(a/2), 2) + cos(rad(n))*cos(rad(n_ta))*pow(sin(b/2),2)));
    s = s * EARTH_RADIUS;
    return s;
}

// 获取两地偏角
double get_angle(double n, double w, double n_ta, double w_ta)
{
	double distance1 = get_distance(n,w,n,w_ta);
	double distance2 = get_distance(n,w,n_ta,w);
	
	if(w<w_ta)
		distance1=0-distance1;
	
	if(n<n_ta)
		distance2=0-distance2;
	
	return atan2(distance2,distance1)* 180 / PI;
}
