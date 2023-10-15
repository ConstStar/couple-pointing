#ifndef __HMC5883L_H
#define __HMC5883L_H

// 是否开启校准HMC5883L（自测模式）
// 仅在校准HMC5883下使用
// 校准过程20秒，在20秒内慢速旋转设备，至少旋转一圈
// 校准结果将显示到OLED中，需要手动将结果放到HMC5883L_X_OFFEST、HMC5883L_Y_OFFEST、HMC5883L_K_X、HMC5883L_K_Y中
// 自测模式后可以通过观察led是否一直指向南来判断得到的值是否准确，如果误差较大可以重新启动自测模式
#define HMC5883L_SELFTEST 0

#define HMC5883L_ADDER 0x3c
#define HMC5883L_CONFIGURATION_A  0x00
#define HMC5883L_CONFIGURATION_B  0x01
#define HMC5883L_MODE  0x02
#define HMC5883L_X_MSB 0x03

// 偏移量信息
#define HMC5883L_X_OFFEST 0.076147
#define HMC5883L_Y_OFFEST -0.149541
#define HMC5883L_K_X 4.342629
#define HMC5883L_K_Y 4.257812


// 我的设备B的偏移量
//#define HMC5883L_X_OFFEST 0.043578
//#define HMC5883L_Y_OFFEST -0.041743
//#define HMC5883L_K_X 4.333996
//#define HMC5883L_K_Y 4.282907


void hmc5883l_init(void);

int16_t hmc5883l_get_angle(void);

#endif
