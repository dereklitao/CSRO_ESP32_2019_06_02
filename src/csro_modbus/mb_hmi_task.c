#include "mb_config.h"

void modbus_hmi_task(void *param)
{
    while (true)
    {
        char msg[100];
        for (size_t i = 0; i < 40; i++)
        {
            sprintf(msg, "%d|", airsys_regs.coils[i]);
            uart_write_bytes(UART_NUM_2, msg, strlen(msg));
        }
        uart_write_bytes(UART_NUM_2, "\r\n", strlen("\r\n"));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // if (xSemaphoreTake(slave_hmi.command_sem, portMAX_DELAY) == pdTRUE)
        // {
        //     slave_handle_command(&slave_hmi);
        //     slave_hmi.rx_len = 0;
        // }
    }
}