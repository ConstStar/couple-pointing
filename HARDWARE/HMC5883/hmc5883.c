#include "sys.h"

#include <math.h>

#include "hmc5883.h"
#include "iic.h"

// 写入单字节数据
void hmc5883_write(uint8_t sem_addr,uint8_t data)
{
	iic_send_data(0x3C,sem_addr,&data,1);
}


// 连续读出HMC5883内部角度数据，地址范围0x3~0x5
void hmc5883_read(uint8_t* buf)
{   
	iic_read_data(0x3C,0x03,buf,6);
}

// 初始化
void hmc5883_init(void)
{
	iic_init();
    hmc5883_write(0x02,0x00);  //
}

// 获取XY轴角度
double hmc5883_get_angle(void)
{
	uint8_t buf[6];
	double x,y;
	
	hmc5883_read(buf);      //连续读出数据，存储在BUF中

	x=buf[0] << 8 | buf[1]; //Combine MSB and LSB of X Data output register  最高有效位
	//Z=BUF[2] << 8 | BUF[3]; //Combine MSB and LSB of Z Data output register
	y=buf[4] << 8 | buf[5]; //Combine MSB and LSB of Y Data output register

	if(x>0x7fff)x-=0xffff;	  
	if(y>0x7fff)y-=0xffff;
	//if(Z>0x7fff)Z-=0xffff;

	return atan2((double)y,(double)x) * (180 / 3.14159265) + 180; //计算XY平面角度
//	Angle_XZ= atan2((double)Z,(double)X) * (180 / 3.14159265) + 180; //计算XZ平面角度
//	Angle_YZ= atan2((double)Z,(double)Y) * (180 / 3.14159265) + 180; //计算YZ平面角度
}
