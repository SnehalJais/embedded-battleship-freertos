/**
 * @file task_ipc_tx.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-09-03
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "circular_buffer.h"
#include "main.h"

#if defined(ECE353_FREERTOS)
#include "task_ipc.h"

/* Global Variables */
TaskHandle_t TaskHandle_IPC_Tx = NULL;

/**
 * @brief
 * This task is used to process outgoing IPC packets.
 * @param param
 * Unused parameter
 */
void task_ipc_tx(void *param)
{
    ipc_packet_t packet;
    
 

    while (1)
    {
        if (xQueueReceive(Queue_IPC_Tx, &packet, portMAX_DELAY) == pdTRUE)
        {
            const char *cmd_name = (packet.cmd == IPC_CMD_FIRE) ? "FIRE" :
                                   (packet.cmd == IPC_CMD_RESULT) ? "RESULT" :
                                   (packet.cmd == IPC_CMD_GAME_CONTROL) ? "GAME_CONTROL" :
                                   (packet.cmd == IPC_CMD_ERROR) ? "ERROR" : "UNKNOWN";
            printf("IPC TX Task: Transmitting packet - CMD: %s (%d), checksum: 0x%02X\r\n", 
                   cmd_name, packet.cmd, packet.checksum);
            
            for(int i = 0; i < sizeof(ipc_packet_t); i++)
            {
                // Transmit the next byte
                cyhal_uart_putc(&IPC_Uart_Obj, ((uint8_t *)&packet)[i]);
            }
            printf("IPC TX Task: Packet transmission complete (%d bytes)\r\n", sizeof(ipc_packet_t));
        }
    }
}

bool task_ipc_resources_init_tx(void)
{
    /* Create the FreeRTOS Queue */
    Queue_IPC_Tx = xQueueCreate(IPC_TX_QUEUE_LENGTH, sizeof(ipc_packet_t));
    if (Queue_IPC_Tx == NULL)
    {
        return false;
    }

    /* Start the IPC Tx Task */
    BaseType_t task_ipc_tx_status = xTaskCreate(
        task_ipc_tx,       // Function that implements the task.
        "IPC Tx Task",     // Text name for the task.
        IPC_STACK_SIZE,    // Stack size in words, not bytes.
        NULL,              // Parameter passed into the task.
        IPC_PRIORITY,      // Priority at which the task is created.
        &TaskHandle_IPC_Tx // Used to pass out the created task's handle.
    );

    if (task_ipc_tx_status != pdPASS)
    {
        return false;
    }
    else
    {
        return true; // Resources initialized successfully
    }
}
#endif