// SPDX-License-Identifier: MIT
// Copyright (c) 2022 The Pybricks Authors

/**
 * @addtogroup UsbDriver Driver: USB
 * @{
 */

#ifndef _PBDRV_USB_H_
#define _PBDRV_USB_H_

#include <pbdrv/config.h>

/**
 * Indicates battery charging capabilites that were detected on a USB port.
 */
typedef enum {
    /** The USB cable is not connected (no VBUS). */
    PBDRV_USB_BCD_NONE,
    /** The USB cable is connected to a non-standard charger or PS/2 port. */
    PBDRV_USB_BCD_NONSTANDARD,
    /** The USB cable is connected to standard downstream port. */
    PBDRV_USB_BCD_STANDARD_DOWNSTREAM,
    /** The USB cable is connected to charging downstream port. */
    PBDRV_USB_BCD_CHARGING_DOWNSTREAM,
    /** The USB cable is connected to dedicated charging port. */
    PBDRV_USB_BCD_DEDICATED_CHARGING,
} pbdrv_usb_bcd_t;

#if PBDRV_CONFIG_USB

/**
 * Gets the result of the USB battery charger detection.
 * @return              The result.
 */
pbdrv_usb_bcd_t pbdrv_usb_get_bcd(void);

#else // PBDRV_CONFIG_USB

static inline pbdrv_usb_bcd_t pbdrv_usb_get_bcd(void) {
    return PBDRV_USB_BCD_NONE;
}

#endif // PBDRV_CONFIG_USB

#endif // _PBDRV_USB_H_

/** @} */
