#include "joystick.h"
#include "cy_result.h"
#include "cyhal_adc.h"

/* Static Global Variables */
static cyhal_adc_t joystick_adc_obj;
static cyhal_adc_channel_t joystick_adc_chan_x_obj;
static cyhal_adc_channel_t joystick_adc_chan_y_obj;
const cyhal_adc_channel_config_t channel_config = { .enable_averaging = false, .min_acquisition_ns = 220, .enabled = true };

const cyhal_adc_config_t joystick_config = {
    .continuous_scanning = true, 
    .resolution = 12, 
    .average_count = 1, 
    .average_mode_flags = 0,
    .ext_vref_mv = 0,
    .vneg = CYHAL_ADC_VNEG_VREF,
    .vref = CYHAL_ADC_REF_VDDA_DIV_2,
    .ext_vref = NC,
    .is_bypassed = false,
    .bypass_pin = NC
};

/* Public API */

/** Initialize the ADC channels connected to the Joystick
 *
 * @param - None
 */
cy_rslt_t joystick_init(void)
{
    cy_rslt_t rslt;

    /* Initialize ADC.*/
    rslt = cyhal_adc_init(&joystick_adc_obj, PIN_ANALOG_JOY_X, NULL);
    if(rslt != CY_RSLT_SUCCESS)
    {
        return rslt; // If the initialization fails, return the error code
    }

    /* Set the Reference Voltage to be VDDA */
    rslt = cyhal_adc_configure(&joystick_adc_obj, &joystick_config);
    if(rslt != CY_RSLT_SUCCESS)
    {
        return rslt; // If the initialization fails, return the error code
    }


    /* Initialize X direction */
    rslt = cyhal_adc_channel_init_diff(
        &joystick_adc_chan_x_obj, 
        &joystick_adc_obj, 
        PIN_ANALOG_JOY_X, 
        CYHAL_ADC_VNEG, 
        &channel_config
    );
    if(rslt != CY_RSLT_SUCCESS)
    {
        return rslt; // If the initialization fails, return the error code
    }

    /* Initialize Y direction */
    rslt = cyhal_adc_channel_init_diff(
        &joystick_adc_chan_y_obj, 
        &joystick_adc_obj, 
        PIN_ANALOG_JOY_Y, 
        CYHAL_ADC_VNEG, 
        &channel_config
    );

    if(rslt != CY_RSLT_SUCCESS)
    {
        return rslt; // If the initialization fails, return the error code
    }

    return CY_RSLT_SUCCESS; // Return success if all initializations completed
}

/** Read X direction of Joystick 
 *
 * @param - None
 */
uint16_t  joystick_read_x(void)
{
    /* ADD CODE */
    return cyhal_adc_read_u16(&joystick_adc_chan_x_obj);

}

/** Read Y direction of Joystick 
 *
 * @param - None
 */
uint16_t  joystick_read_y(void)
{
    /* ADD CODE */
    return cyhal_adc_read_u16(&joystick_adc_chan_y_obj);

}


/**
 * @brief 
 * Returns the current position of the joystick 
 * @return joystick_position_t 
 */
joystick_position_t joystick_get_pos(void)
{
    uint16_t x_val;
    uint16_t y_val;
    joystick_position_t position = JOYSTICK_POS_CENTER;
    
    /* ADD CODE */
    x_val = joystick_read_x();
    y_val = joystick_read_y();

    // Determine X direction: < 0.825V means moved, > 2.475V means moved
    bool x_left = (x_val > JOYSTICK_THRESH_X_LEFT_2P475V);
    bool x_right = (x_val < JOYSTICK_THRESH_X_RIGHT_0P825V);
    
    // Determine Y direction: < 0.825V means moved, > 2.475V means moved  
    bool y_up = (y_val > JOYSTICK_THRESH_Y_UP_2P475V);
    bool y_down = (y_val < JOYSTICK_THRESH_Y_DOWN_0P825V);
  
    if (x_left && y_up) {
        position = JOYSTICK_POS_UPPER_LEFT;
    }
    else if (x_right && y_up) {
        position = JOYSTICK_POS_UPPER_RIGHT;
    }
    else if (x_left && y_down) {
        position = JOYSTICK_POS_LOWER_LEFT;
    }
    else if (x_right && y_down) {
        position = JOYSTICK_POS_LOWER_RIGHT;
    }
    // Then cardinal directions
    else if (x_left) {
        position = JOYSTICK_POS_LEFT;
    }
    else if (x_right) {
        position = JOYSTICK_POS_RIGHT;
    }
    else if (y_up) {
        position = JOYSTICK_POS_UP;
    }
    else if (y_down) {
        position = JOYSTICK_POS_DOWN;
    }
    // Default to center if no thresholds exceeded
    else {
        position = JOYSTICK_POS_CENTER;
    }

    return position;
}
