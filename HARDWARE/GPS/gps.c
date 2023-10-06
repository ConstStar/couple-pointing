#include "sys.h"

#include <string.h>
#include <stdio.h>

#include "usart2.h"
#include "gps.h"

#include "utils.h"


// �ҵ�gps����
extern char gps_buffer[RX_DATA_SIZE];

// ��ʼ��GPS
void gps_init(void)
{
	usart2_init();
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

// ��ȡ����ֱ�ӵ�λ�úͷ���
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


// ��ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_get_info(GPS* gps)
{
	char gps_buffer_temp[RX_DATA_SIZE];
	char *p=NULL;
	
	strcpy(gps_buffer_temp,gps_buffer);
	gps_buffer[0] = '\0';	// ���
	
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
	
	
	p = gps_buffer_temp;
	
	// ��λ���UTCʱ��
	p = strstr(p, ",");
	sscanf(p+1,"%d",&gps_info_temp.utc_time);
	
	// ��λ���״̬
	p = strstr(p+1, ",");
	
	// γ��
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// ���û�����������
		return -1;	
	sscanf(p+1,"%d.%d",gps_info_temp.n,gps_info_temp.n+1);
	
	// γ�ȷ���
	p = strstr(p+1, ",");
	
	// ����
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// ���û�����������
		return -1;	
	sscanf(p+1,"%d.%d",gps_info_temp.w,gps_info_temp.w+1);
	
	
	// ���ȷ���
	p = strstr(p+1, ",");
	
	// �Եغ���
	p = strstr(p+1, ",");
	
	// �Եغ���
	p = strstr(p+1, ",");
	if(*(p+1)==',')	// ���û�����������
		return -1;	
	sscanf(p+1,"%d",&gps_info_temp.angle);
	
	// UTC����
	p = strstr(p+1, ",");
	sscanf(p+1,"%d",&gps_info_temp.utc_date);
	
//			// ��ƫ��
//			p = strtok(NULL, ",");
//			
//			// ��ƫ�Ƿ���
//			p = strtok(NULL, ",");
	
	
	// ���λ����ϢΪ�ջ��߲����� �����
	if(gps_info_temp.n[0]<=0||gps_info_temp.n[1]<=0||gps_info_temp.w[0]<=0||gps_info_temp.w[1]<=0)
		return -1;
	
	*(gps) = gps_info_temp;
	
	return 0;
}


