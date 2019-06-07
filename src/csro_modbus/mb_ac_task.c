
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
        if (master_read_discs(&master_ac, 1, 39, byte_result))
        {
            for (size_t i = 0; i < 39; i++)
            {
                airsys_regs.coils[i + 1] = byte_result[i];
            }
        }

        //ac_input[1-20] ---->  sys_holding[1-20]
        if (master_read_input_regs(&master_ac, 1, 20, word_result))
        {
            for (size_t i = 0; i < 20; i++)
            {
                airsys_regs.holdings[i + 1] = byte_result[i];
            }
        }

        //ac_coil[1] ---->  sys_coil[40]
        if (master_read_coils(&master_ac, 1, 1, byte_result) && (airsys_regs.coil_flags[40] != 1))
        {
            airsys_regs.coils[40] = byte_result[0];
        }

        //ac_holding[1] ----> sys_holding[21]
        if (master_read_holding_regs(&master_ac, 1, 1, word_result) && (airsys_regs.holding_flags[21] != 1))
        {
            airsys_regs.holdings[21] = word_result[0];
        }

        //ac_holding[36-39] ----> sys_holding[22-25]
        if (master_read_holding_regs(&master_ac, 36, 4, word_result))
        {
            airsys_regs.holdings[22] = word_result[0];
            airsys_regs.holdings[25] = word_result[3];
        }

        //ac_holding[61-62] ----> sys_holding[26-27]
        if (master_read_holding_regs(&master_ac, 36, 4, word_result))
        {
            if (airsys_regs.holding_flags[26] != 1)
            {
                airsys_regs.holdings[26] = word_result[0];
            }
            if (airsys_regs.holding_flags[27] != 1)
            {
                airsys_regs.holdings[27] = word_result[1];
            }
            if (airsys_regs.holdings[21] == 1)
            {
                airsys_regs.holdings[28] = airsys_regs.holdings[27];
            }
            else if (airsys_regs.holdings[21] == 2)
            {
                airsys_regs.holdings[28] = airsys_regs.holdings[26];
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
