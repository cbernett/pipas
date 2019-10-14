#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"

#define TIMER            NRF_TIMER1
#define TIMER_IRQn       TIMER1_IRQn
#define TIMER_IRQHandler TIMER1_IRQHandler

static uint32_t timer_seconds;

static void timer_start(void)
{
    // Reset the second variable
    timer_seconds = 0;
    
    // Ensure the timer uses 24-bit bitmode or higher
    TIMER->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
    
    // Set the prescaler to 4, for a timer interval of 1 us (16M / 2^4)
    TIMER->PRESCALER = 4;
    
    // Set the CC[0] register to hit after 1 second
    TIMER->CC[0] = 1000000;
    
    // Make sure the timer clears after reaching CC[0]
    TIMER->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    
    // Trigger the interrupt when reaching CC[0]
    TIMER->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    
    // Set a low IRQ priority and enable interrupts for the timer module
    NVIC_SetPriority(TIMER_IRQn, 3);
    NVIC_EnableIRQ(TIMER_IRQn);
    
    // Clear and start the timer
    TIMER->TASKS_CLEAR = 1;
    TIMER->TASKS_START = 1;
}

static uint32_t millis(void)
{
    // Store the current value of the timer in the CC[1] register, by triggering the capture task
    TIMER->TASKS_CAPTURE[1] = 1;
    
    // Combine the state of the second variable with the current timer state, and return the result
    return (timer_seconds * 16.768) + (TIMER->CC[1] / 1000);
}

static uint64_t micros(void)
{
    // Store the current value of the timer in the CC[1] register, by triggering the capture task
    TIMER->TASKS_CAPTURE[1] = 1;
    
    // Combine the state of the second variable with the current timer state, and return the result
    return (uint64_t)timer_seconds * 1000000 + TIMER->CC[1];
}

// Timer interrupt handler
void TIMER_IRQHandler(void)
{
    if(TIMER->EVENTS_COMPARE[0])
    {
        TIMER->EVENTS_COMPARE[0] = 0;

        // Increment the second variable
        timer_seconds++;
    }
}