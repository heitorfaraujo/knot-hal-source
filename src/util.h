/*
 * Copyright (c) 2015, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#endif

#include <stdlib.h>

#ifndef __UTIL_H__
#define __UTIL_H__

#ifdef __cplusplus
extern "C"{
#endif

#ifndef ARDUINO
/**
 * \brief Gets current time in time base.
 * \param   time  Reference variable to the time.
 * \param   time_base  Base time converter value (default in microseconds).
 * \return  0 if success; otherwise -1.
 */
int tline(unsigned long *ptime, unsigned long time_base /*=1*/);
/**
 * \brief Gets current time in MILLISECONDS.
 * \return  timeline value.
 */
unsigned long tline_ms(void);

#else
/**
 * \brief Gets current time in MILLISECONDS.
 * \return  timeline value.
 */
static inline unsigned long tline_ms(void) { return millis(); }
/**
 * \brief Gets current time in MICROSECONDS.
 * \return  timeline value.
 */
static inline unsigned long tline_us(void) { return micros(); }

#endif

/**
 * \brief Check if timeline has expired.
 * \param   time  Current time reference.
 * \param   last  Last time reference.
 * \param   timeout Elapsed time.
 * \return  true for timeout expired; otherwise, false.
 */
int tline_timeout(unsigned long time,  unsigned long last,  unsigned long timeout);

/**
 * \brief Get a random number value.
 * \param   interval  Range to random value.
 * \param   ntime  Number of times the interval.
 * \param   min Minimum value.
 * \return  random number value.
 */
int get_random_value(int interval, int ntime, int min);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __UTIL_H__ */
