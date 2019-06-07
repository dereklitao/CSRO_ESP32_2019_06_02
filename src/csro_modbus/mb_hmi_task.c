#include "mb_config.h"

void modbus_hmi_task(void *param)
{
    while (true)
    {
        // char msg[100];
        // uart_write_bytes(UART_NUM_2, "COIL: ", strlen("COIL: "));
        // for (size_t i = 0; i < 42; i++)
        // {
        //     sprintf(msg, "%d|", airsys_regs.coils[i]);
        //     uart_write_bytes(UART_NUM_2, msg, strlen(msg));
        // }
        // uart_write_bytes(UART_NUM_2, "\r\n", strlen("\r\n"));
        // uart_write_bytes(UART_NUM_2, "HOLDING: ", strlen("HOLDING: "));
        // for (size_t i = 0; i < 30; i++)
        // {
        //     sprintf(msg, "%d|", airsys_regs.holdings[i]);
        //     uart_write_bytes(UART_NUM_2, msg, strlen(msg));
        // }
        // uart_write_bytes(UART_NUM_2, "\r\n", strlen("\r\n"));
        // vTaskDelay(200 / portTICK_PERIOD_MS);
        if (xSemaphoreTake(slave_hmi.command_sem, portMAX_DELAY) == pdTRUE)
        {
            slave_handle_command(&slave_hmi);
            slave_hmi.rx_len = 0;
        }
    }
    vTaskDelete(NULL);
}

void modbus_relay_task(void *param)
{
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
        vTaskDelay(50 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}