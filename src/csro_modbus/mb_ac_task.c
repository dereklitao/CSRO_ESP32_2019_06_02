
#include "mb_config.h"
#include "csro_devices/csro_devices.h"

modbus_master master_ap;
modbus_master master_ac;
modbus_slave slave_hmi;
device_regs airsys_regs;

void modbus_ac_read_task(void *param)
{
    while (true)
    {
        uint8_t byte_result[100];
        uint16_t word_result[100];

        //ac_disc[1-39] ---->  sys_coil[1-39]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_discs(&master_ac, 1, 39, byte_result))
            {
                for (size_t i = 0; i < 39; i++)
                {
                    airsys_regs.coils[i + 1] = byte_result[i];
                }
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }

        //ac_input[1-20] ---->  sys_holding[1-20]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_input_regs(&master_ac, 1, 20, word_result))
            {
                for (size_t i = 0; i < 20; i++)
                {
                    airsys_regs.holdings[i + 1] = word_result[i];
                }
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }

        //ac_coil[1] ---->  sys_coil[40]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_coils(&master_ac, 1, 1, byte_result) && (airsys_regs.coil_flags[40] == false))
            {
                airsys_regs.coils[40] = byte_result[0];
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }

        //ac_holding[1] ----> sys_holding[21]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_holding_regs(&master_ac, 1, 1, word_result) && (airsys_regs.holding_flags[21] == false))
            {
                airsys_regs.holdings[21] = word_result[0];
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }

        //ac_holding[36-39] ----> sys_holding[22-25]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_holding_regs(&master_ac, 36, 4, word_result))
            {
                airsys_regs.holdings[22] = word_result[0];
                airsys_regs.holdings[25] = word_result[3];
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }

        //ac_holding[61-62] ----> sys_holding[26-27]
        if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (master_read_holding_regs(&master_ac, 61, 2, word_result))
            {
                airsys_regs.holdings[26] = word_result[0];
                airsys_regs.holdings[27] = word_result[1];
                if (airsys_regs.holding_flags[28] == false)
                {
                    if (airsys_regs.holdings[21] == 1)
                    {
                        airsys_regs.holdings[28] = airsys_regs.holdings[27];
                    }
                    else if (airsys_regs.holdings[21] == 2)
                    {
                        airsys_regs.holdings[28] = airsys_regs.holdings[26];
                    }
                }
            }
            xSemaphoreGive(master_ac.uart_mutex);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void modbus_ac_write_task(void *param)
{
    while (true)
    {
        if (xSemaphoreTake(master_ac.write_sem, portMAX_DELAY) == pdTRUE)
        {
            if (xSemaphoreTake(master_ac.uart_mutex, portMAX_DELAY) == pdTRUE)
            {
                if (airsys_regs.coil_flags[40] == true) //power switch
                {
                    master_write_single_coil(&master_ac, 1, airsys_regs.coils[40]);
                    airsys_regs.coil_flags[40] = false;
                }
                if (airsys_regs.holding_flags[21] == true) //mode
                {
                    master_write_single_holding_reg(&master_ac, 1, airsys_regs.holdings[21]);
                    airsys_regs.holding_flags[21] = false;
                }
                if (airsys_regs.holding_flags[28] == true) //set temperature
                {
                    if (airsys_regs.holdings[21] == 1)
                    {
                        master_write_single_holding_reg(&master_ac, 62, airsys_regs.holdings[28]);
                    }
                    else if (airsys_regs.holdings[21] == 2)
                    {
                        master_write_single_holding_reg(&master_ac, 61, airsys_regs.holdings[28]);
                    }
                    airsys_regs.holding_flags[28] = false;
                }
                xSemaphoreGive(master_ac.uart_mutex);
            }
        }
    }
    vTaskDelete(NULL);
}
