// SPDX-License-Identifier: MIT
// Copyright (c) 2020-2022 The Pybricks Authors

// Software battery implementation for simulating battery in tests

#include <pbdrv/config.h>

#if PBDRV_CONFIG_BATTERY_TEST

#include <pbdrv/battery.h>
#include <pbio/error.h>

void pbdrv_battery_init(void) {
}

pbio_error_t pbdrv_battery_get_voltage_now(uint16_t *value) {
    *value = 7200;
    return PBIO_SUCCESS;
}

pbio_error_t pbdrv_battery_get_type(pbdrv_battery_type_t *value) {
    *value = PBDRV_BATTERY_TYPE_LIION;
    return PBIO_SUCCESS;
}

pbio_error_t pbdrv_battery_get_current_now(uint16_t *value) {
    *value = 0;
    return PBIO_ERROR_NOT_SUPPORTED;
}

pbio_error_t pbdrv_battery_get_temperature(uint32_t *value) {
    *value = 0;
    return PBIO_ERROR_NOT_SUPPORTED;
}

#endif // PBDRV_CONFIG_BATTERY_TEST
