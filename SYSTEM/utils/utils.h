#ifndef __UTILS_H
#define __UTILS_H	

#include "gps.h"

// ����У���
void get_check_sum(const char *str,char *code);

// ����У����Ƿ���ȷ
// У����ȷ���� 0   У����󷵻� -1
int check_sum(const char *str,const char *code);

// UTCת�ɱ���ʱ��
void uct_to_beijing(uint8_t *year,uint8_t *month,uint8_t *day,uint8_t *hour);

// ��ȡ���ؼ�ľ��� ��λkm
double get_distance(double n, double w, double n_ta, double w_ta);

// ��ȡ����ƫ��
double get_angle(double n, double w, double n_ta, double w_ta);

// ����Ϣ��������ȡGPS��Ϣ
// ����ֵ 0:��ȡ�ɹ� -1:��ȡʧ��
int gps_from_message(char* message,GPS *gps);

// ��ȡGPS����ֱ�ӵ�λ�úͷ���
void gps_get_distance_and_angle(GPS gps_a,GPS gps_b,double* distance,double* angle);

// ��GPS��Ϣת����Ϣ����
void gps_to_message(GPS gps,char *data_buf);

#endif  
