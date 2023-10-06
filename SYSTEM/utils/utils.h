#ifndef __UTILS_H
#define __UTILS_H	

// 计算校验和
void get_check_sum(const char *str,char *code);

// 计算校验和是否正确
// 校验正确返回 0   校验错误返回 -1
int check_sum(const char *str,const char *code);

// UTC转成北京时间
void uct_to_beijing(uint8_t *year,uint8_t *month,uint8_t *day,uint8_t *hour);

// 获取两地间的距离 单位km
double get_distance(double n, double w, double n_ta, double w_ta);

// 获取两地偏角
double get_angle(double n, double w, double n_ta, double w_ta);

#endif  
