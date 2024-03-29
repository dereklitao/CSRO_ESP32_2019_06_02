#ifndef CSRO_COMMON_H_
#define CSRO_COMMON_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "lwip/apps/sntp.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include "time.h"
#include "cJSON.h"

#define MQTT_BROKER "mqtt://192.168.2.2"

#define AIRSYS
/* 
#define AIRSYS
*/

typedef enum
{
    idle = 0,
    smart_config = 1,
    smart_config_timeout = 2,
    mqtt_router_connecting = 3,
    mqtt_server_connecting = 4,
    mqtt_server_connected = 5,
} wifi_status;

typedef struct
{
    wifi_status status;
    uint8_t restore_flag;
    uint8_t router_flag;
    char router_ssid[50];
    char router_pass[50];

    uint8_t mac[6];
    char mac_str[20];
    uint8_t ip[4];
    char ip_str[20];

    char host_name[20];
    char dev_type[20];

    uint32_t power_cnt;
    uint32_t router_cnt;
    uint32_t server_cnt;

    uint8_t time_sync;
    time_t time_start;
    time_t time_now;
    struct tm time_info;
    char time_str[64];
} csro_system;

typedef struct
{
    char id[50];
    char name[50];
    char pass[50];

    char sub_topic[100];
    char pub_topic[100];
    char lwt_topic[100];

    char content[1000];

    char broker[50];
    char prefix[50];

    uint16_t interval;
} csro_mqtt;

extern csro_system sysinfo;
extern csro_mqtt mqttinfo;
extern esp_mqtt_client_handle_t mqtt_client;

//common.c
void csro_wifi_get_info(void);
void csro_system_get_info(void);

//csro_smart_config.c
void csro_start_smart_config(void);

//csro_mqtt.c
void csro_start_mqtt(void);

#endif