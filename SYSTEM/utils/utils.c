#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utils.h"

const double PI = 3.141592654;
const double EARTH_RADIUS = 6378.137; 	//����뾶

// ����У���
void get_check_sum(const char *str,char *code)
{
	int i=0;
	if(str == NULL || code == NULL)
		return;

	unsigned char data=0;
	//����У���
	for(i=0;str[i]!='\0';++i)
	{
		data^=str[i];
	}
   
	sprintf(code,"%02X",data);
}

// ����У����Ƿ���ȷ
// У����ȷ���� 0   У����󷵻� -1
int check_sum(const char *str,const char *code)
{
	char buf[5];
	int i=0;
	if(str == NULL || code == NULL)
		return -1;

	unsigned char data=0;
	//����У���
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


// UTCת�ɱ���ʱ��
void uct_to_beijing(uint8_t *year,uint8_t *month,uint8_t *day,uint8_t *hour)
{
	(*hour)+=8;
	
	// �ж�ʱ���Ƿ�Խ��
	if((*hour)+8>=24)
	{
		(*hour)%=24;
		(*day)++;
	}

	// �ж������Ƿ�Խ��
	if((*month) == 2)
	{
		
		// �ж�����ƽ��
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

	// �ж������Ƿ�Խ��
	if((*month)>12)
	{
		(*year)++;
	}
	
}


double rad(double d)
{
    return d * PI / 180.0;
}



// ��ȡ���ؼ�ľ��� ��λkm
double get_distance(double n, double w, double n_ta, double w_ta)
{
    double a = rad(n) - rad(n_ta);
    double b = rad(w) - rad(w_ta);
    double s = 2 * asin(sqrt(pow(sin(a/2), 2) + cos(rad(n))*cos(rad(n_ta))*pow(sin(b/2),2)));
    s = s * EARTH_RADIUS;
    return s;
}

// ��ȡ����ƫ��
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
