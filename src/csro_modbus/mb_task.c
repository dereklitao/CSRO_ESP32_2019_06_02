
#include "mb_config.h"
#include "csro_devices/csro_devices.h"

modbus_master master_ap;
modbus_master master_ac;
modbus_slave slave_hmi;
device_regs airsys_regs;

void modbus_ap_task(void *param)
{
    while (true)
    {
        uart_write_bytes(UART_NUM_0, "UART0\r\n", 7);
        //master_read_coils(&master_ap, 1, 20, airsys_regs.coils);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void modbus_ac_task(void *param)
{
    while (true)
    {
        master_read_discs(&master_ac, 1, 39, &airsys_regs.coils[1]);
        master_read_coils(&master_ac, 1, 1, &airsys_regs.coils[1]);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

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