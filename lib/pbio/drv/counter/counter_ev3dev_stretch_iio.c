// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2020 The Pybricks Authors

// ev3dev-stretch PRU/IIO Quadrature Encoder Counter driver
//
// This driver uses the PRU quadrature encoder found in ev3dev-stretch.

#include <pbdrv/config.h>

#if PBDRV_CONFIG_COUNTER_EV3DEV_STRETCH_IIO

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <libudev.h>

#include <pbio/util.h>
#include "counter.h"

#include <pbdrv/clock.h>

#define DEBUG 0
#if DEBUG
#define dbg_err(s) perror(s)
#else
#define dbg_err(s)
#endif

typedef struct {
    pbdrv_counter_dev_t *dev;
    FILE *count;
    uint32_t time_us_last;
    int32_t rotations_last;
    int32_t millidegrees_last;
} private_data_t;

static private_data_t private_data[PBDRV_CONFIG_COUNTER_EV3DEV_STRETCH_IIO_NUM_DEV];

static pbio_error_t pbdrv_counter_ev3dev_stretch_iio_get_angle(pbdrv_counter_dev_t *dev, int32_t *rotations, int32_t *millidegrees) {
    private_data_t *priv = dev->priv;

    if (!priv->count) {
        return PBIO_ERROR_NO_DEV;
    }

    uint32_t time_now = pbdrv_clock_get_us();

    // If values were recently read, return those again.
    // This reduces unnecessary I/O operations.
    if (time_now - priv->time_us_last < 2000) {
        *rotations = priv->rotations_last;
        *millidegrees = priv->millidegrees_last;
        return PBIO_SUCCESS;
    }

    if (fseek(priv->count, 0, SEEK_SET) == -1) {
        return PBIO_ERROR_IO;
    }

    int32_t count;
    if (fscanf(priv->count, "%d", &count) == EOF) {
        return PBIO_ERROR_IO;
    }

    // ev3dev stretch provides 720 counts per rotation.
    *rotations = count / 720;
    *millidegrees = (count % 720) * 500;

    // Updated cached values
    priv->time_us_last = time_now;
    priv->rotations_last = *rotations;
    priv->millidegrees_last = *millidegrees;

    return PBIO_SUCCESS;
}

static const pbdrv_counter_funcs_t pbdrv_counter_ev3dev_stretch_iio_funcs = {
    .get_angle = pbdrv_counter_ev3dev_stretch_iio_get_angle,
};

void pbdrv_counter_ev3dev_stretch_iio_init(pbdrv_counter_dev_t *devs) {
    char buf[256];
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *entry;

    udev = udev_new();
    if (!udev) {
        dbg_err("Failed to get udev context");
        return;
    }

    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        dbg_err("Failed to get udev context");
        goto free_udev;
    }

    if ((errno = udev_enumerate_add_match_subsystem(enumerate, "iio")) < 0) {
        dbg_err("udev_enumerate_add_match_subsystem failed");
        goto free_enumerate;
    }

    if ((errno = udev_enumerate_add_match_property(enumerate, "OF_NAME", "ev3-tacho-rpmsg")) < 0) {
        dbg_err("udev_enumerate_add_match_property failed");
        goto free_enumerate;
    }

    if ((errno = udev_enumerate_scan_devices(enumerate) < 0)) {
        dbg_err("udev_enumerate_scan_devices failed");
        goto free_enumerate;
    }

    entry = udev_enumerate_get_list_entry(enumerate);
    if (!entry) {
        dbg_err("udev_enumerate_get_list_entry failed");
        goto free_enumerate;
    }


    for (size_t i = 0; i < PBIO_ARRAY_SIZE(private_data); i++) {
        private_data_t *priv = &private_data[i];

        snprintf(buf, sizeof(buf), "%s/in_count%d_raw", udev_list_entry_get_name(entry), (int)i);
        priv->count = fopen(buf, "r");
        if (!priv->count) {
            dbg_err("failed to open count attribute");
            continue;
        }

        setbuf(priv->count, NULL);

        // FIXME: assuming that these are the only counter devices
        // counter_id should be passed from platform data instead
        _Static_assert(PBDRV_CONFIG_COUNTER_EV3DEV_STRETCH_IIO_NUM_DEV == PBDRV_CONFIG_COUNTER_NUM_DEV,
            "need to fix counter_ev3dev_stretch_iio implementation to allow other counter devices");
        priv->dev = &devs[i];
        priv->dev->funcs = &pbdrv_counter_ev3dev_stretch_iio_funcs;
        priv->dev->priv = priv;
    }

free_enumerate:
    udev_enumerate_unref(enumerate);
free_udev:
    udev_unref(udev);
}

#endif // PBDRV_CONFIG_COUNTER_EV3DEV_STRETCH_IIO
