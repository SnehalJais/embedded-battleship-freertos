/**
 * @file buzzer.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "buzzer.h"
#include "cyhal.h"
#include "cyhal_pwm.h"

/* Clock support */
#include "cyhal_clock.h"

/* Static PWM object used by the buzzer driver */
static cyhal_pwm_t buzzer_pwm;
static bool buzzer_initialized = false;
static float buzzer_duty = 0.5f;
static uint32_t buzzer_freq = 3500; // default

/* Optional: clock object reserved for the PWM/peripheral used by the buzzer.
 * We allocate/reserve it when initializing the buzzer to improve frequency
 * stability on platforms where the HAL allows configuring the peripheral clock.
 * If allocation/configuration fails we fall back to the existing behavior.
 */
static cyhal_clock_t buzzer_clock;
static bool buzzer_clock_reserved = false;

cy_rslt_t buzzer_init(float duty, uint32_t frequency)
{
	cy_rslt_t rslt;
	buzzer_duty = duty;
	buzzer_freq = frequency;
	if (buzzer_initialized)
	{
		// Already initialized
		return CY_RSLT_SUCCESS;
	}

	/* Initialize the PWM object first */
	rslt = cyhal_pwm_init(&buzzer_pwm, PIN_BUZZER, NULL);
	if (rslt != CY_RSLT_SUCCESS)
	{
		return rslt;
	}

	rslt = cyhal_pwm_set_duty_cycle(&buzzer_pwm, buzzer_duty, buzzer_freq);
	if (rslt != CY_RSLT_SUCCESS)
	{
		cyhal_pwm_free(&buzzer_pwm);
		return rslt;
	}

	// Do not start by default
	buzzer_initialized = true;
	return CY_RSLT_SUCCESS;
}

void buzzer_on(void)
{
	if (!buzzer_initialized)
	{
		return;
	}

	cyhal_pwm_start(&buzzer_pwm);

	// Set the initial duty cycle
	cyhal_pwm_set_duty_cycle(&buzzer_pwm, buzzer_duty, buzzer_freq);	

}

void buzzer_off(void)
{
	if (!buzzer_initialized) return;
	cyhal_pwm_stop(&buzzer_pwm);
	// Drive pin low to ensure buzzer off
	//cyhal_gpio_write(PIN_BUZZER, false);
}

/**
 * Free buzzer resources including the reserved clock (if allocated).
 * Call this if you want to release hardware resources used by the buzzer.
 */
void buzzer_free(void)
{
	if (buzzer_initialized)
	{
		cyhal_pwm_free(&buzzer_pwm);
		buzzer_initialized = false;
	}

	if (buzzer_clock_reserved)
	{
		cyhal_clock_free(&buzzer_clock);
		buzzer_clock_reserved = false;
	}
}
