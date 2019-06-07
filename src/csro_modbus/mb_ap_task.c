#include "mb_config.h"

void modbus_ap_read_task(void *param)
{
    while (true)
    {
        uint8_t byte_result[100];
        uint16_t word_result[100];

        //ap_disc[1-31] ---->  sys_coil[101-131]
        if (xSemaphoreTake(master_ap.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_discs(&master_ap, 1, 31, byte_result))
            {
                for (size_t i = 0; i < 31; i++)
                {
                    airsys_regs.coils[i + 101] = byte_result[i];
                }
            }
            xSemaphoreGive(master_ap.uart_mutex);
        }

        //ap_input[1-19] ---->  sys_holding[101-119]
        if (xSemaphoreTake(master_ap.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_input_regs(&master_ap, 1, 19, word_result))
            {
                for (size_t i = 0; i < 19; i++)
                {
                    airsys_regs.holdings[i + 101] = word_result[i];
                }
            }
            xSemaphoreGive(master_ap.uart_mutex);
        }

        //ap_holding[1-16] ----> sys_holding[120-135]
        if (xSemaphoreTake(master_ap.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_holding_regs(&master_ap, 1, 16, word_result))
            {
                for (size_t i = 0; i < 16; i++)
                {
                    if (airsys_regs.holding_flags[120 + i] == false)
                    {
                        airsys_regs.holdings[120 + i] = word_result[i];
                    }
                }
            }
            xSemaphoreGive(master_ap.uart_mutex);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void modbus_ap_write_task(void *param)
{
    while (true)
    {
        if (xSemaphoreTake(master_ap.write_sem, portMAX_DELAY) == pdTRUE)
        {
            if (xSemaphoreTake(master_ap.uart_mutex, portMAX_DELAY) == pdTRUE)
            {
                for (size_t i = 0; i < 16; i++)
                {
                    if (airsys_regs.holding_flags[120 + i] == true)
                    {
                        master_write_single_holding_reg(&master_ap, i + 1, airsys_regs.holdings[120 + i]);
                        airsys_regs.holding_flags[120 + i] = false;
                    }
                }
                xSemaphoreGive(master_ap.uart_mutex);
            }
        }
    }
    vTaskDelete(NULL);
}
