#include "sys.h"

#include <math.h>
#include <stdio.h>
#include <delay.h>

#include "hmc5883l.h"
#include "iic.h"

#if HMC5883L_SELFTEST

#include "oled.h"
#include "led.h"

#endif

void hmc5883l_rawread(float *GaX, float *GaY){
		
	uint8_t data[6];
	iic_read_data(HMC5883L_ADDER, 0x03,data,6);
	
	int16_t dxra,dyra;

	dxra = (data[0] << 8) | data[1]; 
	*GaX = (float)dxra /1090;

	dyra = (data[4] << 8) | data[5]; 
	*GaY = (float)dyra /1090;
	
}


#if HMC5883L_SELFTEST

// 调用自测模式
void hmc5883l_selftest(void)
{
	led_init();
	
	char temp_format[20];
	uint8_t i=0;

	float ga_x,ga_y,ga_x_max=0,ga_x_min=0,ga_y_max=0,ga_y_min=0;
	
	uint8_t data = 0x70;
	iic_send_data(HMC5883L_ADDER,HMC5883L_CONFIGURATION_A, &data,1);
	
	data = 0x20;
	iic_send_data(HMC5883L_ADDER,HMC5883L_CONFIGURATION_B, &data,1);
	
	data = 0x00;
	iic_send_data(HMC5883L_ADDER,HMC5883L_CONFIGURATION_B, &data,1);
	
	oled_clear();
	oled_show_string(0,0,"start",16);
	delay_ms(5000);
	
	oled_show_char(32,2,'%',16);
	
	while(i != 100)
	{
		hmc5883l_rawread(&ga_x, &ga_y);
	
		// 统计xy的最大值和最小值
		ga_x_max = ga_x_max < ga_x? ga_x:ga_x_max;
		ga_x_min = ga_x_min > ga_x? ga_x:ga_x_min;
		ga_y_max = ga_y_max < ga_y? ga_y:ga_y_max;
		ga_y_min = ga_y_min > ga_y? ga_y:ga_y_min;

		
		delay_ms(200);
		
		i++;
		
		// 显示进度
		oled_show_num(0,2,i,3,16);
	}
	
	oled_clear();
	
	
	// 显示结果
	float x_offest = (ga_x_max+ga_x_min)/2;
	float y_offest = (ga_y_max+ga_y_min)/2;
	float k_x = 2/(ga_x_max-ga_x_min);
	float k_y = 2/(ga_y_max-ga_y_min);
	
	// X_OFFEST
	sprintf(temp_format,"X_OFFEST");
	oled_show_string(0,0,temp_format,12);
	sprintf(temp_format,"      %f",x_offest);
	oled_show_string(0,1,temp_format,12);
	
	// Y_OFFEST
	sprintf(temp_format,"Y_OFFEST");
	oled_show_string(0,2,temp_format,12);
	sprintf(temp_format,"      %f",y_offest);
	oled_show_string(0,3,temp_format,12);
	
	// K_X
	sprintf(temp_format,"K_X %f",k_x);
	oled_show_string(0,4,temp_format,12);
	
	// K_Y
	sprintf(temp_format,"K_Y %f",k_y);
	oled_show_string(0,5,temp_format,12);
	
	
	// 旋转测试
	while(1)
	{
		// 根据当前偏移量获取角度
		float raw_ga_x,raw_ga_y;
		int16_t angle;

		hmc5883l_rawread(&raw_ga_x,&raw_ga_y);

		float ga_x = (raw_ga_x -  x_offest) * k_x;
		float ga_y = (raw_ga_y - y_offest) * k_y;
		
			
		if((ga_x > 0)&&(ga_y > 0)) angle = atan(ga_y/ga_x)*57;
		else if((ga_x > 0)&&(ga_y < 0)) angle = 360+atan(ga_y/ga_x)*57;
		else if((ga_x == 0)&&(ga_y > 0)) angle = 90;
		else if((ga_x == 0)&&(ga_y < 0)) angle = 270;
		else if(ga_x < 0) angle = 180+atan(ga_y/ga_x)*57;
		
		// 方向灯
		led_set_light_angle(angle);
		
		// 显示当前的角度
		oled_show_num(0,6,angle,3,12);
		delay_ms(200);
	}
}
#endif

// 初始化hmc5883l
void hmc5883l_init()
{
	
	iic_init();
	
	
	uint8_t data = 0x00;
	iic_send_data(HMC5883L_ADDER,HMC5883L_MODE, &data,1);
	
#if HMC5883L_SELFTEST
	hmc5883l_selftest();
#endif
}

// 获取XY轴角度
int16_t hmc5883l_get_angle(void)
{
	float raw_ga_x,raw_ga_y;

	int16_t angle;

	hmc5883l_rawread(&raw_ga_x,&raw_ga_y);

	float GaX = (raw_ga_x -  HMC5883L_X_OFFEST) * HMC5883L_K_X;
	float GaY = (raw_ga_y - HMC5883L_Y_OFFEST) * HMC5883L_K_Y;
	
		
	if((GaX > 0)&&(GaY > 0)) angle = atan(GaY/GaX)*57;
	else if((GaX > 0)&&(GaY < 0)) angle = 360+atan(GaY/GaX)*57;
	else if((GaX == 0)&&(GaY > 0)) angle = 90;
	else if((GaX == 0)&&(GaY < 0)) angle = 270;
	else if(GaX < 0) angle = 180+atan(GaY/GaX)*57;
	
	return angle;
}
