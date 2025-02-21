#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include "hw.h"

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

LOG_MODULE_REGISTER(hw, LOG_LEVEL_DBG);

void vibrator_init(void)
{
    if (!pwm_is_ready_dt(&servo)) {
        LOG_ERR("PWM device %s not ready", servo.dev->name);
        return;
    }
    
    // Initialize with 0% duty cycle (off)
    pwm_set_pulse_dt(&servo, 0);
    LOG_INF("PWM initialized (range: %u-%u μs)", min_pulse, max_pulse);
}

// The current list of commands is as follows:

// D0 - Identify device & firmware version
// D1 - Identify TCode version
// D2 - List available axes and associated user range preferences
// DSTOP - Stop device

#define DEVICE_VERSION "1.0.0"
#define TCODE_VERSION "0.3"
void handle_device_cmd(const char *cmd)
{
    switch (cmd[1])
    {
    case '0':
        LOG_INF("Device: %s, Firmware: %s", CONFIG_BT_DEVICE_NAME, DEVICE_VERSION);
        break;
    case '1':
        LOG_INF("TCode version: %s", TCODE_VERSION);
        break;
    case '2':
        LOG_INF("Available axes: None");
        LOG_INF("User range preferences: 0-100");
        break;
    default:
        //DSTOP
        if (strcmp(cmd, "DSTOP") == 0)
        {
            LOG_INF("Stopping device");
            pwm_set_pulse_dt(&servo, 0);
        }
        else
        {
            LOG_WRN("Unknown device command: %s", cmd);
        } 

        break;
    }
}

void handle_vibration_cmd(const char *cmd, int len)
{
    //V + CHANNEL + digits + \n
   if (strlen(cmd) < 4)
    {
        LOG_ERR("Invalid vibration command: %s", cmd);
        return;
    }
    if (cmd[1] != '0')
    {
        LOG_ERR("Invalid vibration channel (we only have 0): %c", cmd[1]);
        return;
    }

    // Extract vibration intensity
    const char *mag_str = cmd + 2;
    size_t mag_len = len - 3;
    float magnitude = 0;
    int divisor = 1;
    //Magnitude is 0.DIGITS
    for (size_t i = 0; i < mag_len; i++)
    {
        if (mag_str[i] < '0' || mag_str[i] > '9')
        {
            LOG_ERR("Invalid vibration intensity: %s", cmd);
            return;
        }
        magnitude = magnitude * 10 + (mag_str[i] - '0');
        divisor *= 10;
    }
    magnitude /= divisor;

    // Set PWM duty cycle
    uint32_t pulse = min_pulse + (max_pulse - min_pulse) * magnitude;

    pwm_set_pulse_dt(&servo, pulse);
    LOG_INF("Vibration channel %c set to %.2f%% (pulse: %u μs)", cmd[1], magnitude * 100, pulse);

}


void vibrator_control(const char *cmd)
{
    int len = strlen(cmd);
    if (cmd == NULL || len == 0 || cmd[len - 1] != '\n') {
        LOG_ERR("Received empty command: %s", cmd);
        return;
    }

    switch (cmd[0])
    {
        case 'D':
        handle_device_cmd(cmd);
        break;
    case 'V':
        handle_vibration_cmd(cmd, len);
        break;
    
    default:
        break;
    }
    
}
