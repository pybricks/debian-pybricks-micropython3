// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2020 The Pybricks Authors

#include <pbdrv/config.h>

#if PBDRV_CONFIG_COUNTER_NXT

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <pbio/util.h>
#include <pbio/port.h>
#include "counter.h"

#include <nxos/drivers/motors.h>

typedef struct {
    pbdrv_counter_dev_t *dev;
    uint8_t index;
} private_data_t;

static private_data_t private_data[PBDRV_CONFIG_COUNTER_NXT_NUM_DEV];

static pbio_error_t pbdrv_counter_nxt_get_angle(pbdrv_counter_dev_t *dev, int32_t *rotations, int32_t *millidegrees) {
    private_data_t *priv = dev->priv;

    int32_t degrees = nx_motors_get_tach_count(priv->index);
    *millidegrees = (degrees % 360) * 1000;
    *rotations = degrees / 360;

    return PBIO_SUCCESS;
}

static const pbdrv_counter_funcs_t pbdrv_counter_nxt_funcs = {
    .get_angle = pbdrv_counter_nxt_get_angle,
};

void pbdrv_counter_nxt_init(pbdrv_counter_dev_t *devs) {
    for (size_t i = 0; i < PBIO_ARRAY_SIZE(private_data); i++) {
        private_data_t *priv = &private_data[i];

        priv->index = i;

        // FIXME: assuming that these are the only counter devices
        // counter_id should be passed from platform data instead
        _Static_assert(PBDRV_CONFIG_COUNTER_NXT_NUM_DEV == PBDRV_CONFIG_COUNTER_NUM_DEV,
            "need to fix pbdrv_counter_nxt implementation to allow other counter devices");
        priv->dev = &devs[i];
        priv->dev->funcs = &pbdrv_counter_nxt_funcs;
        priv->dev->priv = priv;
    }
}

#endif // PBDRV_CONFIG_COUNTER_NXT
