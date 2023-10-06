#ifndef __GPS_H
#define __GPS_H

typedef struct 
{
	uint32_t n[2];			// ά��
	uint32_t w[2];			// ����
	uint32_t utc_time;		// UTCʱ��
	uint32_t utc_date;		// UTC����
	uint32_t angle;			// ��ת�Ƕ�
} GPS;

// ��ʼ��GPS
void gps_init(void);

// ����Ϣ��������ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_from_message(char* message,GPS *gps);

// ��ȡ����ֱ�ӵ�λ�úͷ���
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle);

// ��GPS��Ϣת����Ϣ����
void gps_to_message(GPS gps,char *data_buf);

// ��ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_get_info(GPS* gps);

#endif
