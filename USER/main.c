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

//��ʼ����
#define START_TASK_PRIO		3			//�������ȼ�
#define START_STK_SIZE 		128			//�����ջ��С
OS_TCB StartTaskTCB;					//������ƿ�
CPU_STK START_TASK_STK[START_STK_SIZE];	//�����ջ
void start_task(void *p_arg);			//������

///MQTT����
#define MQTT_TASK_PRIO		4
#define MQTT_STK_SIZE 		256
OS_TCB mqtt_task_tcb;
CPU_STK MQTT_TASK_STK[MQTT_STK_SIZE];
void mqtt_task(void *p_arg);

///OLED��ʾ����
#define	OLED_TASK_PRIO		5
#define OLED_STK_SIZE 		128
OS_TCB oled_task_tcb;
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);

///����ָ������
#define	POINT_TASK_PRIO		3
#define POINT_STK_SIZE 		256
OS_TCB point_task_tcb;
CPU_STK POINT_TASK_STK[POINT_STK_SIZE];
void point_task(void *p_arg);

//GPS����
#define	GPS_TASK_PRIO		4
#define GPS_STK_SIZE 		256
OS_TCB gps_task_tcb;
CPU_STK GPS_TASK_STK[GPS_STK_SIZE];
void gps_task(void *p_arg);

//HC14����
#define	HC14_TASK_PRIO		4
#define HC14_STK_SIZE 		256
OS_TCB hc14_task_tcb;
CPU_STK HC14_TASK_STK[HC14_STK_SIZE];
void hc14_task(void *p_arg);


// OLED�ź���������֪ͨ��Ļ������ʾ����
OS_SEM oled_sem;



// �Է���������Ϣ ��Ϣ����Ϊ16����gps��λ��Ϣ
extern char mqtt_receive_message[RX_DATA_SIZE];		// ��Դ�� mqtt
extern char hc14_receive_message[RX_DATA_SIZE];		// ��Դ�� hc14


// hc14 ����״̬ 0��δ���� 1������
uint8_t hc14_connect_state = 0;

//mqtt ����״̬ 0��δ���� 1������
extern uint8_t mqtt_connect_state;



GPS gps_info;			// �ҵ�λ����Ϣ
GPS gps_info_ta;		// �Է���λ����Ϣ

double distance = 0;	// ��ž���	km
double angle = 0;		// ��ŷ���



//������
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�жϷ�������
	SystemInit();	//���ڱ���������stm32f103zet6��ֲ�����������������뱾����ȷ��delay��������ʹ��
	
	delay_init();  			//��ʼ��ʱ��
	usart1_init();			// ��ʼ������1
	gps_init();				// ��ʼ��GPS
	oled_init();			// ��ʼ��OLED
	led_init();				// ��ʼ��LED
	esp8266_init();			// ��ʼ��ESP8266
	hmc5883l_init();		// ��ʼ������ָ����
	iwdg_init(25);			// ��ʼ���������Ź�

#if DEBUG
	printf("��������\r\n");
#endif

	
	OSInit(&err);		    //��ʼ��UCOSIII
	OS_CRITICAL_ENTER();	//�����ٽ���			 
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);      //����UCOSIII
}


//��ʼ����������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	
	//����OLED�ź���
    OSSemCreate(&oled_sem,"oled_sem",1,&err);
	
	
	//����MQTT����
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
				 
	//����OLED��ʾ����
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
				 
	//��������ָ������
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
				 
	//����GPS����
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
				 
	//����HC14����
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
				 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	OSTaskDel((OS_TCB*)0,&err);	//ɾ��start_task��������
}


//MQTT������
void mqtt_task(void *p_arg)
{
	char data_buf[120];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	esp8266_connect_mqtt();		// ��������
	
	while(1)
	{
		// ι��
		iwdg_feed();
	
		// �ж�MQTT����״̬ ���������������
		if(esp8266_get_connected_state() == -1)
		{
#if DEBUG
			printf("MQTT �ѶϿ�����\r\n");
#endif
			esp8266_connect_mqtt();
		}
	
		
		OS_CRITICAL_ENTER();
#if DEBUG
		printf("mqtt runing...\r\n");
#endif
		// ���HC14������
		if(hc14_connect_state == 0)
		{
			// ��ͨ��MQTT����������
			if(gps_from_message(mqtt_receive_message,&gps_info_ta) == 0)
			{	
#if DEBUG
				printf("mqtt receive message\r\n");
#endif		
				// ����λ�úͷ���
				gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
				
				// �ͷ��ź���
				OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
			}
		}
		
		// ι��
		iwdg_feed();
		
		//ͨ��MQTT���͸��Է�
		gps_to_message(gps_info,data_buf);
		esp8266_send_mqtt_message(data_buf);
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,2,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


//HC14������
void hc14_task(void *p_arg)
{
	char data_buf[120];
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	// ��¼hc14���ٴ�û�з�����Ϣ
	// �����ж϶Է�hc14�Ƿ����ߡ�
	int hc14_no_message_count = 0;	
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();
#if DEBUG
		printf("hc14 runing...\r\n");
#endif
		
		// ��ͨ��HC14����������
		if(gps_from_message(hc14_receive_message,&gps_info_ta) == 0)
		{
#if DEBUG
			printf("hc14 receive message\r\n");
#endif
			
			// ����λ�úͷ���
			gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
			
			// ����HC14����״̬
			hc14_no_message_count = 0;
			hc14_connect_state = 1;
			
			// �ͷ��ź���
			OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
		}
		else
		{
			++hc14_no_message_count;
			
			// �������12�� ������HC14Ϊ������
			if(hc14_no_message_count > 12)
			{
#if DEBUG
				printf("HC14������\r\n");
#endif
				hc14_connect_state = 0;
			}
		}
		
		// ���������Ч
		if(gps_info.n[0] != 0 && gps_info.w[0] !=0)
		{
			//ͨ��HC14���͸��Է�
			gps_to_message(gps_info,data_buf);
			usart1_printf("#%s\r\n",data_buf);
		}
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//OLED��ʾ������
void oled_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	while(1)
	{
		// ι��
		iwdg_feed();
		
        OSSemPend( &oled_sem,
                    0,                      //��Զ�ȴ�
                    OS_OPT_PEND_BLOCKING,   //��������
                    0,                      //����¼ʱ���
                    &err                    //������
                  );
		
		
		OS_CRITICAL_ENTER();
		
#if DEBUG
		printf("oled runing...\r\n");
#endif
		// �Է��ķ���Ƕ�
		float ta_angle = angle;
		
		oled_clear();
		char temp_format[50];
		
		
		// �������
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
		
				
		// ���㷽��
		uint8_t hzk_index[2];
		
		// ��һ����
		if(ta_angle>=0 && ta_angle<=90)
		{
			// ��������
			if(ta_angle>45)
			{
				hzk_index[0] = 3;	//��
				hzk_index[1] = 0;	//��
				sprintf(temp_format,"%d",(int)(90-ta_angle));
			}
			
			// ��������
			else
			{
				hzk_index[0] = 0;	//��
				hzk_index[1] = 3;	//��
				sprintf(temp_format,"%d",(int)(ta_angle-0));
			}
		}
		
		// �ڶ�����
		else if(ta_angle>=90 && ta_angle<=180)
		{
			// ��������
			if(angle>135)
			{
				hzk_index[0] = 1;	//��
				hzk_index[1] = 3;	//��
				sprintf(temp_format,"%d",(int)(180-ta_angle));
			}
			
			// ��������
			else
			{
				hzk_index[0] = 3;	//��
				hzk_index[1] = 1;	//��
				sprintf(temp_format,"%d",(int)(ta_angle-90));
			}
		}
		
		// ��������
		else if(ta_angle>=-180 && ta_angle<=-90)
		{
			// ��������
			if(ta_angle>-135)
			{
				hzk_index[0] = 2;	//��
				hzk_index[1] = 1;	//��
				sprintf(temp_format,"%d",(int)(-90-ta_angle));
			}
			
			// ��������
			else
			{
				hzk_index[0] = 1;	//��
				hzk_index[1] = 2;	//��
				sprintf(temp_format,"%d",(int)(ta_angle+180));
			}
		}
		
		// ��������
		else if(ta_angle>=-90 && ta_angle<=0)
		{
			// ��������
			if(ta_angle>-45)
			{
				hzk_index[0] = 0;	//��
				hzk_index[1] = 2;	//��
				sprintf(temp_format,"%d",(int)(0-ta_angle));
			}
			
			// ��������
			else
			{
				hzk_index[0] = 2;	//��
				hzk_index[1] = 0;	//��
				sprintf(temp_format,"%d",(int)(ta_angle+90));
			}
		}
		
		oled_show_chinese(48,0,hzk_index[0]);	//����
		oled_show_chinese(64,0,4);	//ƫ
		oled_show_chinese(80,0,hzk_index[1]);	//����
		oled_show_string(96,0,temp_format,16);
		oled_show_chinese(112,0,5);	//��
		
		
		// �������ʱ��
		
		// �ҵ�gpt��������
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		
		// �Է���gpt��������
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
		
		
		// ��ʽ������
		uint32_t temp1 = year*10000+month*100+day;
		uint32_t temp2 = year_ta*10000+month_ta*100+day_ta;
		
		// ���������ͬ ��ֻ��ʾʱ��
		if( temp1 == temp2)
		{
			
			// ��ʽ��ʱ��
			temp1 = hour*10000+minute*100+second;
			temp2 = hour_ta*10000+minute_ta*100+second_ta;
			
			// ��ʾһ�������ʱ�� ��Ϊ����ʱ��
			if(temp1 > temp2)
			{
				sprintf(temp_format,"%02d:%02d:%02d",hour_ta,minute_ta,second_ta);
			}
			else
			{
				sprintf(temp_format,"%02d:%02d:%02d",hour,minute,second);
			}
			
		}
		
		// ������ڲ�ͬ ��ֻ��ʾ����
		else
		{			
			// ��¼һ����������� ��Ϊ����ʱ��
			if(temp1 > temp2)
			{
				sprintf(temp_format,"20%02d.%02d.%02d",year_ta,month_ta,day_ta);
			}
			else
			{
				sprintf(temp_format,"20%02d.%02d.%02d",year,month,day);
			}
				
		}
		
		oled_show_chinese(0,2,6);	//��
		oled_show_chinese(16,2,7);	//��
		oled_show_chinese(32,2,8);	//��
		oled_show_string(48,2,temp_format,16);
		
		// �����������
		if(mqtt_connect_state == 0)
		{
			oled_show_string(0,7,"internet offline",12);
		}
		else
		{
			oled_show_string(0,7,"internet online",12);
		}
			
		// HC14�������
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


//����ָ��������
void point_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("point runing...\r\n");
#endif
		// ��ȡ��ǰ����ĽǶ�
		uint16_t self_angle = hmc5883l_get_angle();
	
		// ����Է�������Ƕ������Ϸ���Ĳ�ֵ
		uint16_t num = (uint16_t)(self_angle+angle + 90 + 360)%360;
		
		led_set_light_angle(num);

#if DEBUG
		printf("ָ��λ�ø���%d %lf %lf\r\n",num,self_angle,angle);
#endif
		
		OS_CRITICAL_EXIT();
		OSTimeDlyHMSM(0,0,0,300,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//GPS������
void gps_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	while(1)
	{
		// ι��
		iwdg_feed();
		
		OS_CRITICAL_ENTER();

#if DEBUG
		printf("gps runing...\r\n");
#endif
		
		// ��ȡ�ҵ�gps��Ϣ ���͸��Է�
		if(gps_get_info(&gps_info) == 0)
		{
			
			// ����λ�úͷ���
			gps_get_distance_and_angle(gps_info,gps_info_ta,&distance,&angle);
			
			//�ͷ��ź���
			OSSemPost(&oled_sem,OS_OPT_POST_1,&err);
		}
		
		OS_CRITICAL_EXIT();
		
		OSTimeDlyHMSM(0,0,1,500,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}


