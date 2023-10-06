#ifndef __HMC5883_H
#define __HMC5883_H	

void hmc5883_write(uint8_t sem_addr,uint8_t data);

void hmc5883_read(uint8_t* buf);

void hmc5883_init(void);

double hmc5883_get_angle(void);

#endif
