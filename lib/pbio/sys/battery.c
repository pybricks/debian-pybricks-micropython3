// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022 The Pybricks Authors

// Provides battery status indication and shutdown on low battery.

// TODO: need to handle high battery current
// TODO: need to handle battery pack switch and Li-ion batteries for Technic Hub and NXT

#include <pbdrv/battery.h>
#include <pbdrv/charger.h>
#include <pbdrv/config.h>
#include <pbdrv/clock.h>
#include <pbdrv/usb.h>
#include <pbsys/status.h>

// period over which the battery voltage is averaged (in milliseconds)
#define BATTERY_PERIOD_MS       2500

// These values are for Alkaline (AA/AAA) batteries
#define BATTERY_OK_MV           6000    // 1.0V per cell
#define BATTERY_LOW_MV          5400    // 0.9V per cell
#define BATTERY_CRITICAL_MV     4800    // 0.8V per cell

// These values are for LEGO rechargeable battery packs
#define LIION_FULL_MV           8300    // 4.15V per cell
#define LIION_OK_MV             7200    // 3.6V per cell
#define LIION_LOW_MV            6800    // 3.4V per cell
#define LIION_CRITICAL_MV       6000    // 3.0V per cell

static uint32_t prev_poll_time;
static uint16_t avg_battery_voltage;

#if PBDRV_CONFIG_BATTERY_ADC_TYPE == 1
// special case to reduce code size on Move hub
#define battery_critical_mv BATTERY_CRITICAL_MV
#define battery_low_mv BATTERY_LOW_MV
#define battery_ok_mv BATTERY_OK_MV
#else
static uint16_t battery_critical_mv;
static uint16_t battery_low_mv;
static uint16_t battery_ok_mv;
#endif

/**
 * Initializes the system battery monitor.
 */
void pbsys_battery_init(void) {
    #if PBDRV_CONFIG_BATTERY_ADC_TYPE != 1
    pbdrv_battery_type_t type;
    if (pbdrv_battery_get_type(&type) == PBIO_SUCCESS && type == PBDRV_BATTERY_TYPE_LIION) {
        battery_critical_mv = LIION_CRITICAL_MV;
        battery_low_mv = LIION_LOW_MV;
        battery_ok_mv = LIION_OK_MV;
    } else {
        battery_critical_mv = BATTERY_CRITICAL_MV;
        battery_low_mv = BATTERY_LOW_MV;
        battery_ok_mv = BATTERY_OK_MV;
    }
    #endif

    pbdrv_battery_get_voltage_now(&avg_battery_voltage);
    // This is mainly for the Technic Hub. It seems that the first battery voltage
    // read is always low and causes the hub to shut down because of low battery
    // voltage even though the battery isn't that low.
    if (avg_battery_voltage < battery_critical_mv) {
        avg_battery_voltage = battery_ok_mv;
    }

    prev_poll_time = pbdrv_clock_get_ms();
}

/**
 * Polls the battery.
 *
 * This is called periodically to update the current battery state.
 */
void pbsys_battery_poll(void) {
    uint32_t now;
    uint32_t poll_interval;
    uint16_t battery_voltage;

    now = pbdrv_clock_get_ms();
    poll_interval = now - prev_poll_time;
    prev_poll_time = now;

    pbdrv_battery_get_voltage_now(&battery_voltage);

    avg_battery_voltage = (avg_battery_voltage * (BATTERY_PERIOD_MS - poll_interval)
        + battery_voltage * poll_interval) / BATTERY_PERIOD_MS;

    if (avg_battery_voltage <= battery_critical_mv) {
        pbsys_status_set(PBIO_PYBRICKS_STATUS_BATTERY_LOW_VOLTAGE_SHUTDOWN);
    } else if (avg_battery_voltage >= battery_low_mv) {
        pbsys_status_clear(PBIO_PYBRICKS_STATUS_BATTERY_LOW_VOLTAGE_SHUTDOWN);
    }

    if (avg_battery_voltage <= battery_low_mv) {
        pbsys_status_set(PBIO_PYBRICKS_STATUS_BATTERY_LOW_VOLTAGE_WARNING);
    } else if (avg_battery_voltage >= battery_ok_mv) {
        pbsys_status_clear(PBIO_PYBRICKS_STATUS_BATTERY_LOW_VOLTAGE_WARNING);
    }

    // REVISIT: we should be able to make this event driven rather than polled
    #if PBDRV_CONFIG_CHARGER

    pbdrv_usb_bcd_t bcd = pbdrv_usb_get_bcd();
    bool enable = bcd != PBDRV_USB_BCD_NONE;
    pbdrv_charger_limit_t limit;

    // REVISIT: The only current battery charger chip will automatically monitor
    // VBUS and limit the current if the VBUS voltage starts to drop, so these
    // limits are a bit looser than they could be.
    switch (bcd) {
        case PBDRV_USB_BCD_NONE:
            limit = PBDRV_CHARGER_LIMIT_NONE;
            break;
        case PBDRV_USB_BCD_STANDARD_DOWNSTREAM:
            limit = PBDRV_CHARGER_LIMIT_STD_MAX;
            break;
        default:
            limit = PBDRV_CHARGER_LIMIT_CHARGING;
            break;
    }

    pbdrv_charger_enable(enable, limit);

    #endif // PBDRV_CONFIG_CHARGER
}

/**
 * Tests if the battery is "full".
 *
 * This is only valid on hubs with a built-in battery charger.
 */
bool pbsys_battery_is_full(void) {
    return avg_battery_voltage >= LIION_FULL_MV;
}
