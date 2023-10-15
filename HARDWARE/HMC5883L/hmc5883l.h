#ifndef __HMC5883L_H
#define __HMC5883L_H

// �Ƿ���У׼HMC5883L���Բ�ģʽ��
// ����У׼HMC5883��ʹ��
// У׼����20�룬��20����������ת�豸��������תһȦ
// У׼�������ʾ��OLED�У���Ҫ�ֶ�������ŵ�HMC5883L_X_OFFEST��HMC5883L_Y_OFFEST��HMC5883L_K_X��HMC5883L_K_Y��
// �Բ�ģʽ�����ͨ���۲�led�Ƿ�һֱָ�������жϵõ���ֵ�Ƿ�׼ȷ��������ϴ�������������Բ�ģʽ
#define HMC5883L_SELFTEST 0

#define HMC5883L_ADDER 0x3c
#define HMC5883L_CONFIGURATION_A  0x00
#define HMC5883L_CONFIGURATION_B  0x01
#define HMC5883L_MODE  0x02
#define HMC5883L_X_MSB 0x03

// ƫ������Ϣ
#define HMC5883L_X_OFFEST 0.076147
#define HMC5883L_Y_OFFEST -0.149541
#define HMC5883L_K_X 4.342629
#define HMC5883L_K_Y 4.257812


// �ҵ��豸B��ƫ����
//#define HMC5883L_X_OFFEST 0.043578
//#define HMC5883L_Y_OFFEST -0.041743
//#define HMC5883L_K_X 4.333996
//#define HMC5883L_K_Y 4.282907


void hmc5883l_init(void);

int16_t hmc5883l_get_angle(void);

#endif
