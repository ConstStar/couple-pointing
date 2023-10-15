#ifndef __ESP8266_H
#define __ESP8266_H	


#define REV_OK		0	//������ɱ�־
#define REV_WAIT	1	//����δ��ɱ�־

// ����WiFi
#define ESP8266_WIFI_NAME		"test"
#define ESP8266_WIFI_PASSWORD	"1164442003"
 
// ����MQTT
#define ESP8266_MQTT_ADDR		"39.107.228.202"
#define ESP8266_MQTT_PORT		1883
 
#define MQTT_CLIENT_ID 	"client"		//id
#define MQTT_USERNAME  	""				//����
#define MQTT_PASSWORD  	""				//����

// ����MQTT����
// �����ڶ����豸������дʱ����Ҫ�����������ݻ���
#define MQTT_TOPIC_ME	"position_B"
#define MQTT_TOPIC_TA	"position_A"


// ��ջ���
void esp8266_clear(void);
 
// ����MQTT��Ϣ
// ����ֵ 	0 ���ͳɹ�		-1 ����ʧ��
int esp8266_send_mqtt_message(char *data);

// ��ʼ��ESP8266
void esp8266_init(void);

// ����mqtt
void esp8266_connect_mqtt(void);

// �ж�esp8266 && mqtt������״̬
// ����ֵ		0 ��������		-1 δ����
int esp8266_get_connected_state(void);

#endif
