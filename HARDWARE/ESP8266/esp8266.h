#ifndef __ESP8266_H
#define __ESP8266_H	


#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

// 配置WiFi
#define ESP8266_WIFI_NAME		"test"
#define ESP8266_WIFI_PASSWORD	"1164442003"
 
// 配置MQTT
#define ESP8266_MQTT_ADDR		"39.107.228.202"
#define ESP8266_MQTT_PORT		1883
 
#define MQTT_CLIENT_ID 	"client"		//id
#define MQTT_USERNAME  	""				//名字
#define MQTT_PASSWORD  	""				//密码

// 配置MQTT订阅
// 当给第二个设备编译烧写时，需要讲这两个内容互换
#define MQTT_TOPIC_ME	"position_B"
#define MQTT_TOPIC_TA	"position_A"


// 清空缓存
void esp8266_clear(void);
 
// 发送MQTT消息
// 返回值 	0 发送成功		-1 发送失败
int esp8266_send_mqtt_message(char *data);

// 初始化ESP8266
void esp8266_init(void);

// 连接mqtt
void esp8266_connect_mqtt(void);

// 判断esp8266 && mqtt的连接状态
// 返回值		0 连接正常		-1 未连接
int esp8266_get_connected_state(void);

#endif
