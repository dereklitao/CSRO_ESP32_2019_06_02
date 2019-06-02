#include "csro_devices.h"

#ifdef AIRSYS

void csro_airsys_init(void)
{
}

void csro_update_airsys_state(void)
{
    cJSON *state_json = cJSON_CreateObject();
    cJSON *airsys_json;

    cJSON_AddStringToObject(state_json, "time", sysinfo.time_str);
    cJSON_AddNumberToObject(state_json, "run", (int)(sysinfo.time_now - sysinfo.time_start));
    cJSON_AddItemToObject(state_json, "state", airsys_json = cJSON_CreateObject());
    cJSON_AddStringToObject(airsys_json, "mode", "heat");
    cJSON_AddNumberToObject(airsys_json, "temp", 19.5);
    cJSON_AddStringToObject(airsys_json, "fan", "low");
    char *out = cJSON_PrintUnformatted(state_json);
    strcpy(mqttinfo.content, out);
    free(out);
    cJSON_Delete(state_json);
    sprintf(mqttinfo.pub_topic, "csro/%s/%s/state", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(mqtt_client, mqttinfo.pub_topic, mqttinfo.content, 0, 0, 1);
}

static void compose_config_json(void)
{
    char prefix[50], name[50];
    sprintf(mqttinfo.pub_topic, "csro/climate/%s_%s/config", sysinfo.mac_str, sysinfo.dev_type);
    sprintf(prefix, "csro/%s/%s", sysinfo.mac_str, sysinfo.dev_type);
    sprintf(name, "%s_%s", sysinfo.dev_type, sysinfo.mac_str);
    cJSON *config_json = cJSON_CreateObject();
    cJSON_AddStringToObject(config_json, "~", prefix);
    cJSON_AddStringToObject(config_json, "name", name);
    cJSON_AddStringToObject(config_json, "avty_t", "~/available");
    cJSON_AddStringToObject(config_json, "pl_avail", "online");
    cJSON_AddStringToObject(config_json, "pl_not_avail", "offline");
    cJSON_AddNumberToObject(config_json, "qos", 1);
    cJSON_AddStringToObject(config_json, "mode_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "mode_stat_tpl", "{{value_json.state.mode}}");
    cJSON_AddStringToObject(config_json, "mode_cmd_t", "~/set/mode");
    cJSON_AddStringToObject(config_json, "temp_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "temp_stat_tpl", "{{value_json.state.temp}}");
    cJSON_AddStringToObject(config_json, "temp_cmd_t", "~/set/temp");
    cJSON_AddStringToObject(config_json, "fan_mode_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "fan_mode_stat_tpl", "{{value_json.state.fan}}");
    cJSON_AddStringToObject(config_json, "fan_mode_cmd_t", "~/set/fan");
    cJSON_AddNumberToObject(config_json, "min_temp", 10);
    cJSON_AddNumberToObject(config_json, "max_temp", 35);
    cJSON_AddNumberToObject(config_json, "temp_step", 0.5);
    char *out = cJSON_PrintUnformatted(config_json);
    strcpy(mqttinfo.content, out);
    free(out);
    cJSON_Delete(config_json);
}

void csro_airsys_on_connect(esp_mqtt_client_handle_t client)
{
    sprintf(mqttinfo.sub_topic, "csro/%s/%s/set/#", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_subscribe(client, mqttinfo.sub_topic, 1);

    compose_config_json();
    esp_mqtt_client_publish(client, mqttinfo.pub_topic, mqttinfo.content, 0, 1, 1);

    sprintf(mqttinfo.pub_topic, "csro/%s/%s/available", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(mqtt_client, mqttinfo.pub_topic, "online", 0, 1, 1);
    csro_update_airsys_state();
}

void csro_airsys_on_message(esp_mqtt_event_handle_t event)
{
    csro_update_airsys_state();
}

#endif