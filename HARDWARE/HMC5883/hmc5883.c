#include "sys.h"

#include <math.h>

#include "hmc5883.h"
#include "iic.h"

// д�뵥�ֽ�����
void hmc5883_write(uint8_t sem_addr,uint8_t data)
{
	iic_send_data(0x3C,sem_addr,&data,1);
}


// ��������HMC5883�ڲ��Ƕ����ݣ���ַ��Χ0x3~0x5
void hmc5883_read(uint8_t* buf)
{   
	iic_read_data(0x3C,0x03,buf,6);
}

// ��ʼ��
void hmc5883_init(void)
{
	iic_init();
    hmc5883_write(0x02,0x00);  //
}

// ��ȡXY��Ƕ�
double hmc5883_get_angle(void)
{
	uint8_t buf[6];
	double x,y;
	
	hmc5883_read(buf);      //�����������ݣ��洢��BUF��

	x=buf[0] << 8 | buf[1]; //Combine MSB and LSB of X Data output register  �����Чλ
	//Z=BUF[2] << 8 | BUF[3]; //Combine MSB and LSB of Z Data output register
	y=buf[4] << 8 | buf[5]; //Combine MSB and LSB of Y Data output register

	if(x>0x7fff)x-=0xffff;	  
	if(y>0x7fff)y-=0xffff;
	//if(Z>0x7fff)Z-=0xffff;

	return atan2((double)y,(double)x) * (180 / 3.14159265) + 180; //����XYƽ��Ƕ�
//	Angle_XZ= atan2((double)Z,(double)X) * (180 / 3.14159265) + 180; //����XZƽ��Ƕ�
//	Angle_YZ= atan2((double)Z,(double)Y) * (180 / 3.14159265) + 180; //����YZƽ��Ƕ�
}
