/**
 * @file hw01-images.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #ifndef __HW01_IMAGES_H__
 #define __HW01_IMAGES_H__

#include <complex.h>
#include <stdint.h>
#include <stdlib.h>

// Bitmap sizes for speaker
#define SPEAKER_WIDTH_PIXELS  46
#define SPEAKER_HEIGHT_PIXELS  35

// Bitmap sizes for alarmclock
#define ALARM_CLOCK_WIDTH_PIXELS  48
#define ALARM_CLOCK_HEIGHT_PIXELS  50

extern const uint8_t Speaker_Bitmaps[];
extern const uint8_t Alarm_Clock_Bitmaps[];

void draw_alarm_clock(uint16_t f_color);
void erase_alarm_clock(void);
void draw_speaker(uint16_t f_color);
void erase_speaker(void);

#endif