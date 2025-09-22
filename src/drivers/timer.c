/**
 * @file timer.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2024-08-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "timer.h"
#include <complex.h>
#include "cyhal_timer.h"
#include "cyhal_timer_impl.h"
cy_rslt_t timer_init(cyhal_timer_t *timer_obj, cyhal_timer_cfg_t *timer_cfg, uint32_t ticks, void *Handler)
{
    cy_rslt_t rslt;
    timer_cfg->period = ticks;
    timer_cfg->direction = CYHAL_TIMER_DIR_UP;
    timer_cfg->is_compare = false;
    timer_cfg->is_continuous = true;
    timer_cfg->value = 0;

    rslt = cyhal_timer_init(timer_obj, NC, NULL);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }
    rslt = cyhal_timer_configure(timer_obj, timer_cfg);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }
    rslt = cyhal_timer_set_frequency(timer_obj, 100000000);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }
   cyhal_timer_register_callback(timer_obj, Handler, NULL);

    cyhal_timer_enable_event(timer_obj, CYHAL_TIMER_IRQ_TERMINAL_COUNT, 3,true);
    rslt = cyhal_timer_start(timer_obj);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }
    return rslt; // Return the result of the initialization
}