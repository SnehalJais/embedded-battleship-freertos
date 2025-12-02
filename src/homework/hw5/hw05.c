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

/* Game state variables */
uint8_t player_id = 255;       /* 0=Player1, 1=Player2, 255=unassigned */
bool opponent_ready = false;   /* Flag set when opponent sends NEW_GAME */
bool ack_received = false;     /* Flag set when opponent sends ACK */
uint8_t next_first_player = 0; /* 0 = I go first next game, 1 = opponent goes first */
bool game_over = false;        /* Flag set when game ends (someone won) */
bool i_won = false;            /* Flag set if I won the game */
EventGroupHandle_t ECE353_RTOS_Events = NULL;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_system_control(void *arg);

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
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

    /* Set the initial state of the LEDs*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_OUTPUT_PORT, 0x01); // Turn on LED0

    /* Wait for LCD queue to be initialized */
    while (xQueue_LCD_response == NULL || xQueue_LCD == NULL)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /* Clear the LCD screen */
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;

    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    // draw the board and other initial messages
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    lcd_console_payload_t *console_payload = &lcd_msg.payload.console;
    console_payload->x_offset = 210;
    console_payload->y_offset = 50;
    console_payload->message = "Hits!";
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload->x_offset = 210;
    console_payload->y_offset = 100;
    console_payload->message = "Misses!";
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(200));

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
        printf("Waiting for opponent ACK...\r\n");
        uint32_t timeout = 5000; /* 5 second timeout in ms */
        uint32_t elapsed = 0;
        while (!ack_received && elapsed < timeout)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            elapsed += 100;
        }

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

    printf("Ready to start ship placement!\r\n");

    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Draw empty board */
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Display hits/misses on right side */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
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

    vTaskDelay(pdMS_TO_TICKS(500)); /* Small delay before placement */

    /* Display current move */
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    console_payload->x_offset = 70;
    console_payload->y_offset = 400;
    if (player_id == 0)
    {
        console_payload->message = "Current Move: Yours";
    }
    else
    {
        console_payload->message = "Current Move: Opponent";
    }
    console_payload->length = strlen(console_payload->message);
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100));

    /* Wait for game end condition */
    game_over = false;
    i_won = false;

    printf("Game in progress... waiting for game end\r\n");

    /* Check for game end every 100ms with 30 second timeout for testing */
    uint32_t game_timeout = 30000; /* 30 seconds for testing */
    uint32_t game_elapsed = 0;
    while (!game_over && game_elapsed < game_timeout)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        game_elapsed += 100;
        /* game_over will be set by IPC RX task when win condition detected */
    }

    /* If timeout reached, test with a win condition */
    if (!game_over)
    {
        printf("Game timeout - simulating win for testing\r\n");
        i_won = true;
        game_over = true;
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
        /* Send END_GAME to opponent since I lost */
        ipc_send_game_control(IPC_GAME_CONTROL_END_GAME);
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

    /* Task completes - game ends */
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100)); /* Just idle */
    }
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