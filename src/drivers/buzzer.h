/**
 * @file buzzer.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdio.h>
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "ece353-pins.h"

cy_rslt_t buzzer_init(float duty, uint32_t frequency);
void buzzer_on(void);
void buzzer_off(void);

#endif