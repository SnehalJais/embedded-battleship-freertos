/**
 * @file console.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-06-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"
#include "console.h"
#include "cy_result.h"
#include "cy_retarget_io.h"
#include "cyhal_uart.h"

/* Initialize the SCB used to print/receive messages from the console */
void console_init(void)
{
    cy_rslt_t result;

    /* Initialize the retarget-io to use the UART */
    result = cy_retarget_io_init(PIN_CONSOLE_TX, PIN_CONSOLE_RX, 115200);

    if (result != CY_RSLT_SUCCESS)
    {
        /* Console initialization failed - continue without it */
        /* This allows the system to run even if UART is unavailable */
        return;
    }
}
