// SPDX-License-Identifier: MIT
// Copyright (c) 2020 The Pybricks Authors

// OS-level hub built-in light matrix management.

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <contiki.h>

#include <pbdrv/led.h>
#include <pbio/error.h>
#include <pbio/event.h>
#include <pbio/light_matrix.h>
#include <pbio/util.h>
#include <pbsys/config.h>
#include <pbsys/status.h>

#include "../src/light/light_matrix.h"

#if PBSYS_CONFIG_HUB_LIGHT_MATRIX

typedef struct {
    /** Struct for PBIO light matrix implementation. */
    pbio_light_matrix_t light_matrix;
} pbsys_hub_light_matrix_t;

static pbsys_hub_light_matrix_t pbsys_hub_light_matrix_instance;

/** The hub built-in light matrix instance. */
pbio_light_matrix_t *pbsys_hub_light_matrix = &pbsys_hub_light_matrix_instance.light_matrix;

static pbio_error_t pbsys_hub_light_matrix_set_pixel(pbio_light_matrix_t *light_matrix, uint8_t row, uint8_t col, uint8_t brightness) {
    // REVISIT: currently hub light matrix is hard-coded as LED array at index 0
    // on all platforms
    pbdrv_led_array_dev_t *array;
    if (pbdrv_led_array_get_dev(0, &array) == PBIO_SUCCESS) {
        return pbdrv_led_array_set_brightness(array, row * light_matrix->size + col, brightness);
    }

    return PBIO_SUCCESS;
}

static const pbio_light_matrix_funcs_t pbsys_hub_light_matrix_funcs = {
    .set_pixel = pbsys_hub_light_matrix_set_pixel,
};

static void pbsys_hub_light_matrix_clear(void) {
    // turn of all pixels
    for (uint8_t r = 0; r < pbsys_hub_light_matrix->size; r++) {
        for (uint8_t c = 0; c < pbsys_hub_light_matrix->size; c++) {
            pbsys_hub_light_matrix_set_pixel(pbsys_hub_light_matrix, r, c, 0);
        }
    }
}

static void pbsys_hub_light_matrix_show_stop_sign(uint8_t brightness) {
    // 3x3 "stop sign" at top center of light matrix
    for (uint8_t r = 0; r < pbsys_hub_light_matrix->size; r++) {
        for (uint8_t c = 0; c < pbsys_hub_light_matrix->size; c++) {
            uint8_t b = r < 3 && c > 0 && c < 4 ? brightness: 0;
            pbsys_hub_light_matrix_set_pixel(pbsys_hub_light_matrix, r, c, b);
        }
    }
}

// Animation frame for on/off animation.
static uint32_t pbsys_hub_light_matrix_user_power_animation_next(pbio_light_animation_t *animation) {

    // Start at 2% and increment up to 100% in 7 steps.
    static uint8_t brightness = 2;
    static int8_t increment = 14;

    // Show the stop sign fading in/out.
    brightness += increment;
    pbsys_hub_light_matrix_show_stop_sign(brightness);

    // Stop at 100% and re-initialize so we can use this again for shutdown.
    if (brightness == 100 || brightness == 0) {
        pbio_light_animation_stop(&pbsys_hub_light_matrix->animation);
        brightness = 96;
        increment = -8;
    }
    return 40;
}

/**
 * Starts the power up and down animation. The first call makes it fade in the
 * stop sign. All subsequent calls are fade out.
 */
static void pbsys_hub_light_matrix_start_power_animation(void) {
    pbio_light_animation_init(&pbsys_hub_light_matrix->animation, pbsys_hub_light_matrix_user_power_animation_next);
    pbio_light_animation_start(&pbsys_hub_light_matrix->animation);
}

void pbsys_hub_light_matrix_init(void) {
    pbio_light_matrix_init(pbsys_hub_light_matrix, 5, &pbsys_hub_light_matrix_funcs);
    pbsys_hub_light_matrix_start_power_animation();
}

// Animation frame for program running animation.
static uint32_t pbsys_hub_light_matrix_user_program_animation_next(pbio_light_animation_t *animation) {
    // The indexes of pixels to light up
    static const uint8_t indexes[] = { 1, 2, 3, 8, 13, 12, 11, 6 };

    // Each pixel has a repeating brightness pattern of the form /\_ through
    // which we can cycle in 256 steps.
    static uint8_t cycle = 0;

    pbdrv_led_array_dev_t *array;
    if (pbdrv_led_array_get_dev(0, &array) == PBIO_SUCCESS) {
        for (size_t i = 0; i < PBIO_ARRAY_SIZE(indexes); i++) {
            // The pixels are spread equally across the pattern.
            uint8_t offset = cycle + i * (UINT8_MAX / PBIO_ARRAY_SIZE(indexes));
            uint8_t brightness = offset > 200 ? 0 : (offset < 100 ? offset : 200 - offset);

            // Set the brightness for this pixel
            pbdrv_led_array_set_brightness(array, indexes[i], brightness);
        }
        // This increment controls the speed of the pattern
        cycle += 9;
    }

    return 40;
}

void pbsys_hub_light_matrix_handle_event(process_event_t event, process_data_t data) {
    if (event == PBIO_EVENT_STATUS_SET) {
        pbio_pybricks_status_t status = (intptr_t)data;

        if (status == PBIO_PYBRICKS_STATUS_USER_PROGRAM_RUNNING) {
            // The user animation updates only a subset of pixels to save time,
            // so the rest must be cleared before it starts.
            pbsys_hub_light_matrix_clear();
            pbio_light_animation_init(&pbsys_hub_light_matrix->animation, pbsys_hub_light_matrix_user_program_animation_next);
            pbio_light_animation_start(&pbsys_hub_light_matrix->animation);
        } else if (status == PBIO_PYBRICKS_STATUS_SHUTDOWN_REQUEST && !pbsys_status_test(PBIO_PYBRICKS_STATUS_USER_PROGRAM_RUNNING)) {
            // If shutdown was requested and no program is running, start power
            // down animation. This makes it run while the system processes are
            // deinitializing. If it was running, we should wait for it to end
            // first, which is handled below to avoid a race condition.
            pbsys_hub_light_matrix_start_power_animation();
        }
    } else if (event == PBIO_EVENT_STATUS_CLEARED) {
        pbio_pybricks_status_t status = (intptr_t)data;

        // The user program has ended.
        if (status == PBIO_PYBRICKS_STATUS_USER_PROGRAM_RUNNING) {
            if (pbsys_status_test(PBIO_PYBRICKS_STATUS_SHUTDOWN_REQUEST)) {
                // If it ended due to forced shutdown, show power-off animation.
                pbsys_hub_light_matrix_start_power_animation();
            } else {
                // If it simply completed, show stop sign.
                pbsys_hub_light_matrix_show_stop_sign(100);
            }
        }
    }
}

#endif // PBSYS_CONFIG_HUB_LIGHT_MATRIX
