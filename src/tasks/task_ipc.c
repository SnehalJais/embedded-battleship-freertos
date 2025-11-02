/**
 * @file task_ipc.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-09-03
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "task_ipc.h"
#include "cy_result.h"
#include "cyhal_hw_types.h"
#include "cyhal_uart.h"
#include "main.h"
#include "task_console.h"
#include <string.h>

#if defined(ECE353_FREERTOS)

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
QueueHandle_t Queue_IPC_Tx;

cyhal_uart_t IPC_Uart_Obj;
cyhal_uart_cfg_t IPC_Uart_Config =
    {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = CYHAL_UART_PARITY_NONE,
        .rx_buffer = NULL,
        .rx_buffer_size = 0};

uint32_t IPC_Actual_Baud;

/**
 * @brief
 * Simple checksum calculation function.  Take the XOR of all bytes
 * except the start and checksum bytes.
 * @param packet
 * @return __inline
 */
static __inline uint8_t calculate_checksum(volatile ipc_packet_t *packet)
{
    uint8_t checksum = 0;

    for (int i = 1; i < sizeof(ipc_packet_t) - 1; i++)
    {
        checksum ^= ((uint8_t *)packet)[i]; // XOR all bytes except start, checksum, and end
    }
    return checksum;
}

/**
 * @brief
 * Validates the given IPC packet by checking the start byte and checksum
 * @param packet
 * @return __inline
 */
bool validate_packet(volatile ipc_packet_t *packet)
{
    if (packet == NULL)
    {
        printf("Null packet received\n");
        return false; // Null packet
    }
    if (packet->start_byte != IPC_PACKET_START)
    {
        printf("Invalid start byte: 0x%02X\n", packet->start_byte);
        return false; // Invalid start byte
    }

    uint8_t calculated_checksum = calculate_checksum(packet);
    if (calculated_checksum != packet->checksum)
    {
        printf("Calculated: 0x%02X, Received: 0x%02X\n", calculated_checksum, packet->checksum);
        return false; // Checksum mismatch
    }

    return true; // Packet is valid
}

/**
 * @brief
 * This function is used to send a "fire" command to the opponent
 * @param row
 * @param col
 * @return true
 * @return false
 */
bool ipc_send_fire(uint8_t row, uint8_t col)
{
    ipc_packet_t packet = {0};
    packet.start_byte = IPC_PACKET_START;
    packet.cmd = IPC_CMD_FIRE;
    packet.load.fire.row = row;
    packet.load.fire.col = col;
    packet.checksum = calculate_checksum(&packet);

    // transmit the packet
    if (xQueueSend(Queue_IPC_Tx, &packet, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return false; // Failed to send packet to IPC Tx Task
    }

    return true; // Packet sent successfully
}

/**
 * @brief
 *  This function is used to send a "result" command to the opponent
 * @param result
 * @return true
 * @return false
 */
bool ipc_send_result(ipc_result_t result)
{
    ipc_packet_t packet = {0};
    packet.start_byte = IPC_PACKET_START;
    packet.cmd = IPC_CMD_RESULT;
    packet.load.result = result;
    packet.checksum = calculate_checksum(&packet);

    // transmit the packet
    if (xQueueSend(Queue_IPC_Tx, &packet, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return false; // Failed to send packet to IPC Tx Task
    }

    return true; // Packet sent successfully
}

/**
 * @brief
 * This function is used to send a "game control" command to the opponent
 * @param control
 * @return true
 * @return false
 */
bool ipc_send_game_control(ipc_game_control_t control)
{
    ipc_packet_t packet = {0};
    packet.start_byte = IPC_PACKET_START;
    packet.cmd = IPC_CMD_GAME_CONTROL;
    packet.load.game_control = control;
    packet.checksum = calculate_checksum(&packet);

    // transmit the packet
    if (xQueueSend(Queue_IPC_Tx, &packet, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return false; // Failed to send packet to IPC Tx Task
    }

    return true; // Packet sent successfully
}

/**
 * @brief
 * This function is used to send an "error" command to the opponent
 * @param error
 * @return true
 * @return false
 */
bool ipc_send_error(ipc_error_t error)
{
    ipc_packet_t packet = {0};
    packet.start_byte = IPC_PACKET_START;
    packet.cmd = IPC_CMD_ERROR;
    packet.load.error = error;
    packet.checksum = calculate_checksum(&packet);

    // transmit the packet
    if (xQueueSend(Queue_IPC_Tx, &packet, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return false; // Failed to send packet to IPC Tx Task
    }

    return true; // Packet sent successfully
}

/**
 * @brief
 * Interrupt handler for the IPC UART. This function handles both RX and TX interrupts.
 *
 * @param handler_arg Pointer to handler arguments (not used).
 * @param event The UART event that triggered the interrupt.
 */
void ipc_event_handler(void *handler_arg, cyhal_uart_event_t event)
{
    (void)handler_arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t c;
    static uint8_t raw_data[sizeof(ipc_packet_t)] = {0};
    static uint8_t raw_data_index = 0;

    if ((event & CYHAL_UART_IRQ_RX_NOT_EMPTY) == CYHAL_UART_IRQ_RX_NOT_EMPTY)
    {
        if (cyhal_uart_getc(&IPC_Uart_Obj, &c, 0) == CY_RSLT_SUCCESS)
        {
            /* first byte must be start */
            if (raw_data_index == 0 && c != IPC_PACKET_START)
            {
                return; // stay at 0, wait for a real start byte
            }

            /* store this byte */
            raw_data[raw_data_index++] = c;

            /* got a whole packet? */
            if (raw_data_index == sizeof(ipc_packet_t))
            {
                /* copy into the REAL produce buffer that lives in task_ipc_rx.c */
                memcpy((void *)IPC_Rx_Produce_Buffer,
                       (const void *)raw_data,
                       sizeof(ipc_packet_t));

                /* wake bottom-half */
                vTaskNotifyGiveFromISR(TaskHandle_IPC_Rx, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

                /* start over for next packet */
                raw_data_index = 0;
            }
            /* else: DO NOTHING — keep collecting bytes */
        }
    }
    if ((event & CYHAL_UART_IRQ_TX_EMPTY) == CYHAL_UART_IRQ_TX_EMPTY)
    {
        // no action needed here
    }
}

bool task_ipc_init(void)
{
    cy_rslt_t rslt;

    // Initialize the IPC UART
    rslt = cyhal_uart_init(
        &IPC_Uart_Obj,
        PIN_IPC_TX,
        PIN_IPC_RX,
        NC,
        NC,
        NULL,
        &IPC_Uart_Config);
    if (rslt != CY_RSLT_SUCCESS)
    {
        return false; // Initialization failed
    }

    rslt = cyhal_uart_set_baud(&IPC_Uart_Obj, 115200, &IPC_Actual_Baud);
    if (rslt != CY_RSLT_SUCCESS)
    {
        return false; // Initialization failed
    }

    cyhal_uart_clear(&IPC_Uart_Obj);

    // Register the UART handler
    cyhal_uart_register_callback(&IPC_Uart_Obj, ipc_event_handler, NULL);

    // Enable Rx Interrupts
    cyhal_uart_enable_event(
        &IPC_Uart_Obj,
        CYHAL_UART_IRQ_RX_NOT_EMPTY,
        INT_PRIORITY_IPC,
        true);

    if (task_ipc_resources_init_rx() == false)
    {
        return false; // Initialization failed
    }

    if (task_ipc_resources_init_tx() == false)
    {
        return false; // Initialization failed
    }

    return true; // Initialization successful
}
#endif