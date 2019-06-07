#include "csro_devices.h"
#include "csro_modbus/mb_config.h"

#define BUF_SIZE (1024)
#define TXD0_PIN (GPIO_NUM_15)
#define RXD0_PIN (GPIO_NUM_2)
#define RTS0_PIN (GPIO_NUM_0)
#define TXD1_PIN (GPIO_NUM_4)
#define RXD1_PIN (GPIO_NUM_17)
#define RTS1_PIN (GPIO_NUM_16)
#define TXD2_PIN (GPIO_NUM_5)
#define RXD2_PIN (GPIO_NUM_19)
#define RTS2_PIN (GPIO_NUM_18)

#define GPIO_SELECTED_PIN ((1ULL << LED_PIN) | (1ULL << RELAY01_PIN) | (1ULL << RELAY02_PIN) | (1ULL << RELAY05_PIN) | (1ULL << RELAY06_PIN) | (1ULL << RELAY07_PIN) | (1ULL << RELAY08_PIN) | (1ULL << RELAY10_PIN) | (1ULL << RELAY11_PIN) | (1ULL << RELAY12_PIN))

#ifdef AIRSYS

static void uart_receive_one_byte(uart_port_t uart_num, uint8_t data)
{
    if (uart_num == master_ap.uart_num)
    {
        master_ap.rx_buf[master_ap.rx_len++] = data;
    }
    else if (uart_num == master_ac.uart_num)
    {
        master_ac.rx_buf[master_ac.rx_len++] = data;
    }
    else if (uart_num == slave_hmi.uart_num)
    {
        slave_hmi.rx_buf[slave_hmi.rx_len++] = data;
    }
}

static void uart_receive_complete(uart_port_t uart_num)
{
    static portBASE_TYPE HPTaskAwoken = 0;

    if (uart_num == master_ap.uart_num)
    {
        uart_flush(master_ap.uart_num);
        xSemaphoreGiveFromISR(master_ap.reply_sem, &HPTaskAwoken);
    }
    else if (uart_num == master_ac.uart_num)
    {
        uart_flush(master_ac.uart_num);
        xSemaphoreGiveFromISR(master_ac.reply_sem, &HPTaskAwoken);
    }
    else if (uart_num == slave_hmi.uart_num)
    {
        uart_flush(slave_hmi.uart_num);
        xSemaphoreGiveFromISR(slave_hmi.command_sem, &HPTaskAwoken);
    }
}

static bool master_ap_send_receive(uint16_t timeout)
{
    master_ap.rx_len = 0;
    uart_write_bytes(master_ap.uart_num, (const char *)master_ap.tx_buf, master_ap.tx_len);
    if (xSemaphoreTake(master_ap.reply_sem, timeout / portTICK_PERIOD_MS) == pdTRUE)
    {
        master_ap.status = true;
        airsys_regs.coils[42] = 1;
    }
    else
    {
        master_ap.status = false;
        airsys_regs.coils[42] = 0;
    }
    return master_ap.status;
}

static bool master_ac_send_receive(uint16_t timeout)
{
    master_ac.rx_len = 0;
    uart_write_bytes(master_ac.uart_num, (const char *)master_ac.tx_buf, master_ac.tx_len);
    if (xSemaphoreTake(master_ac.reply_sem, timeout / portTICK_PERIOD_MS) == pdTRUE)
    {
        master_ac.status = true;
        airsys_regs.coils[41] = 1;
    }
    else
    {
        master_ac.status = false;
        airsys_regs.coils[41] = 0;
    }
    return master_ac.status;
}

static void led_relay_task(void *param)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_SELECTED_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    static bool led_status = false;
    static uint8_t cnt = 0;
    while (true)
    {
        gpio_set_level(RELAY01_PIN, airsys_regs.coils[201]);
        gpio_set_level(RELAY02_PIN, airsys_regs.coils[202]);
        gpio_set_level(RELAY03_PIN, airsys_regs.coils[203]);
        gpio_set_level(RELAY04_PIN, airsys_regs.coils[204]);
        gpio_set_level(RELAY05_PIN, airsys_regs.coils[205]);
        gpio_set_level(RELAY06_PIN, airsys_regs.coils[206]);
        gpio_set_level(RELAY07_PIN, airsys_regs.coils[207]);
        gpio_set_level(RELAY08_PIN, airsys_regs.coils[208]);
        gpio_set_level(RELAY10_PIN, airsys_regs.coils[210]);
        gpio_set_level(RELAY11_PIN, airsys_regs.coils[211]);
        gpio_set_level(RELAY12_PIN, airsys_regs.coils[212]);
        cnt = (cnt + 1) % 10;
        if (cnt == 1)
        {
            gpio_set_level(LED_PIN, led_status);
            led_status = !led_status;
        }
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void csro_airsys_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUM_0, &uart_config);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_param_config(UART_NUM_2, &uart_config);

    uart_set_pin(UART_NUM_0, TXD0_PIN, RXD0_PIN, RTS0_PIN, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, RTS1_PIN, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM_2, TXD2_PIN, RXD2_PIN, RTS2_PIN, UART_PIN_NO_CHANGE);

    uart_handler.receive_one_byte = uart_receive_one_byte;
    uart_handler.receive_complete = uart_receive_complete;

    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);

    uart_set_mode(UART_NUM_0, UART_MODE_RS485_HALF_DUPLEX);
    uart_set_mode(UART_NUM_1, UART_MODE_RS485_HALF_DUPLEX);
    uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);

    master_ap.uart_num = UART_NUM_0;
    master_ap.slave_id = 1;
    master_ap.master_send_receive = master_ap_send_receive;
    master_ap.reply_sem = xSemaphoreCreateBinary();
    master_ap.write_sem = xSemaphoreCreateBinary();
    master_ap.uart_mutex = xSemaphoreCreateMutex();

    master_ac.uart_num = UART_NUM_1;
    master_ac.slave_id = 1;
    master_ac.master_send_receive = master_ac_send_receive;
    master_ac.reply_sem = xSemaphoreCreateBinary();
    master_ac.write_sem = xSemaphoreCreateBinary();
    master_ac.uart_mutex = xSemaphoreCreateMutex();

    slave_hmi.uart_num = UART_NUM_2;
    slave_hmi.slave_id = 1;
    slave_hmi.command_sem = xSemaphoreCreateBinary();
    slave_hmi.regs = &airsys_regs;

    xTaskCreate(modbus_ac_read_task, "modbus_ac_read_task", 2048, NULL, configMAX_PRIORITIES - 7, NULL);
    xTaskCreate(modbus_ac_write_task, "modbus_ac_write_task", 2048, NULL, configMAX_PRIORITIES - 4, NULL);

    xTaskCreate(modbus_ap_read_task, "modbus_ap_read_task", 2048, NULL, configMAX_PRIORITIES - 7, NULL);
    xTaskCreate(modbus_ap_write_task, "modbus_ap_write_task", 2048, NULL, configMAX_PRIORITIES - 4, NULL);

    xTaskCreate(modbus_hmi_task, "modbus_hmi_task", 2048, NULL, configMAX_PRIORITIES - 8, NULL);
    xTaskCreate(led_relay_task, "led_relay_task", 2048, NULL, configMAX_PRIORITIES - 6, NULL);
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