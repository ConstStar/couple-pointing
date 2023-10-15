#include "sys.h"

#include <string.h>

#include "includes.h"
#include "delay.h"
#include "iwdg.h"
#include "utils.h"

#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "esp8266.h"
#include "oled.h"
#include "led.h"
#include "hmc5883l.h"
#include "gps.h"

//开始任务
#define START_TASK_PRIO		3			//任务优先级
#define START_STK_SIZE 		128			//任务堆栈大小
OS_TCB StartTaskTCB;					//任务控制块
CPU_STK START_TASK_STK[START_STK_SIZE];	//任务堆栈
void start_task(void *p_arg);			//任务函数

///MQTT任务
#define MQTT_TASK_PRIO		4
#define MQTT_STK_SIZE 		256
OS_TCB mqtt_task_tcb;
CPU_STK MQTT_TASK_STK[MQTT_STK_SIZE];
void mqtt_task(void *p_arg);

///OLED显示任务
#define	OLED_TASK_PRIO		5
#define OLED_STK_SIZE 		128
OS_TCB oled_task_tcb;
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);

///操作指针任务
#define	POINT_TASK_PRIO		3
#define POINT_STK_SIZE 		256
OS_TCB point_task_tcb;
CPU_STK POINT_TASK_STK[POINT_STK_SIZE];
void point_task(void *p_arg);

//GPS任务
#define	GPS_TASK_PRIO		4
#define GPS_STK_SIZE 		256
OS_TCB gps_task_tcb;
CPU_STK GPS_TASK_STK[GPS_STK_SIZE];
void gps_task(void *p_arg);

//HC14任务
#define	HC14_TASK_PRIO		4
#define HC14_STK_SIZE 		256
OS_TCB hc14_task_tcb;
CPU_STK HC14_TASK_STK[HC14_STK_SIZE];
void hc14_task(void *p_arg);


// OLED信号量，用来通知屏幕更新显示内容
OS_SEM oled_sem;



// 对方发来的消息 消息内容为16进制gps定位信息
extern char mqtt_receive_message[RX_DATA_SIZE];		// 来源于 mqtt
extern char hc14_receive_message[RX_DATA_SIZE];		// 来源于 hc14


// hc14 连接状态 0：未连接 1：连接
uint8_t hc14_connect_state = 0;

//mqtt 连接状态 0：未连接 1：连接
extern uint8_t mqtt_connect_state;



GPS gps_info;			// 我的位置信息
GPS gps_info_ta;		// 对方的位置信息

double distance = 0;	// 存放距离	km
double angle = 0;		// 存放方向



//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	SystemInit();	//由于本工程是由stm32f103zet6移植所来，因此这里因加入本函数确保delay函数正常使用
	
	delay_init();  			//初始化时钟
	usart1_init();			// 初始化串口1
	gps_init();				// 初始化GPS
	oled_init();			// 初始化OLED
	led_init();				// 初始化LED
	esp8266_init();			// 初始化ESP8266
	hmc5883l_init();		// 初始化电子指南针
	iwdg_init(25);			// 初始化独立看门狗

#if DEBUG
	printf("程序启动\r\n");
#endif

	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}


//开始任务任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//进入临界区
	
	//创建OLED信号量
    OSSemCreate(&oled_sem,"oled_sem",1,&err);
	
	
	//创建MQTT任务
	OSTaskCreate((OS_TCB 	* )&mqtt_task_tcb,		
				 (CPU_CHAR	* )"MQTT task", 		
                 (OS_TASK_PTR )mqtt_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )MQTT_TASK_PRIO,     
                 (CPU_STK   * )&MQTT_TASK_STK[0],	
                 (CPU_STK_SIZE)MQTT_STK_SIZE/10,	
                 (CPU_STK_SIZE)MQTT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建OLED显示任务
	OSTaskCreate((OS_TCB 	* )&oled_task_tcb,		
				 (CPU_CHAR	* )"OLED task", 		
                 (OS_TASK_PTR )oled_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )OLED_TASK_PRIO,     	
                 (CPU_STK   * )&OLED_TASK_STK[0],	
                 (CPU_STK_SIZE)OLED_STK_SIZE/10,	
                 (CPU_STK_SIZE)OLED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建操作指针任务
	OSTaskCreate((OS_TCB 	* )&point_task_tcb,		
				 (CPU_CHAR	* )"Point task", 		
                 (OS_TASK_PTR )point_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )POINT_TASK_PRIO,     	
                 (CPU_STK   * )&POINT_TASK_STK[0],	
                 (CPU_STK_SIZE)POINT_STK_SIZE/10,	
                 (CPU_STK_SIZE)POINT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建GPS任务
	OSTaskCreate((OS_TCB 	* )&gps_task_tcb,		
				 (CPU_CHAR	* )"GPS task", 		
                 (OS_TASK_PTR )gps_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )GPS_TASK_PRIO,     	
                 (CPU_STK   * )&GPS_TASK_STK[0],	
                 (CPU_STK_SIZE)GPS_STK_SIZE/10,	
                 (CPU_STK_SIZE)GPS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	//创建HC14任务
	OSTaskCreate((OS_TCB 	* )&hc14_task_tcb,		
				 (CPU_CHAR	* )"HC14 task", 		
                 (OS_TASK_PTR )hc14_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )HC14_TASK_PRIO,     	
                 (CPU_STK   * )&HC14_TASK_STK[0],	
                 (CPU_STK_SIZE)HC14_STK_SIZE/10,	
                 (CPU_STK_SIZE)HC14_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}


//MQTT任务函数
void mqtt_task(void *p_arg)
{
	char data_buf[120];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	esp8266_connect_mqtt();		// 连接网络
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
	
		// 判断MQTT连接状态 如果不在线则重连
		if(esp8266_get_connected_state() == -1)
		{
#if DEBUG
			printf("MQTT 已断开连接\r\n");
#endif
			esp8266_connect_mqtt();
		}
	
		
		OS_CRITICAL_ENTER();
#if DEBUG
		printf("mqtt runing...\r\n");
#endif
		// 如果HC14不在线
		if(hc14_connect_state == 0)
		{
			// 则通过MQTT来接受数据
			if(gps_from_message(mqtt_receive_message,&gps_info_ta) == 0)
			{	
#if DEBUG
				printf("mqtt receive message\r\n");
#endif		
				// 计算位置和方向
				gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
				
				// 释放信号量
				OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
			}
		}
		
		// 喂狗
		iwdg_feed();
		
		//通过MQTT发送给对方
		gps_to_message(gps_info,data_buf);
		esp8266_send_mqtt_message(data_buf);
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


//HC14任务函数
void hc14_task(void *p_arg)
{
	char data_buf[120];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	// 记录hc14多少次没有发送消息
	// 用来判断对方hc14是否“在线”
	int hc14_no_message_count = 0;	
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
#if DEBUG
		printf("hc14 runing...\r\n");
#endif
		
		// 则通过HC14来接受数据
		if(gps_from_message(hc14_receive_message,&gps_info_ta) == 0)
		{
#if DEBUG
			printf("hc14 receive message\r\n");
#endif
			
			// 计算位置和方向
			gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
			
			// 设置HC14在线状态
			hc14_no_message_count = 0;
			hc14_connect_state = 1;
			
			// 释放信号量
			OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
		}
		else
		{
			++hc14_no_message_count;
			
			// 如果超过12次 则设置HC14为不在线
			if(hc14_no_message_count > 12)
			{
#if DEBUG
				printf("HC14不在线\r\n");
#endif
				hc14_connect_state = 0;
			}
		}
		
		// 如果数据有效
		if(gps_info.n[0] != 0 && gps_info.w[0] !=0)
		{
			//通过HC14发送给对方
			gps_to_message(gps_info,data_buf);
			usart1_printf("#%s\r\n",data_buf);
		}
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//OLED显示任务函数
void oled_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// 喂狗
		iwdg_feed();
		
        OSSemPend( &oled_sem,
                    0,                      //永远等待
                    OS_OPT_PEND_BLOCKING,   //进行阻塞
                    0,                      //不记录时间戳
                    &err                    //出错处理
                  );
		
		
		OS_CRITICAL_ENTER();
		
#if DEBUG
		printf("oled runing...\r\n");
#endif
		// 对方的方向角度
		float ta_angle = angle;
		
		oled_clear();
		char temp_format[50];
		
		
		// 计算距离
		if(distance>1)
		{
			sprintf(temp_format,"%dkm",(int)distance);
		}
		else
		{
			distance*=1000;
			if(distance<1)
			{
				sprintf(temp_format,"<1m");
			}
			else
			{
				sprintf(temp_format,"%dm",(int)distance);
			}
		}
		oled_show_string(0,0,temp_format,16);
		
				
		// 计算方向
		uint8_t hzk_index[2];
		
		// 第一象限
		if(ta_angle>=0 && ta_angle<=90)
		{
			// 更靠近北
			if(ta_angle>45)
			{
				hzk_index[0] = 3;	//北
				hzk_index[1] = 0;	//东
				sprintf(temp_format,"%d",(int)(90-ta_angle));
			}
			
			// 更靠近东
			else
			{
				hzk_index[0] = 0;	//东
				hzk_index[1] = 3;	//北
				sprintf(temp_format,"%d",(int)(ta_angle-0));
			}
		}
		
		// 第二象限
		else if(ta_angle>=90 && ta_angle<=180)
		{
			// 更靠近西
			if(angle>135)
			{
				hzk_index[0] = 1;	//西
				hzk_index[1] = 3;	//北
				sprintf(temp_format,"%d",(int)(180-ta_angle));
			}
			
			// 更靠近北
			else
			{
				hzk_index[0] = 3;	//北
				hzk_index[1] = 1;	//西
				sprintf(temp_format,"%d",(int)(ta_angle-90));
			}
		}
		
		// 第三象限
		else if(ta_angle>=-180 && ta_angle<=-90)
		{
			// 更靠近南
			if(ta_angle>-135)
			{
				hzk_index[0] = 2;	//南
				hzk_index[1] = 1;	//西
				sprintf(temp_format,"%d",(int)(-90-ta_angle));
			}
			
			// 更靠近西
			else
			{
				hzk_index[0] = 1;	//西
				hzk_index[1] = 2;	//南
				sprintf(temp_format,"%d",(int)(ta_angle+180));
			}
		}
		
		// 第四象限
		else if(ta_angle>=-90 && ta_angle<=0)
		{
			// 更靠近东
			if(ta_angle>-45)
			{
				hzk_index[0] = 0;	//东
				hzk_index[1] = 2;	//南
				sprintf(temp_format,"%d",(int)(0-ta_angle));
			}
			
			// 更靠近南
			else
			{
				hzk_index[0] = 2;	//南
				hzk_index[1] = 0;	//东
				sprintf(temp_format,"%d",(int)(ta_angle+90));
			}
		}
		
		oled_show_chinese(48,0,hzk_index[0]);	//方向
		oled_show_chinese(64,0,4);	//偏
		oled_show_chinese(80,0,hzk_index[1]);	//方向
		oled_show_string(96,0,temp_format,16);
		oled_show_chinese(112,0,5);	//度
		
		
		// 计算更新时间
		
		// 我的gpt数据日期
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		
		// 对方的gpt数据日期
		uint8_t year_ta;
		uint8_t month_ta;
		uint8_t day_ta;
		uint8_t hour_ta;
		uint8_t minute_ta;
		uint8_t second_ta;

		year = gps_info.utc_date%100;
		month = gps_info.utc_date/100%100;
		day = gps_info.utc_date/10000;
		hour = gps_info.utc_time/10000%100;
		minute = gps_info.utc_time/100%100;
		second = gps_info.utc_time%100;
		uct_to_beijing(&year,&month,&day,&hour);
		
		year_ta = gps_info_ta.utc_date%100;
		month_ta = gps_info_ta.utc_date/100%100;
		day_ta = gps_info_ta.utc_date/10000;
		hour_ta = gps_info_ta.utc_time/10000%100;
		minute_ta = gps_info_ta.utc_time/100%100;
		second_ta = gps_info_ta.utc_time%100;
		uct_to_beijing(&year_ta,&month_ta,&day_ta,&hour_ta);
		
		
		// 格式化日期
		uint32_t temp1 = year*10000+month*100+day;
		uint32_t temp2 = year_ta*10000+month_ta*100+day_ta;
		
		// 如果日期相同 则只显示时间
		if( temp1 == temp2)
		{
			
			// 格式化时间
			temp1 = hour*10000+minute*100+second;
			temp2 = hour_ta*10000+minute_ta*100+second_ta;
			
			// 显示一个最晚的时间 作为更新时间
			if(temp1 > temp2)
			{
				sprintf(temp_format,"%02d:%02d:%02d",hour_ta,minute_ta,second_ta);
			}
			else
			{
				sprintf(temp_format,"%02d:%02d:%02d",hour,minute,second);
			}
			
		}
		
		// 如果日期不同 则只显示日期
		else
		{			
			// 记录一个最晚的日期 作为更新时间
			if(temp1 > temp2)
			{
				sprintf(temp_format,"20%02d.%02d.%02d",year_ta,month_ta,day_ta);
			}
			else
			{
				sprintf(temp_format,"20%02d.%02d.%02d",year,month,day);
			}
				
		}
		
		oled_show_chinese(0,2,6);	//更
		oled_show_chinese(16,2,7);	//新
		oled_show_chinese(32,2,8);	//于
		oled_show_string(48,2,temp_format,16);
		
		// 网络连接情况
		if(mqtt_connect_state == 0)
		{
			oled_show_string(0,7,"internet offline",12);
		}
		else
		{
			oled_show_string(0,7,"internet online",12);
		}
			
		// HC14连接情况
		if(hc14_connect_state == 0)
		{
			oled_show_string(0,6,"hc14     offline",12);
		}
		else
		{
			oled_show_string(0,6,"hc14     online",12);
		}
		
		OS_CRITICAL_EXIT();

		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


//操作指针任务函数
void point_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("point runing...\r\n");
#endif
		// 获取当前自身的角度
		uint16_t self_angle = hmc5883l_get_angle();
	
		// 计算对方和自身角度与正南方向的差值
		uint16_t num = (uint16_t)(self_angle+angle + 90 + 360)%360;
		
		led_set_light_angle(num);

#if DEBUG
		printf("指针位置更新%d %lf %lf\r\n",num,self_angle,angle);
#endif
		
		OS_CRITICAL_EXIT();
		OSTimeDlyHMSM(0,0,0,300,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//GPS任务函数
void gps_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// 喂狗
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("gps runing...\r\n");
#endif
		
		// 获取我的gps信息 发送给对方
		if(gps_get_info(&gps_info) == 0)
		{
			
			// 计算位置和方向
			gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
			
			//释放信号量
			OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
		}
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,1,500,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


