// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2023 The Pybricks Authors

// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2023 LEGO System A/S

/**
 * @addtogroup Observer pbio/observer: Servo state observer
 *
 * Luenberger state observer implementation to estimate motor speed.
 * @{
 */

#ifndef _PBIO_OBSERVER_H_
#define _PBIO_OBSERVER_H_

#include <stdint.h>

#include <pbio/control_settings.h>
#include <pbio/dcmotor.h>
#include <pbio/differentiator.h>
#include <pbio/angle.h>

/**
 * Device-type specific constants that describe the motor model.
 */
typedef struct _pbio_observer_model_t {
    int32_t d_angle_d_speed;
    int32_t d_speed_d_speed;
    int32_t d_current_d_speed;
    int32_t d_angle_d_current;
    int32_t d_speed_d_current;
    int32_t d_current_d_current;
    int32_t d_angle_d_voltage;
    int32_t d_speed_d_voltage;
    int32_t d_current_d_voltage;
    int32_t d_angle_d_torque;
    int32_t d_speed_d_torque;
    int32_t d_current_d_torque;
    int32_t d_voltage_d_torque;
    int32_t d_torque_d_voltage;
    int32_t d_torque_d_speed;
    int32_t d_torque_d_acceleration;
    int32_t torque_friction;
} pbio_observer_model_t;

/**
 * Configurable observer settings.
 */
typedef struct _pbio_observer_settings_t {
    /**
     * If this speed cannot be reached even with the maximum control signal
     * defined in actuation_max, the controller is stalled.
     */
    int32_t stall_speed_limit;
    /**
     * Minimum consecutive stall time before stall flag getter returns true.
     */
    uint32_t stall_time;
    /**
     * Lower limit of the feedback voltage where we cannot get useful stall
     * information because it is less than needed to overcome static friction.
     */
    int32_t feedback_voltage_negligible;
    /**
     * When the motor is stuck, the observer moves ahead and pushes back with
     * a feedback voltage. When it equal the given voltage, it is fully stuck.
     * This ratio (0--100) sets the threshold above which we say it is stalled.
     */
    int32_t feedback_voltage_stall_ratio;
    /**
     * Feedback gain (mV/deg) to correct the observer for low estimation errors.
     */
    int32_t feedback_gain_low;
    /**
     * Feedback gain (mV/deg) to correct the observer for high estimation error.
     */
    int32_t feedback_gain_high;
    /**
     * Threshold angle (mdeg) from which the higher observer feedback is used.
     */
    int32_t feedback_gain_threshold;
    /**
     * Speed (mdeg/s) below which the coulomb friction starts to linearly scale
     * to zero to avoid a sudden numeric switch in the friction force.
     */
    int32_t coulomb_friction_speed_cutoff;
} pbio_observer_settings_t;

/**
 * Motor state observer object.
 */
typedef struct _pbio_observer_t {
    /**
     * Angle state of observer (estimated system angle) in millidegrees
     */
    pbio_angle_t angle;
    /**
     * Speed state of observer (estimated system speed) in millidegrees/second.
     */
    int32_t speed;
    /**
     * Current state of observer (estimated system current) in tenths of milliAmperes: 10000 = 1A.
     */
    int32_t current;
    /**
     * Numeric angle differentiator used to verify estimated speed.
     */
    pbio_differentiator_t differentiator;
    /**
     * Latest speed value from angle differentiator.
     */
    int32_t speed_numeric;
    /**
     * Whether the motor is stalled according to the model.
     */
    bool stalled;
    /**
     * If stalled, this is the time that stall was first detected.
     */
    uint32_t stall_start;
    /**
     * Model parameters used by this model.
     */
    const pbio_observer_model_t *model;
    /**
     * Control settings, which includes stall settings.
     */
    pbio_observer_settings_t settings;
} pbio_observer_t;

// Observer state functions:

void pbio_observer_reset(pbio_observer_t *obs, const pbio_angle_t *angle);
void pbio_observer_get_estimated_state(const pbio_observer_t *obs, int32_t *speed_num, pbio_angle_t *angle_est, int32_t *speed_est);
void pbio_observer_update(pbio_observer_t *obs, uint32_t time, const pbio_angle_t *angle, pbio_dcmotor_actuation_t actuation, int32_t voltage);
bool pbio_observer_is_stalled(const pbio_observer_t *obs, uint32_t time, uint32_t *stall_duration);
int32_t pbio_observer_get_feedback_voltage(const pbio_observer_t *obs, const pbio_angle_t *angle);

// Model conversion functions:

int32_t pbio_observer_get_max_torque(void);
int32_t pbio_observer_get_feedforward_torque(const pbio_observer_model_t *model, int32_t rate_ref, int32_t acceleration_ref);
int32_t pbio_observer_torque_to_voltage(const pbio_observer_model_t *model, int32_t desired_torque);
int32_t pbio_observer_voltage_to_torque(const pbio_observer_model_t *model, int32_t voltage);

#endif // _PBIO_OBSERVER_H_

/** @} */
