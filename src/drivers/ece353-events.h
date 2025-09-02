/**
 * @file ece353-events.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __ECE353_EVENTS_H__  
 #define  __ECE353_EVENTS_H__

 typedef struct ece353_events_t
{
    unsigned int sw1 : 1;               /* Falling edge of SW1 detected */
    unsigned int tmr_msec_0100 : 1;     /* Timer 100mS has occurred */
} ece353_events_t;

extern volatile ece353_events_t ECE353_Events;

 #endif