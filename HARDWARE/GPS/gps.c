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


