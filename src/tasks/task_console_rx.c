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

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "task_console.h"
#include "devices.h"
#include "task_eeprom.h"
#include "task_imu.h"
#include "cyhal_uart.h"
/**
 * @brief
 * This file contains the implementation of the console receive (Rx) task.
 * The task is responsible for processing incoming console commands and
 * controlling the state of the LEDs accordingly.
 *
 * The task uses a double buffer to process the incoming console commands.
 * The supported commands will be "RED_ON" and "RED_OFF" to control the red LED.
 */

/* ADD CODE */
/* Global Variables */
console_buffer_t console_buffer1;
console_buffer_t console_buffer2;

QueueHandle_t xQueue_Console_Rx;

// External reference to the system control response queue
QueueHandle_t Queue_System_Control_Responses;

// Pointers to the produce and consume buffers
console_buffer_t *produce_console_buffer;
console_buffer_t *consume_console_buffer;

// Allocate task handle for the console Rx task
TaskHandle_t TaskHandle_Console_Rx;

/**
 * @brief
 * This function is the bottom half task for receiving console input.
 *
 * It waits for a task notification from the ISR indicating that a new
 * command has been received. The task then processes the command and
 * controls the state of the LEDs accordingly.
 *
 * @param param Unused parameter
 */
void task_console_rx(void *param)
{
    (void)param; // Unused parameter
    // printf("DEBUG: Console RX task started and waiting for notifications\n\r");

    while (1)
    {
        /* ADD CODE */
        // wait indefinitely for a task notification
        //  printf("DEBUG: Console RX task waiting for notification...\n\r");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Add newline to separate command input from response
        printf("\r\n");

        // processthe data pointed to by the console buffer pointer
        // if "RED_ON" turn on red LED
        if (strcmp(consume_console_buffer->data, "RED_ON") == 0)
        {
            printf("Turning on RED LED\r\n");
            leds_set_state(LED_RED, LED_ON);
        }
        // if "RED_OFF" turn off the LED
        else if (strcmp(consume_console_buffer->data, "RED_OFF") == 0)
        {
            printf("Turning off RED LED\r\n");
            leds_set_state(LED_RED, LED_OFF);
        }
        // Handle EEPROM and IMU commands
        else
        {
            // Parse the command using strtok or sscanf
            char *token = strtok(consume_console_buffer->data, " ");

            if (token != NULL && (strcmp(token, "EEPROM") == 0 || strcmp(token, "eeprom") == 0))
            {
                // Parse EEPROM commands: "EEPROM w <address> <value>" or "EEPROM r <address>"
                char *operation = strtok(NULL, " ");
                if (operation != NULL && strcmp(operation, "w") == 0)
                {
                    // Write command: EEPROM w <address> <value>
                    char *addr_str = strtok(NULL, " ");
                    char *value_str = strtok(NULL, " ");

                    if (addr_str != NULL && value_str != NULL)
                    {
                        uint16_t address = (uint16_t)strtol(addr_str, NULL, 0);
                        uint8_t value = (uint8_t)strtol(value_str, NULL, 0);

                        // Call EEPROM write function
                        if (system_sensors_eeprom_write(Queue_System_Control_Responses, address, value))
                        {
                            task_console_printf("EEPROM Write: Addr=0x%04X, Value=0x%02X - Success\r\n", address, value);
                        }
                        else
                        {
                            task_console_printf("EEPROM Write Failed: Addr=0x%04X, Value=0x%02X\r\n", address, value);
                        }
                    }
                    else
                    {
                        task_console_printf("Usage: EEPROM w <address> <value>\r\n");
                    }
                }
                else if (operation != NULL && strcmp(operation, "r") == 0)
                {
                    // Read command: EEPROM r <address>
                    char *addr_str = strtok(NULL, " ");

                    if (addr_str != NULL)
                    {
                        uint16_t address = (uint16_t)strtol(addr_str, NULL, 0);
                        uint8_t data = 0;

                        // Call EEPROM read function
                        if (system_sensors_eeprom_read(Queue_System_Control_Responses, address, &data))
                        {
                            task_console_printf("EEPROM Read: Addr=0x%04X Value=0x%02X\r\n", address, data);
                        }
                        else
                        {
                            task_console_printf("EEPROM Read Failed: Addr=0x%04X\r\n", address);
                        }
                    }
                    else
                    {
                        task_console_printf("Usage: EEPROM r <address>\r\n");
                    }
                }
                else
                {
                    task_console_printf("Usage: EEPROM [w|r] <address> [value]\r\n");
                }
            }
            else if (token != NULL && (strcmp(token, "IMU") == 0 || strcmp(token, "imu") == 0))
            {
                // Parse IMU commands: "IMU r" or just "IMU" (read accelerometer data)
                char *operation = strtok(NULL, " ");
                if (operation == NULL || strcmp(operation, "r") == 0)
                {
                    // Read command: IMU r or just IMU
                    uint16_t imu_data[3]; // X, Y, Z accelerometer data

                    // Call IMU read function
                    if (system_sensors_imu_read(Queue_System_Control_Responses, imu_data))
                    {
                        // Convert uint16_t to signed int16_t for proper negative value display
                        int16_t x = (int16_t)imu_data[0];
                        int16_t y = (int16_t)imu_data[1];
                        int16_t z = (int16_t)imu_data[2];
                        task_console_printf("IMU Data: X=%d, Y=%d, Z=%d\r\n", x, y, z);
                    }
                    else
                    {
                        task_console_printf("IMU Read Failed\r\n");
                    }
                }
                else
                {
                    task_console_printf("Usage: IMU [r]\r\n");
                }
            }
            else
            {
                task_console_printf("Unknown command: %s\r\n", consume_console_buffer->data);
            }
        }
    }
}

/**
 * @brief
 * This function initializes the resources for the console Rx task.
 * @return true if resources were initialized successfully
 * @return false if resource initialization failed
 */
bool task_console_resources_init_rx(void)
{
    BaseType_t rslt;

    /* ADD CODE */
    // Create the System Control Response Queue for gatekeeper communication
    Queue_System_Control_Responses = xQueueCreate(10, sizeof(device_response_msg_t));
    if (Queue_System_Control_Responses == NULL)
    {
        return false;
    }

    // allocate an array from heap for the two console buffers
    console_buffer1.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);
    console_buffer2.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);

    produce_console_buffer = &console_buffer1;
    consume_console_buffer = &console_buffer2;

    // set initial lengths to zero
    produce_console_buffer->index = 0;
    consume_console_buffer->index = 0;

    // Check if memory allocation was successful
    if (console_buffer1.data == NULL || console_buffer2.data == NULL)
    {
        return false; // Memory allocation failed
    }

    // Create the console Rx task
    rslt = xTaskCreate(
        task_console_rx,
        "Console Rx",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        &TaskHandle_Console_Rx);

    if (rslt != pdPASS)
    {
        printf("ERROR: Console RX task creation failed!\n\r");
        return false; // Task creation failed
    }

    // printf("DEBUG: Console RX task created successfully\n\r");
    return true; // Resources initialized successfully
}
#endif