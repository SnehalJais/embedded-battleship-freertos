/**
 * @file hw05.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-06-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "cyhal_hw_types.h"
#include "main.h"

#if defined(HW05)
#include "drivers.h"
#include "devices.h"
#include "task_console.h"
#include "task_io_expander.h"
#include "task_light_sensor.h"
#include "task_temp_sensor.h"
#include "task_eeprom.h"
#include "task_imu.h"
#include "task_lcd.h"
#include "task_buttons.h"
#include "task_joystick.h"
#include "task_buzzer.h"
#include "battleship.h"
#include "task_ipc.h"
#include "rtos_events.h"

char APP_DESCRIPTION[] = "ECE353: HW05 - FreeRTOS CLI";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
#define EEPROM_SHIPS_ADDR 0x0000 /* EEPROM address to store opponent ships count */

/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/
void update_opponent_ships_leds(uint8_t ships_remaining);

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
cyhal_i2c_t *I2C_Obj = NULL;
cyhal_spi_t *SPI_Obj = NULL;

SemaphoreHandle_t Semaphore_I2C = NULL;
SemaphoreHandle_t Semaphore_SPI = NULL;

QueueHandle_t Queue_System_Control_Responses = NULL;
QueueHandle_t Queue_Sensor_Responses;
QueueHandle_t xQueue_LCD_response = NULL;
/* xQueue_LCD is defined in task_lcd.c */
extern QueueHandle_t xQueue_LCD;
extern QueueHandle_t Queue_position;

/* Game state variables */
uint8_t player_id = 255;       /* 0=Player1, 1=Player2, 255=unassigned */
bool opponent_ready = false;   /* Flag set when opponent sends NEW_GAME */
bool ack_received = false;     /* Flag set when opponent sends ACK */
uint8_t next_first_player = 0; /* 0 = I go first next game, 1 = opponent goes first */
bool game_over = false;        /* Flag set when game ends (someone won) */
bool i_won = false;            /* Flag set if I won the game */
uint8_t current_turn = 0;      /* 0 = Player 0's turn, 1 = Player 1's turn (alternates during gameplay) */
EventGroupHandle_t ECE353_RTOS_Events = NULL;

/* Board border color - determined by player ID (Blue for Player 1, Red for Player 2) */
uint16_t board_border_color = LCD_COLOR_BLUE; /* Will be set in initialize_game_players() */
/* Hit and Miss counters for game play */
uint16_t my_hits = 0;                 /* Number of times I hit opponent's ships */
uint16_t my_misses = 0;               /* Number of times I missed */
uint16_t opponent_hits = 0;           /* Number of times opponent hit my ships */
uint16_t opponent_misses = 0;         /* Number of times opponent missed */
uint8_t opponent_ships_remaining = 5; /* Number of opponent's ships still alive (tracked in EEPROM) */

/* Ship placement board - tracks where your ships are located */
/* 0 = empty, 1-5 = ship ID placed here */
/* This is used by task_ipc_rx.c to check opponent's fire commands */
uint8_t occupied_board[10][10] = {0};

/* Track which tiles have been hit (turned red) - 0 = not hit, 1 = hit */
uint8_t hit_tiles[10][10] = {0};

/* Track opponent's board - tiles you've fired at and the results */
/* 0 = unknown/never fired, 1 = hit, 2 = miss */
uint8_t opponent_board[10][10] = {0};

/* Track last fire coordinates so RESULT handler knows which tile to update */
uint8_t last_fire_row = 0;
uint8_t last_fire_col = 0;

/* Track hits on each ship - ship_hit_count[ship_id-1] = number of hits */
uint8_t ship_hit_count[5] = {0};           /* 5 ships, 0 hits initially */
uint8_t ship_lengths[5] = {2, 3, 3, 4, 5}; /* Destroyer, Submarine, Cruiser, Battleship, Carrier (matches placement order) */
bool light_mode;                           // checking if light mode or dark mode
#define LIGHT_THRESHOLD 200

/* Board tile color - determined at game start and stays consistent */
uint16_t board_tile_fill_color = LCD_COLOR_BLACK; /* Default to black (dark mode) */

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_system_control(void *arg);
void light_mode_sensor(void);
void handle_incoming_fire(uint8_t fire_row, uint8_t fire_col);
void task_gameplay(void);
void draw_initial_board(void);
void initialize_game_players(void);
void draw_battleship_board(void);
void task_ship_placement(void);
bool battleship_check_light_threshold(void);

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
/**
 * @brief
 * Draw the initial empty board with hits/misses display
 * Called once at the start of gameplay before ship placement
 */
void draw_initial_board(void)
{
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;
    lcd_console_payload_t *console_payload;

    /* Draw empty board */
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    vTaskDelay(pdMS_TO_TICKS(200));

    /* Display hits/misses on right side */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload = &lcd_msg.payload.console;
    console_payload->x_offset = 210;
    console_payload->y_offset = 50;
    console_payload->message = "Hits: 0";
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Display misses below hits */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload->x_offset = 210;
    console_payload->y_offset = 100;
    console_payload->message = "Misses: 0";
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    vTaskDelay(pdMS_TO_TICKS(200));
}

/**
 * @brief Handle incoming FIRE command from opponent
 * @param fire_row Row coordinate of the fire
 * @param fire_col Column coordinate of the fire
 *
 * Checks if the fire hit a ship, updates the board, and sends the result back
 */
void handle_incoming_fire(uint8_t fire_row, uint8_t fire_col)
{
    /* Check if this position has a ship */
    uint8_t ship_id = occupied_board[fire_row][fire_col];

    if (ship_id > 0) /* Hit! (ship_id is 1-5) */
    {
        printf("HIT on ship %d at (%d,%d)!\r\n", ship_id, fire_row, fire_col);

        /* Increment hit count for this ship */
        ship_hit_count[ship_id - 1]++;

        /* Mark this tile as hit */
        hit_tiles[fire_row][fire_col] = 1;

        /* Draw red tile on MY board to show hit */
        lcd_msg_t lcd_msg;
        lcd_cmd_status_t status;
        lcd_msg.command = LCD_CMD_DRAW_TILE;
        lcd_msg.response_queue = xQueue_LCD_response;
        lcd_msg.payload.battleship.row = fire_row;
        lcd_msg.payload.battleship.col = fire_col;
        lcd_msg.payload.battleship.fill_color = LCD_COLOR_RED;
        lcd_msg.payload.battleship.border_color = LCD_COLOR_RED;
        xQueueSend(xQueue_LCD, &lcd_msg, 0);
        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

        /* Check if ship is sunk */
        if (ship_hit_count[ship_id - 1] == ship_lengths[ship_id - 1])
        {
            printf("  ⚓⚓⚓ SHIP %d IS SUNK! (hits: %d == length: %d) ⚓⚓⚓\r\n",
                   ship_id, ship_hit_count[ship_id - 1], ship_lengths[ship_id - 1]);
            printf("  Sending IPC_RESULT_SUNK...\r\n");
            ipc_send_result(IPC_RESULT_SUNK);

            /* Check if this was my last ship - count how many ships are fully sunk */
            uint8_t ships_sunk = 0;
            for (uint8_t i = 0; i < 5; i++)
            {
                if (ship_hit_count[i] >= ship_lengths[i])
                {
                    ships_sunk++;
                }
            }

            if (ships_sunk == 5) /* All my ships are destroyed */
            {
                extern bool game_over;
                extern bool i_won;
                printf("  ╔═══════════════════════════════════════════╗\r\n");
                printf("  ║ ALL MY SHIPS DESTROYED - I LOST!          ║\r\n");
                printf("  ╚═══════════════════════════════════════════╝\r\n");
                printf("  Sending IPC_GAME_CONTROL_END_GAME...\r\n");
                game_over = true;
                i_won = false;
                ipc_send_game_control(IPC_GAME_CONTROL_END_GAME);
                printf("  END_GAME signal sent!\r\n");
            }
        }
        else if (ship_hit_count[ship_id - 1] > ship_lengths[ship_id - 1])
        {
            /* Ship already sunk - this is a redundant hit on a dead ship */
            printf("  Ship %d already sunk - Sending IPC_RESULT_HIT (redundant hit)\r\n", ship_id);
            ipc_send_result(IPC_RESULT_HIT);
        }
        else
        {
            ipc_send_result(IPC_RESULT_HIT);
        }
    }
    else /* Miss */
    {
        printf("MISS at (%d,%d)\r\n", fire_row, fire_col);

        /* Don't draw anything for misses - keep the board as is */

        ipc_send_result(IPC_RESULT_MISS);
    }
}

/**
 * @brief
 * Initialize game players - wait for SW1 press, determine player roles, handle ACK
 * Sets player_id (0 or 1) and next_first_player
 */
void initialize_game_players(void)
{
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;
    lcd_console_payload_t *console_payload = NULL;

    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Display "Press SW1 to Start" */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload = &lcd_msg.payload.console;
    console_payload->x_offset = 50;
    console_payload->y_offset = 120;
    console_payload->message = "Press SW1 to Start";
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Wait for SW1 press or opponent ready */
    EventBits_t button_event = 0;
    while (player_id == 255)
    {
        /* Wait for SW1 button event on this board */
        button_event = xEventGroupWaitBits(ECE353_RTOS_Events,
                                           ECE353_RTOS_EVENTS_SW1,
                                           pdTRUE,  /* Clear the bit */
                                           pdFALSE, /* Don't wait for all bits */
                                           pdMS_TO_TICKS(100));

        if (button_event & ECE353_RTOS_EVENTS_SW1)
        {
            /* SW1 was pressed on this board - I am Player 1 */
            player_id = 0;
            vTaskDelay(pdMS_TO_TICKS(100)); /* Small delay */
            printf("I pressed SW1 first - I am Player 1\r\n");

            /* Send NEW_GAME to opponent board */
            ipc_send_game_control(IPC_GAME_CONTROL_NEW_GAME);
            break; /* Exit the wait loop */
        }

        /* Check if opponent already sent NEW_GAME */
        if (opponent_ready)
        {
            /* Opponent pressed SW1 first - I am Player 2 */
            player_id = 1;
            printf("Opponent pressed SW1 first - I am Player 2\r\n");
            break;
        }
    }

    printf("Player ID set to: %d\r\n", player_id);

    /* Set border color based on player ID */
    if (player_id == 0)
    {
        board_border_color = LCD_COLOR_BLUE; /* Player 1 (0) = Blue border */
        printf("Border color: BLUE (Player 1)\r\n");
    }
    else
    {
        board_border_color = LCD_COLOR_RED; /* Player 2 (1) = Red border */
        printf("Border color: RED (Player 2)\r\n");
    }

    /* Set up rotation for next game: opposite of current */
    if (player_id == 0)
    {
        /* I am Player 1 this game, opponent is Player 2 */
        /* Next game, opponent should go first (be Player 1) */
        next_first_player = 1; /* Opponent goes first next game */
    }
    else
    {
        /* I am Player 2 this game, opponent is Player 1 */
        /* Next game, I should go first (be Player 1) */
        next_first_player = 0; /* I go first next game */
    }

    /* If Player 1, wait for ACK from opponent */
    if (player_id == 0)
    {
        // printf("Waiting for opponent ACK...\r\n");
        // uint32_t timeout = 5000; /* 5 second timeout in ms */
        // uint32_t elapsed = 0;
        // while (!ack_received && elapsed < timeout)
        // {
        //     vTaskDelay(pdMS_TO_TICKS(100));
        //     elapsed += 100;
        // }

        if (ack_received)
        {
            printf("Received ACK from opponent! Starting ship placement...\r\n");
        }
        else
        {
            printf("No ACK received from opponent (timeout). Continuing anyway...\r\n");
        }
    }
    else if (player_id == 1)
    {
        printf("I am Player 2 - Opponent is Player 1. Ready for ship placement...\r\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief
 * Draw the battleship board for ship placement
 * Clears screen and draws the empty board grid
 */
void draw_battleship_board(void)
{
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;

    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Draw the board once before ship placement */
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));
}

/**
 * @brief Update IO Expander LEDs and EEPROM to show opponent ships remaining
 * @param ships_remaining Number of opponent ships still alive (0-5)
 *
 * LED pattern:
 * 5 ships: LEDs 0-4 on (0b00011111 = 0x1F)
 * 4 ships: LEDs 0-3 on (0b00001111 = 0x0F)
 * 3 ships: LEDs 0-2 on (0b00000111 = 0x07)
 * 2 ships: LEDs 0-1 on (0b00000011 = 0x03)
 * 1 ship:  LED 0 on    (0b00000001 = 0x01)
 * 0 ships: All off     (0b00000000 = 0x00)
 *
 * Also stores the count in EEPROM at address EEPROM_SHIPS_ADDR
 */
void update_opponent_ships_leds(uint8_t ships_remaining)
{
    uint8_t led_pattern = 0x00;

    /* Create LED pattern based on ships remaining */
    if (ships_remaining >= 5)
        led_pattern = 0x1F; /* All 5 LEDs on */
    else if (ships_remaining == 4)
        led_pattern = 0x0F; /* 4 LEDs on */
    else if (ships_remaining == 3)
        led_pattern = 0x07; /* 3 LEDs on */
    else if (ships_remaining == 2)
        led_pattern = 0x03; /* 2 LEDs on */
    else if (ships_remaining == 1)
        led_pattern = 0x01; /* 1 LED on */
    else
        led_pattern = 0x00; /* All LEDs off */

    /* Write to IO Expander LEDs */
    system_sensors_io_expander_write(NULL, IOXP_ADDR_OUTPUT_PORT, led_pattern);

    /* Write count to EEPROM */
    system_sensors_eeprom_write(NULL, EEPROM_SHIPS_ADDR, ships_remaining);

    printf("Updated LEDs and EEPROM: %d opponent ships remaining (pattern: 0x%02X)\r\n",
           ships_remaining, led_pattern);
}

void light_mode_sensor(void)
{
    uint16_t ambient_light = 0;

    system_sensors_get_light(Queue_Sensor_Responses, &ambient_light);
    printf("Ambient light reading: %d (threshold: %d)\r\n", ambient_light, LIGHT_THRESHOLD);

    if ((LIGHT_THRESHOLD) < ambient_light)
    {
        light_mode = true;
        board_tile_fill_color = LCD_COLOR_WHITE; /* Light mode = white tiles */
        printf("LIGHT MODE: Using WHITE tiles\r\n");
    }
    else
    {
        light_mode = false;
        board_tile_fill_color = LCD_COLOR_BLACK; /* Dark mode = black tiles */
        printf("DARK MODE: Using BLACK tiles\r\n");
    }
}

/**
 * @brief
 * Attack phase gameplay - joystick targeting and SW1 to fire at opponent board
 * Called after ship placement is complete
 */
void task_gameplay(void)
{
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;

    /* ATTACK PHASE - Game loop */
    uint32_t game_timeout = 300000; /* 5 minutes for testing */
    uint32_t game_elapsed = 0;
    char score_buffer[32];
    lcd_console_payload_t *console_payload;

    /* Target coordinates for attack */
    uint8_t target_row = 0, target_col = 0;
    uint8_t prev_target_row = 0, prev_target_col = 0;
    uint32_t last_joystick_move = 0;
    const uint32_t JOYSTICK_DEBOUNCE = 300;
    bool cursor_needs_redraw = true; /* Flag to redraw cursor on movement */

    /* Draw YOUR board with your ships */
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Draw all your placed ships in green */
    for (uint8_t row = 0; row < 10; row++)
    {
        for (uint8_t col = 0; col < 10; col++)
        {
            if (occupied_board[row][col] > 0) /* Ship present */
            {
                lcd_msg.command = LCD_CMD_DRAW_TILE;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = row;
                lcd_msg.payload.battleship.col = col;
                lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                lcd_msg.payload.battleship.border_color = board_border_color;
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
            }
            else if (hit_tiles[row][col] == 1) /* Hit on your board */
            {
                lcd_msg.command = LCD_CMD_DRAW_TILE;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = row;
                lcd_msg.payload.battleship.col = col;
                lcd_msg.payload.battleship.fill_color = LCD_COLOR_RED;
                lcd_msg.payload.battleship.border_color = LCD_COLOR_BLUE; // this line changed
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
            }
        }
    }

    /* Draw initial yellow cursor at (0,0) */
    lcd_msg.command = LCD_CMD_DRAW_TILE;
    lcd_msg.response_queue = xQueue_LCD_response;
    lcd_msg.payload.battleship.row = target_row;
    lcd_msg.payload.battleship.col = target_col;

    /* Determine fill color at cursor start position */
    if (hit_tiles[target_row][target_col] == 1)
    {
        lcd_msg.payload.battleship.fill_color = LCD_COLOR_RED;
    }
    else if (occupied_board[target_row][target_col] > 0)
    {
        lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
    }
    else
    {
        lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
    }
    lcd_msg.payload.battleship.border_color = LCD_COLOR_YELLOW;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    while (!game_over && game_elapsed < game_timeout)
    {

        if (battleship_check_light_threshold())
        {
            /* Light threshold crossed - redraw all empty tiles with new color (preserves placed ships) */
            for (uint8_t row = 0; row < 10; row++)
            {
                for (uint8_t col = 0; col < 10; col++)
                {
                    if (occupied_board[row][col] == 0) /* Only redraw empty tiles */
                    {
                        lcd_msg.command = LCD_CMD_DRAW_TILE;
                        lcd_msg.response_queue = xQueue_LCD_response;
                        lcd_msg.payload.battleship.row = row;
                        lcd_msg.payload.battleship.col = col;
                        lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                        lcd_msg.payload.battleship.border_color = board_border_color;
                        xQueueSend(xQueue_LCD, &lcd_msg, 0);
                        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
                    }
                }
            }
        }
        /* Check if light threshold has changed - updates board_tile_fill_color if needed */
        battleship_check_light_threshold();

        /* Display my hits count */
        lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
        lcd_msg.response_queue = xQueue_LCD_response;
        console_payload = &lcd_msg.payload.console;
        console_payload->x_offset = 210;
        console_payload->y_offset = 50;
        sprintf(score_buffer, "Hits: %d", my_hits);
        console_payload->message = score_buffer;
        console_payload->length = strlen(console_payload->message);
        xQueueSend(xQueue_LCD, &lcd_msg, 0);
        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

        /* Display my misses count */
        lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
        lcd_msg.response_queue = xQueue_LCD_response;
        console_payload->x_offset = 210;
        console_payload->y_offset = 100;
        sprintf(score_buffer, "Misses: %d", my_misses);
        console_payload->message = score_buffer;
        console_payload->length = strlen(console_payload->message);
        xQueueSend(xQueue_LCD, &lcd_msg, 0);
        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

        /* Display current turn and target */
        lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
        lcd_msg.response_queue = xQueue_LCD_response;
        console_payload->x_offset = 210;
        console_payload->y_offset = 150;
        if (current_turn == player_id)
        {
            console_payload->message = "YOURS";
        }
        else
        {
            console_payload->message = "OPPNT";
        }
        console_payload->length = strlen(console_payload->message);
        xQueueSend(xQueue_LCD, &lcd_msg, 0);
        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

        /* If it's my turn, use joystick to aim and SW1 to fire */
        if (current_turn == player_id)
        {
            /* Check joystick for target movement */
            joystick_position_t joystick_pos;
            uint32_t current_time = xTaskGetTickCount();

            if (xQueueReceive(Queue_position, &joystick_pos, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                if ((current_time - last_joystick_move) >= pdMS_TO_TICKS(JOYSTICK_DEBOUNCE))
                {
                    /* Save previous position for clearing */
                    prev_target_row = target_row;
                    prev_target_col = target_col;

                    switch (joystick_pos)
                    {
                    case JOYSTICK_POS_LEFT:
                    case JOYSTICK_POS_UPPER_LEFT:
                    case JOYSTICK_POS_LOWER_LEFT:
                        target_col = (target_col > 0) ? target_col - 1 : 9;
                        cursor_needs_redraw = true;
                        last_joystick_move = current_time;
                        break;
                    case JOYSTICK_POS_RIGHT:
                    case JOYSTICK_POS_UPPER_RIGHT:
                    case JOYSTICK_POS_LOWER_RIGHT:
                        target_col = (target_col < 9) ? target_col + 1 : 0;
                        cursor_needs_redraw = true;
                        last_joystick_move = current_time;
                        break;
                    case JOYSTICK_POS_UP:
                        target_row = (target_row > 0) ? target_row - 1 : 9;
                        cursor_needs_redraw = true;
                        last_joystick_move = current_time;
                        break;
                    case JOYSTICK_POS_DOWN:
                        target_row = (target_row < 9) ? target_row + 1 : 0;
                        cursor_needs_redraw = true;
                        last_joystick_move = current_time;
                        break;
                    default:
                        break;
                    }
                }
            }

            /* Redraw cursor if it moved */
            if (cursor_needs_redraw)
            {
                /* Restore previous tile - redraw it with its original appearance without the yellow border */
                lcd_msg.command = LCD_CMD_DRAW_TILE;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = prev_target_row;
                lcd_msg.payload.battleship.col = prev_target_col;

                /* Restore original colors based on board state */
                if (hit_tiles[prev_target_row][prev_target_col] == 1)
                {
                    /* Hit on your board - keep red (priority over ship color) */
                    lcd_msg.payload.battleship.fill_color = LCD_COLOR_RED;
                }
                else if (occupied_board[prev_target_row][prev_target_col] > 0)
                {
                    /* Your own ship - keep yellow */
                    lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                }
                else
                {
                    /* Empty tile - restore board color */
                    lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                }

                /* Restore original border color (board border, not yellow) */
                lcd_msg.payload.battleship.border_color = board_border_color;

                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));

                /* Draw new cursor position with yellow border only - don't change fill */
                lcd_msg.command = LCD_CMD_DRAW_TILE;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = target_row;
                lcd_msg.payload.battleship.col = target_col;

                /* Keep the fill color as-is, only change border to yellow */
                if (occupied_board[target_row][target_col] > 0)
                {
                    /* Your own ship - keep yellow fill */
                    lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                }
                else if (hit_tiles[target_row][target_col] == 1)
                {
                    /* Hit on your board - keep red fill */
                    lcd_msg.payload.battleship.fill_color = LCD_COLOR_RED;
                }
                else
                {
                    /* Empty tile - keep board color fill */
                    lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                }

                /* Only change border to yellow for the cursor */
                lcd_msg.payload.battleship.border_color = LCD_COLOR_YELLOW;
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

                cursor_needs_redraw = false;
            }

            /* Check for SW1 press to fire */
            EventBits_t button_event = xEventGroupWaitBits(ECE353_RTOS_Events,
                                                           ECE353_RTOS_EVENTS_SW1,
                                                           pdTRUE,  /* Clear the bit */
                                                           pdFALSE, /* Don't wait for all bits */
                                                           pdMS_TO_TICKS(10));

            if (button_event & ECE353_RTOS_EVENTS_SW1)
            {
                static uint32_t fire_count = 0;
                fire_count++;

                /* Save fire coordinates for RESULT handler */
                last_fire_row = target_row;
                last_fire_col = target_col;

                /* Send fire command with target coordinates */
                printf(">>> FIRE #%lu at row=%d, col=%d <<<\r\n", fire_count, target_row, target_col);

                if (!ipc_send_fire(target_row, target_col))
                {
                    printf("ERROR: Failed to send fire command!\r\n");
                }
                else
                {
                    printf("Fire command #%lu sent successfully!\r\n", fire_count);
                }

                /* Pass turn to opponent */
                if (!ipc_send_game_control(IPC_GAME_CONTROL_PASS_TURN))
                {
                    printf("ERROR: Failed to send PASS_TURN!\r\n");
                }
                else
                {
                    printf("PASS_TURN sent successfully!\r\n");
                }
                current_turn = 1 - current_turn;
                printf("Current turn updated to: %d\r\n", current_turn);

                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
        game_elapsed += 100;

        /* Check win condition - if opponent has no ships left, I won */
        if (opponent_ships_remaining == 0)
        {
            printf("All opponent ships sunk! I WON!\r\n");
            game_over = true;
            i_won = true;
        }

        /* Check lose condition - if I have no ships left, I lost */
        extern uint8_t player_id;
        uint8_t my_ships_remaining = 0;
        for (uint8_t row = 0; row < 10; row++)
        {
            for (uint8_t col = 0; col < 10; col++)
            {
                if (occupied_board[row][col] > 0) /* Found a ship tile */
                {
                    if (hit_tiles[row][col] == 0) /* Not hit yet */
                    {
                        my_ships_remaining++;
                    }
                }
            }
        }

        if (my_ships_remaining == 0 && (occupied_board[0][0] > 0 || occupied_board[1][0] > 0)) /* All my ships are sunk */
        {
            printf("  Unhit ship tiles remaining: %d\r\n", my_ships_remaining);
            printf("  Sending IPC_GAME_CONTROL_END_GAME...\r\n");
            game_over = true;
            i_won = false;
            ipc_send_game_control(IPC_GAME_CONTROL_END_GAME);
            printf("  END_GAME signal sent!\r\n");
        }
        /* game_over will be set by IPC RX task when opponent wins */
    }

    /* Display game end message */
    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    if (i_won)
    {
        printf("YOU WIN!\r\n");
    }
    else
    {
        printf("YOU LOSE!\r\n");
        /* END_GAME already sent when ships were destroyed - don't send again */
    }

    /* Reassign console_payload after changing message type */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload = &lcd_msg.payload.console; /* Reassign pointer */
    console_payload->x_offset = 100;
    console_payload->y_offset = 100;
    if (i_won)
    {
        console_payload->message = "YOU WIN!";
    }
    else
    {
        console_payload->message = "YOU LOSE!";
    }
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    vTaskDelay(pdMS_TO_TICKS(3000)); /* Show result for 3 seconds */

    /* Wait for opponent to acknowledge game end */
    vTaskDelay(pdMS_TO_TICKS(2000));

    /* Reset game state for next game */
    printf("Resetting game state for next game...\r\n");
    game_over = false;
    i_won = false;
    opponent_ready = false;
    ack_received = false;
    my_hits = 0;
    my_misses = 0;
    opponent_hits = 0;
    opponent_misses = 0;
    opponent_ships_remaining = 5;
    current_turn = 0;

    /* Clear all game boards */
    for (uint8_t i = 0; i < 10; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            occupied_board[i][j] = 0;
            hit_tiles[i][j] = 0;
            opponent_board[i][j] = 0;
        }
    }

    /* Clear ship hit counts */
    for (uint8_t i = 0; i < 5; i++)
    {
        ship_hit_count[i] = 0;
    }

    /* Set who goes first next game based on next_first_player */
    player_id = next_first_player;

    /* Update border color based on new player ID */
    if (player_id == 0)
    {
        board_border_color = LCD_COLOR_BLUE;
        printf("Next game: I am Player 1 (Blue border)\r\n");
    }
    else
    {
        board_border_color = LCD_COLOR_RED;
        printf("Next game: I am Player 2 (Red border)\r\n");
    }

    /* Setup rotation for game after next */
    if (player_id == 0)
    {
        next_first_player = 1;
    }
    else
    {
        next_first_player = 0;
    }

    /* Start new game by calling task_ship_placement again */
    printf("Starting new game!\r\n");
    task_ship_placement();
    vTaskDelay(pdMS_TO_TICKS(500));
    task_gameplay(); /* Recursively call gameplay for next game */
}

/**
 * @brief
 * Handle ship placement phase - move ships with IMU, rotate with SW1, place with SW2
 */
void task_ship_placement(void)
{
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;

    /* Ship placement using battleship.c functions */
    battleship_type_t ship_types[5] = {
        BATTLESHIP_TYPE_DESTROYER,  /* Smallest ship - length 2 */
        BATTLESHIP_TYPE_SUBMARINE,  /* Length 3 */
        BATTLESHIP_TYPE_CRUISER,    /* Length 3 */
        BATTLESHIP_TYPE_BATTLESHIP, /* Length 4 */
        BATTLESHIP_TYPE_CARRIER,    /* Largest ship - length 5 */
    };

    uint8_t current_ship = 0;
    uint8_t ships_placed = 0;
    uint8_t cursor_col = 0, cursor_row = 0;
    bool ship_orientation = true; /* true = horizontal */
    uint16_t imu_data[3];
    int16_t accel_x, accel_y;
    uint32_t last_move_time = 0;
    QueueHandle_t imu_response_queue;

    const int16_t IMU_THRESHOLD = 2500; /* Threshold for IMU movement */
    const uint32_t MOVE_INTERVAL = 300; /* 0.3 seconds */

    /* Track cursor tile positions to know what to clear */
    uint8_t cursor_tiles[5][2]; /* Store col,row of cursor tiles */
    uint8_t cursor_tile_count = 0;

    /* Clear battleship board and create IMU queue */
    battleship_board_clear();
    imu_response_queue = xQueueCreate(1, sizeof(device_response_msg_t));

    /* Draw the board once before ship placement */
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    uint8_t prev_cursor_col = 0, prev_cursor_row = 0; /* Track previous position for clearing */
    bool first_draw = true;                           /* Flag for first ship display - don't clear on first draw */

    while (ships_placed < 5)
    {
        /* Check if light threshold has changed - updates board_tile_fill_color if needed */
        if (battleship_check_light_threshold())
        {
            /* Light threshold crossed - redraw all empty tiles with new color (preserves placed ships) */
            for (uint8_t row = 0; row < 10; row++)
            {
                for (uint8_t col = 0; col < 10; col++)
                {
                    if (occupied_board[row][col] == 0) /* Only redraw empty tiles */
                    {
                        lcd_msg.command = LCD_CMD_DRAW_TILE;
                        lcd_msg.response_queue = xQueue_LCD_response;
                        lcd_msg.payload.battleship.row = row;
                        lcd_msg.payload.battleship.col = col;
                        lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                        lcd_msg.payload.battleship.border_color = board_border_color;
                        xQueueSend(xQueue_LCD, &lcd_msg, 0);
                        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
                    }
                }
            }
        }

        bool ship_moved = false;

        /* Read IMU data - continuous movement based on tilt */
        if (system_sensors_imu_read(imu_response_queue, imu_data))
        {
            accel_x = (int16_t)imu_data[0];
            accel_y = (int16_t)imu_data[1];

            /* Check timing for movement - only allow movement after MOVE_INTERVAL */
            uint32_t current_time = xTaskGetTickCount();
            if (current_time - last_move_time >= MOVE_INTERVAL)
            {
                /* Check X-axis movement - inverted for intuitive control */
                if (accel_x > IMU_THRESHOLD)
                {
                    /* Save previous position before moving */
                    if (!first_draw)
                    {
                        prev_cursor_col = cursor_col;
                        prev_cursor_row = cursor_row;
                    }
                    cursor_col = (cursor_col == 0) ? 9 : cursor_col - 1;
                    last_move_time = current_time;
                    // printf("Ship moved LEFT to col %d\r\n", cursor_col);
                    ship_moved = true;
                }
                else if (accel_x < -IMU_THRESHOLD)
                {
                    /* Save previous position before moving */
                    if (!first_draw)
                    {
                        prev_cursor_col = cursor_col;
                        prev_cursor_row = cursor_row;
                    }
                    cursor_col = (cursor_col + 1) % 10;
                    last_move_time = current_time;
                    printf("Ship moved RIGHT to col %d\r\n", cursor_col);
                    ship_moved = true;
                }
                else if (accel_y > IMU_THRESHOLD)
                {
                    /* Save previous position before moving */
                    if (!first_draw)
                    {
                        prev_cursor_col = cursor_col;
                        prev_cursor_row = cursor_row;
                    }
                    cursor_row = (cursor_row + 1) % 10;
                    last_move_time = current_time;
                    printf("Ship moved DOWN to row %d\r\n", cursor_row);
                    ship_moved = true;
                }
                else if (accel_y < -IMU_THRESHOLD)
                {
                    /* Save previous position before moving */
                    if (!first_draw)
                    {
                        prev_cursor_col = cursor_col;
                        prev_cursor_row = cursor_row;
                    }
                    cursor_row = (cursor_row == 0) ? 9 : cursor_row - 1;
                    last_move_time = current_time;
                    printf("Ship moved UP to row %d\r\n", cursor_row);
                    ship_moved = true;
                }
            }
        }

        /* If ship moved, clear only the previous yellow cursor ship tiles */
        if (ship_moved && !first_draw)
        {
            uint8_t ship_length = battleship_get_ship_length(ship_types[current_ship]);

            for (uint8_t i = 0; i < ship_length; i++)
            {
                uint8_t clear_col = ship_orientation ? (prev_cursor_col + i) : prev_cursor_col;
                uint8_t clear_row = ship_orientation ? prev_cursor_row : (prev_cursor_row + i);

                /* Only clear if this tile doesn't have a placed ship */
                if (occupied_board[clear_row][clear_col] == 0)
                {
                    /* Draw blue board tile to cover the yellow cursor ship */
                    lcd_msg.command = LCD_CMD_DRAW_TILE;
                    lcd_msg.response_queue = xQueue_LCD_response;
                    lcd_msg.payload.battleship.row = clear_row;
                    lcd_msg.payload.battleship.col = clear_col;
                    lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                    lcd_msg.payload.battleship.border_color = board_border_color;
                    xQueueSend(xQueue_LCD, &lcd_msg, 0);
                    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));
                }
            }
        }

        /* Draw ship at current position only if it moved or on first draw */
        if (ship_moved || first_draw)
        {
            if (current_ship < 5)
            {
                lcd_msg.command = LCD_CMD_DRAW_SHIP;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = cursor_row;
                lcd_msg.payload.battleship.col = cursor_col;
                lcd_msg.payload.battleship.type = ship_types[current_ship];
                lcd_msg.payload.battleship.horizontal = ship_orientation;
                lcd_msg.payload.battleship.border_color = BATTLESHIP_CURSOR_COLOR;
                lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

                first_draw = false; /* Mark first draw complete */
            }
        }

        /* Save current position for next move */
        prev_cursor_col = cursor_col;
        prev_cursor_row = cursor_row;

        /* Check buttons - use WaitBits to catch events more reliably */
        EventBits_t button_event = xEventGroupWaitBits(
            ECE353_RTOS_Events,
            ECE353_RTOS_EVENTS_SW1 | ECE353_RTOS_EVENTS_SW2,
            pdFALSE,          /* Don't clear bits yet */
            pdFALSE,          /* Don't wait for all bits */
            pdMS_TO_TICKS(10) /* 10ms timeout to not block IMU reading */
        );

        /* SW1 - Rotate ship */
        if (button_event & ECE353_RTOS_EVENTS_SW1)
        {
            xEventGroupClearBits(ECE353_RTOS_Events, ECE353_RTOS_EVENTS_SW1);

            if (current_ship < 5)
            {
                /* Clear all tiles the current ship occupies before rotating */
                uint8_t ship_length = battleship_get_ship_length(ship_types[current_ship]);
                for (uint8_t i = 0; i < ship_length; i++)
                {
                    uint8_t clear_col = ship_orientation ? (cursor_col + i) : cursor_col;
                    uint8_t clear_row = ship_orientation ? cursor_row : (cursor_row + i);

                    /* Only clear if this tile doesn't have a placed ship */
                    if (occupied_board[clear_row][clear_col] == 0)
                    {
                        lcd_msg.command = LCD_CMD_DRAW_TILE;
                        lcd_msg.response_queue = xQueue_LCD_response;
                        lcd_msg.payload.battleship.row = clear_row;
                        lcd_msg.payload.battleship.col = clear_col;
                        lcd_msg.payload.battleship.fill_color = board_tile_fill_color;
                        lcd_msg.payload.battleship.border_color = board_border_color;
                        xQueueSend(xQueue_LCD, &lcd_msg, 0);
                        xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));
                    }
                }

                /* Now rotate */
                ship_orientation = !ship_orientation;
                // printf("Ship orientation: %s\r\n", ship_orientation ? "horizontal" : "vertical");

                /* Redraw ship in new orientation */
                lcd_msg.command = LCD_CMD_DRAW_SHIP;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = cursor_row;
                lcd_msg.payload.battleship.col = cursor_col;
                lcd_msg.payload.battleship.type = ship_types[current_ship];
                lcd_msg.payload.battleship.horizontal = ship_orientation;
                lcd_msg.payload.battleship.border_color = BATTLESHIP_CURSOR_COLOR;
                lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));
            }
        }

        /* SW2 - Place ship on the board */
        if (button_event & ECE353_RTOS_EVENTS_SW2)
        {
            xEventGroupClearBits(ECE353_RTOS_Events, ECE353_RTOS_EVENTS_SW2);

            if (current_ship >= 5)
            {
                printf("ERROR: current_ship out of bounds (%d)\r\n", current_ship);
                return;
            }

            uint8_t ship_length = battleship_get_ship_length(ship_types[current_ship]);
            printf("Attempting to place ship %d (type=%d, length=%d) at (%d,%d)\r\n",
                   current_ship, ship_types[current_ship], ship_length, cursor_col, cursor_row);

            /* Check if ship will fit within board */
            bool fits_in_board = true;
            if (ship_orientation) /* horizontal */
            {
                if ((cursor_col + ship_length) > 10)
                    fits_in_board = false;
            }
            else /* vertical */
            {
                if ((cursor_row + ship_length) > 10)
                    fits_in_board = false;
            }

            /* Try to place the ship - battleship_place_ship handles overlap checking internally */
            bool placement_success = false;
            if (fits_in_board)
            {
                placement_success = battleship_place_ship(cursor_col, cursor_row, ship_types[current_ship], ship_orientation, player_id);
            }

            if (placement_success)
            {
                printf("Ship %d placed at (%d, %d) - %s - ships_placed now %d\r\n",
                       current_ship, cursor_col, cursor_row,
                       ship_orientation ? "horizontal" : "vertical",
                       ships_placed + 1);

                /* Draw the placed ship in green */
                lcd_msg.command = LCD_CMD_DRAW_SHIP;
                lcd_msg.response_queue = xQueue_LCD_response;
                lcd_msg.payload.battleship.row = cursor_row;
                lcd_msg.payload.battleship.col = cursor_col;
                lcd_msg.payload.battleship.type = ship_types[current_ship];
                lcd_msg.payload.battleship.horizontal = ship_orientation;
                lcd_msg.payload.battleship.border_color = BATTLESHIP_CURSOR_COLOR;
                lcd_msg.payload.battleship.fill_color = LCD_COLOR_YELLOW;
                xQueueSend(xQueue_LCD, &lcd_msg, 0);
                xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

                vTaskDelay(pdMS_TO_TICKS(50));

                /* Mark all tiles this ship occupies with ship ID (1-5) */
                uint8_t ship_length = battleship_get_ship_length(ship_types[current_ship]);
                uint8_t ship_id = current_ship + 1; /* Ship IDs are 1-5 */
                for (uint8_t i = 0; i < ship_length; i++)
                {
                    uint8_t mark_col = ship_orientation ? (cursor_col + i) : cursor_col;
                    uint8_t mark_row = ship_orientation ? cursor_row : (cursor_row + i);
                    occupied_board[mark_row][mark_col] = ship_id; /* Update both local and global */
                    printf("Marked occupied_board[%d][%d] = %d\r\n", mark_row, mark_col, ship_id);
                }

                ships_placed++;
                current_ship++; /* Move to next ship */

                printf("Ready for next ship. current_ship=%d, ships_placed=%d\r\n", current_ship, ships_placed);

                /* Reset cursor for next ship */
                cursor_col = 0;
                cursor_row = 0;
                prev_cursor_col = 0;
                prev_cursor_row = 0;
                ship_orientation = true;
                first_draw = true;
            }
            else
            {
                printf("Placement failed for ship %d at (%d,%d). fits_in_board=%d\r\n",
                       current_ship, cursor_col, cursor_row, fits_in_board);
                ipc_send_error(IPC_ERROR_COORD_OCCUPIED);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    printf("All ships placed! Sending PLAYER_READY...\r\n");
    ipc_send_game_control(IPC_GAME_CONTROL_PLAYER_READY);

    /* Wait for opponent to also finish ship placement and send PLAYER_READY */
    printf("Waiting for opponent to finish ship placement...\r\n");
    /* Don't reset opponent_ready - they may have already sent PLAYER_READY */
    while (!opponent_ready)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("Opponent finished ship placement! Both players ready for gameplay.\r\n");
}

/**
 * @brief
 * This function will initialize all of the software resources for the
 * System Control Task
 * @return true
 * @return false
 */
bool task_system_control_resources_init(void)
{
    /* Create the I2C Semaphore */
    Semaphore_I2C = xSemaphoreCreateMutex();
    if (Semaphore_I2C == NULL)
    {
        return false;
    }

    /* Create the SPI Semaphore */
    Semaphore_SPI = xSemaphoreCreateMutex();
    if (Semaphore_SPI == NULL)
    {
        return false;
    }

    /* Create the System Control Task */
    if (xTaskCreate(
            task_system_control,
            "System Control Task",
            configMINIMAL_STACK_SIZE * 5,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL) != pdPASS)
    {
        return false;
    }

    return true;
}

/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 * @param arg
 */
void task_system_control(void *arg)
{
    (void)arg; // Unused parameter
    task_console_printf("Starting System Control Task\r\n");

    /* Configure the IO Expander*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_CONFIG, 0x80); // Set P7 as input, all others as outputs

    /* Initialize opponent ships count - 5 ships at start */
    opponent_ships_remaining = 5;
    update_opponent_ships_leds(opponent_ships_remaining);

    /* Wait for LCD queue to be initialized */
    while (xQueue_LCD_response == NULL || xQueue_LCD == NULL)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    draw_initial_board();

    /* Clear the LCD screen */
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;

    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Initialize game players - wait for SW1, determine player roles */
    initialize_game_players();

    printf("Ready to start ship placement!\r\n");

    /* Draw battleship board for ship placement phase */
    draw_battleship_board();

    /* Execute ship placement task */
    task_ship_placement();

    vTaskDelay(pdMS_TO_TICKS(500));

    /* Run attack phase gameplay */
    task_gameplay();
}

/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    cy_rslt_t rslt;
    console_init();
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    // LCD initialization
    rslt = lcd_initialize();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LCD initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    // LEDS
    rslt = leds_init();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LEDs initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    // buttons
    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Buzzer initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    // joystick
    rslt = joystick_init();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Joystick initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Initialize the spi interface */
    SPI_Obj = spi_init(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
    if (SPI_Obj == NULL)
    {
        printf("SPI initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Initialize the i2c interface */
    I2C_Obj = i2c_init(PIN_I2C_SDA, PIN_I2C_SCL);
    if (I2C_Obj == NULL)
    {
        printf("I2C initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Configure the chip select pins for the EEPROM and IMU*/
    cyhal_gpio_init(PIN_EEPROM_CS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);
    cyhal_gpio_init(PIN_IMU_CS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);
}
/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    /* Create event group for button and system events */
    ECE353_RTOS_Events = xEventGroupCreate();
    if (ECE353_RTOS_Events == NULL)
    {
        printf("Event group creation failed!\n\r");
        CY_ASSERT(0);
    }

    /* Create LCD response queue FIRST */
    xQueue_LCD_response = xQueueCreate(1, sizeof(lcd_cmd_status_t));
    if (xQueue_LCD_response == NULL)
    {
        printf("LCD response queue creation failed!\n\r");
        CY_ASSERT(0);
    }

    if (!task_system_control_resources_init())
    {
        printf("System Control Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_console_init())
    {
        printf("Console initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_lcd_init())
    {
        printf("LCD Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_button_init())
    {
        printf("Button Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_joystick_init())
    {
        printf("Joystick Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_io_expander_resources_init(I2C_Obj, &Semaphore_I2C))
    {
        printf("IO Expander Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_light_sensor_resources_init(I2C_Obj, &Semaphore_I2C))
    {
        printf("Light Sensor Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_eeprom_resources_init(SPI_Obj, &Semaphore_SPI, PIN_EEPROM_CS))
    {
        printf("EEPROM Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_imu_resources_init(SPI_Obj, &Semaphore_SPI, PIN_IMU_CS))
    {
        printf("IMU Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_ipc_init())
    {
        printf("IPC Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}

#endif