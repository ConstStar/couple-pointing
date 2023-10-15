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


// ��ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_get_info(GPS* gps);

#endif
