#include "mb_config.h"

void modbus_ap_read_task(void *param)
{
    while (true)
    {
        uart_write_bytes(UART_NUM_0, "UART0\r\n", 7);
        //master_read_coils(&master_ap, 1, 20, airsys_regs.coils);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void modbus_ap_write_task(void *param)
{
}
