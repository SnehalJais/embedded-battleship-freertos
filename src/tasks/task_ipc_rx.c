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

    while (1)
    {
        // Wait for a FreeRTOS Task Notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        printf("IPC RX: Packet received, processing...\r\n");

        // Process the IPC Packet stored in the consume buffer
        // Validate packet once to avoid multiple validation calls
        bool is_valid = validate_packet(IPC_Rx_Consume_Buffer);

        if (!is_valid)
        {
            /* Packet validation failed - send error to opponent */
            printf("IPC RX: Packet validation FAILED! Sending IPC_ERROR_CHECKSUM...\r\n");
            extern bool ipc_send_error(ipc_error_t error);
            ipc_send_error(IPC_ERROR_CHECKSUM);
        }
        else if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_FIRE)
        {
            // Valid FIRE command
            uint8_t fire_row = IPC_Rx_Consume_Buffer->load.fire.row;
            uint8_t fire_col = IPC_Rx_Consume_Buffer->load.fire.col;
            printf("IPC RX Task       : Fire at row=%d, col=%d\n\r", fire_row, fire_col);
            
            /* Call handler function in hw05.c */
            extern void handle_incoming_fire(uint8_t fire_row, uint8_t fire_col);
            handle_incoming_fire(fire_row, fire_col);
        }
        else if (is_valid && IPC_Rx_Consume_Buffer->cmd == IPC_CMD_RESULT)
        {
            // Valid RESULT command
            const char *result_str = (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_MISS) ? "MISS" : (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_HIT) ? "HIT"
                                                                                                    : (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_SUNK)  ? "SUNK"
                                                                                                                                                               : "UNKNOWN";
            printf("IPC RX Task       : Result: %s\n\r", result_str);
            
            /* Update my hit/miss counters and opponent_board */
            extern uint16_t my_hits;
            extern uint16_t my_misses;
            extern uint8_t opponent_board[10][10];
            extern uint8_t last_fire_row;
            extern uint8_t last_fire_col;
            
            if (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_HIT || 
                IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_SUNK)
            {
                my_hits++;
                opponent_board[last_fire_row][last_fire_col] = 1;  /* Mark as HIT on opponent's board */
                printf("My hits: %d\r\n", my_hits);
            }
            else if (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_MISS)
            {
                my_misses++;
                opponent_board[last_fire_row][last_fire_col] = 2;  /* Mark as MISS on opponent's board */
                printf("My misses: %d\r\n", my_misses);
            }
            
            /* If opponent's ship was sunk, update LED counter */
            if (IPC_Rx_Consume_Buffer->load.result == IPC_RESULT_SUNK)
            {
                extern uint8_t opponent_ships_remaining;
                extern void update_opponent_ships_leds(uint8_t ships_remaining);
                
                printf("╔═══════════════════════════════════════════════════════════╗\r\n");
                printf("║ RECEIVED: IPC_RESULT_SUNK - OPPONENT SHIP SUNK!           ║\r\n");
                printf("╚═══════════════════════════════════════════════════════════╝\r\n");
                printf("  Ships remaining BEFORE: %d\r\n", opponent_ships_remaining);
                
                if (opponent_ships_remaining > 0)
                {
                    opponent_ships_remaining--;
                    printf("  Ships remaining AFTER:  %d\r\n", opponent_ships_remaining);
                    update_opponent_ships_leds(opponent_ships_remaining);
                    printf("  LEDs/EEPROM updated!\r\n");
                }
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
            if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_ACK)
            {
                extern bool ack_received;
                ack_received = true;
                printf("Received ACK from Player 2\r\n");
            }
            /* Handle PLAYER_READY (for later game phases) */
            if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PLAYER_READY)
            {
                extern bool opponent_ready;
                opponent_ready = true;
                printf("Received PLAYER_READY from opponent - opponent has placed all ships!\r\n");
            }
            /* Handle PASS_TURN - opponent passed their turn to me */
            if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_PASS_TURN)
            {
                extern uint8_t current_turn;
                extern uint8_t player_id;
                current_turn = player_id;  /* Set turn to my player ID */
                printf("Received PASS_TURN - now it's MY turn! (current_turn=%d)\r\n", current_turn);
            }
            /* Handle END_GAME - opponent lost, so I won */
            if (IPC_Rx_Consume_Buffer->load.game_control == IPC_GAME_CONTROL_END_GAME)
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