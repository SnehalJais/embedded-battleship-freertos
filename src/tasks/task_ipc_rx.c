/**
 * @file task_console_rx.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"
#include <stdbool.h>

#if defined(ECE353_FREERTOS)
#include "task_ipc.h"
#include "task_console.h"

/* Globals */
TaskHandle_t TaskHandle_IPC_Rx = NULL;

/* Use a double buffering strategy for IPC packets */
static volatile ipc_packet_t IPC_Rx_Buffer0;
static volatile ipc_packet_t IPC_Rx_Buffer1;

volatile ipc_packet_t *volatile IPC_Rx_Produce_Buffer = &IPC_Rx_Buffer0;
volatile ipc_packet_t *volatile IPC_Rx_Consume_Buffer = &IPC_Rx_Buffer0;

/**
 * @brief
 *
 * This task is used to process received IPC packets.  The task will block
 * on a FreeRTOS Task Notification.  When a notification is received,
 * the task will process the IPC packet stored in the consume buffer.
 *
 * For validation purposes, the task will print out the contents of the
 * received IPC packet to the console.
 *
 * @param arg
 * Unused parameter
 */
void task_ipc_rx(void *param)
{
    /* Forward declaration of occupied_board from hw05.c */
    extern uint8_t occupied_board[10][10];

    while (1)
    {
        // Wait for a FreeRTOS Task Notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Process the IPC Packet stored in the consume buffer
        // Validate packet once to avoid multiple validation calls
        bool is_valid = validate_packet(IPC_Rx_Consume_Buffer);

        if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_FIRE)
        {
            // Valid FIRE command
            uint8_t fire_row = IPC_Rx_Consume_Buffer->load.fire.row;
            uint8_t fire_col = IPC_Rx_Consume_Buffer->load.fire.col;
            printf("IPC RX Task       : Fire at row=%d, col=%d\n\r", fire_row, fire_col);
            
            // Check if opponent hit one of my ships
            ipc_result_t result;
            extern uint16_t opponent_hits;
            extern uint16_t opponent_misses;
            
            if (fire_row < 10 && fire_col < 10 && occupied_board[fire_row][fire_col] == 1)
            {
                // HIT - opponent hit my ship at this coordinate
                result = IPC_RESULT_HIT;
                occupied_board[fire_row][fire_col] = 2; // Mark as hit (optional: for tracking)
                opponent_hits++;
                printf("  --> HIT on my ship! (Opponent hits: %d)\n\r", opponent_hits);
            }
            else
            {
                // MISS - no ship at this coordinate
                result = IPC_RESULT_MISS;
                opponent_misses++;
                printf("  --> MISS! (Opponent misses: %d)\n\r", opponent_misses);
            }
            
            // Send result back to opponent
            ipc_send_result(result);
        }
        else if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_RESULT)
        {
            // Valid RESULT command - this is the response to MY fire command
            extern uint16_t my_hits;
            extern uint16_t my_misses;
            
            const char *result_str = (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_MISS) ? "MISS" : (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_HIT) ? "HIT"
                                                                                                    : (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_SUNK)  ? "SUNK"
                                                                                                                                                               : "UNKNOWN";
            printf("IPC RX Task       : Result: %s\n\r", result_str);
            
            /* Increment my hit/miss counters based on opponent's response */
            if (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_HIT)
            {
                extern uint16_t my_hits;
                my_hits++;
                printf("  --> I hit! (My total hits: %d)\n\r", my_hits);
            }
            else if (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_MISS)
            {
                extern uint16_t my_misses;
                my_misses++;
                printf("  --> I missed! (My total misses: %d)\n\r", my_misses);
            }
            
            /* Check for END_GAME result to detect win condition */
            if (IPC_Rx_Consume_Buffer->load.result == IPC_GAME_CONTROL_END_GAME)
            {
                extern bool game_over;
                extern bool i_won;
                printf("GAME END DETECTED!\n\r");
                game_over = true;
                i_won = true;  /* If opponent sent END_GAME, it means I won */
            }
        }
        else if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_GAME_CONTROL)
        {
            // Valid GAME CONTROL command
            const char *control_str = (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_NEW_GAME) ? "CONTROL_NEW_GAME" : (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PLAYER_READY) ? "CONTROL_PLAYER_READY"
                                                                                                                                 : (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PLAYER_ALIVE)   ? "CONTROL_PLAYER_ALIVE"
                                                                                                                                 : (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PASS_TURN)      ? "CONTROL_PASS_TURN"
                                                                                                                                 : (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_ACK)            ? "CONTROL_ACK"
                                                                                                                                 : (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_END_GAME)       ? "CONTROL_END_GAME"
                                                                                                                                                                                                                 : "UNKNOWN";
            printf("IPC RX Task       : Game Control: %s\n\r", control_str);
            
            /* Handle NEW_GAME - Player 2 receives this */
            if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_NEW_GAME)
            {
                extern bool opponent_ready;
                extern uint8_t player_id;
                opponent_ready = true;  /* Signal that opponent pressed SW1 */
                player_id = 1;          /* I am Player 2 */
                printf("Received NEW_GAME - I am Player 2\r\n");
                /* Send ACK back to Player 1 */
                ipc_send_game_control(IPC_GAME_CONTROL_ACK);
            }
            /* Handle ACK - Player 1 receives this */
            else if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_ACK)
            {
                extern bool ack_received;
                ack_received = true;
                printf("Received ACK from Player 2\r\n");
            }
            /* Handle PLAYER_READY (for later game phases) */
            else if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PLAYER_READY)
            {
                extern bool opponent_ready;
                opponent_ready = true;
            }
            /* Handle END_GAME - opponent lost, so I won */
            else if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_END_GAME)
            {
                extern bool game_over;
                extern bool i_won;
                printf("Received END_GAME from opponent - I WON!\r\n");
                game_over = true;
                i_won = true;  /* Opponent sent END_GAME means they lost, I won */
            }
        }
        else if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_ERROR)
        {
            // Valid ERROR command
            const char *error_str = (IPC_Rx_Consume_Buffer->load.error == IPC_ERROR_CHECKSUM) ? "ERROR_CHECKSUM" : (IPC_Rx_Consume_Buffer->load.error == IPC_ERROR_COORD_INVALID) ? "ERROR_COORD_INVALID"
                                                                                                               : (IPC_Rx_Consume_Buffer->load.error == IPC_ERROR_COORD_OCCUPIED)  ? "ERROR_COORD_OCCUPIED"
                                                                                                               : (IPC_Rx_Consume_Buffer->load.error == IPC_ERROR_SYSTEM_FAILURE)  ? "ERROR_SYSTEM_FAILURE"
                                                                                                                                                                                  : "UNKNOWN";
            printf("IPC RX Task       : Error: %s\n\r", error_str);
        }
        else
        {
            // Invalid packet or unknown command
            printf("  INVALID PACKET OR UNKNOWN COMMAND\n\r");
        }
        /* swap the buffers so the ISR can write to the other one next time */
        volatile ipc_packet_t *tmp = IPC_Rx_Produce_Buffer;
        IPC_Rx_Produce_Buffer = IPC_Rx_Consume_Buffer;
        IPC_Rx_Consume_Buffer = tmp;
    }
}

bool task_ipc_resources_init_rx(void)
{
    // Create the IPC Rx Task
    BaseType_t task_ipc_rx_status = xTaskCreate(
        task_ipc_rx,       // Function that implements the task.
        "IPC Rx Task",     // Text name for the task.
        IPC_STACK_SIZE,    // Stack size in words, not bytes.
        NULL,              // Parameter passed into the task.
        IPC_PRIORITY,      // Priority at which the task is created.
        &TaskHandle_IPC_Rx // Used to pass out the created task's handle.
    );

    if (task_ipc_rx_status != pdPASS)
    {
        return false;
    }

    return true;
}

#endif