#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "hw.h"
#include "ble.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
    /* Initialize hardware */
    vibrator_init();
    
    /* Initialize BLE */
    vibrator_ble_init();

    return 0;
}
