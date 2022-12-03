#ifndef TIMER_H
#define TIMER_H

// TC3 Constants
const int TC3_PRESCALER = 64;
const int TC3_CLOCK_FREQUENCY = 32768;
const int TC3_PRESCALED_CLOCK_FREQUENCY = TC3_CLOCK_FREQUENCY / TC3_PRESCALER;
const int NUM_MS_IN_SEC = 1000;

/**
 * Configure and enable TC3 timer.
 **/
void timer3_init();

/**
 * Reset the TC3 timer counters only. 
 **/
void timer3_reset();

/**
 * Set the TC3 timer counter period. An interrupt will fire every `period_ms` milliseconds.
 * 
 * PARAMS:
 * uint16_t period_ms -> The timer counter period in which an interrupt should fire, in milliseconds.
 **/
void timer3_set(uint16_t period_ms);

#endif
