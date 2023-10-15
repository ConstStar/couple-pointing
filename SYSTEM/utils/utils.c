#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "usart2.h"

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

    if(w>w_ta)
        distance1=0-distance1;

    if(n>n_ta)
        distance2=0-distance2;

    return atan2(distance2,distance1)* 180 / PI;
}

// ����Ϣ��������ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_from_message(char* message,GPS *gps)
{
	char gps_buffer_temp[RX_DATA_SIZE];	//��ʱGPS��Ϣ
	char *p=NULL;
	
	if(message[0] == '\0')
		return -1;
	
	strcpy(gps_buffer_temp,message);
	message[0] = '\0';	// ���
	
	// �ָ�У������ ��У���
	p = strtok(gps_buffer_temp, "*");
	if(p == NULL)
		return -1;
	
	GPS gps_info_temp;	// ��ʱ�����λ����Ϣ
	gps_info_temp.n[0] = 0;
	gps_info_temp.n[1] = 0;
	gps_info_temp.w[0] = 0;
	gps_info_temp.w[1] = 0;
	gps_info_temp.utc_date = 0;
	gps_info_temp.utc_time = 0;
	gps_info_temp.angle = 0;

	// У�鲻ͨ��
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
	
	// ���λ����Ϣ������ �����
	if(gps_info_temp.n[0]<=0||gps_info_temp.n[1]<=0||gps_info_temp.w[0]<=0||gps_info_temp.w[1]<=0)
		return -1;
	
	*(gps) = gps_info_temp;
	
	return 0;
}

// ��ȡGPS����ֱ�ӵ�λ�úͷ���
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle)
{
	// �ҵľ�γ����Ϣ
	float gps_n;
	float gps_w;
	
	// �Է��ľ�γ����Ϣ
	float gps_n_ta;
	float gps_w_ta;
	
	gps_n = gps_a.n[0]/100+(gps_a.n[0]%100+gps_a.n[1]/10000.0)/60.0;
	gps_w = gps_a.w[0]/100+(gps_a.w[0]%100+gps_a.w[1]/10000.0)/60.0;
	
	gps_n_ta = gps_b.n[0]/100+(gps_b.n[0]%100+gps_b.n[1]/10000.0)/60.0;
	gps_w_ta = gps_b.w[0]/100+(gps_b.w[0]%100+gps_b.w[1]/10000.0)/60.0;
	
	(*distance) = get_distance(gps_n,gps_w,gps_n_ta,gps_w_ta);
	(*angle) = get_angle(gps_n,gps_w,gps_n_ta,gps_w_ta);
}

// ��GPS��Ϣת����Ϣ����
void gps_to_message(GPS gps,char *data_buf)
{
	char data_code[3];
	sprintf(data_buf,"%x,%x,%x,%x,%x,%x",gps.utc_time,gps.n[0],gps.n[1],gps.w[0],gps.w[1],gps.utc_date);
	get_check_sum(data_buf,data_code);
	strcat(data_buf,"*");
	strcat(data_buf,data_code);
}


