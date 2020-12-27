/*
  test_nucleo.c - test functions for grbl_port_opencm3 on nucleo
  Part of grbl_port_opencm3 project.

  Copyright (c) 2017-2020 The Ant Team

  Grbl_port_opencm3 is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl_port_opencm3 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl_port_opencm3.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "test_nucleo.h"

//#ifdef TEST_NUCLEO_SW_DEBOUNCE_TIMER
#include "limits.h"
//#endif

void test_heartbeat_initialization()
{
  /* Enable GPIOA clock. */
  rcc_periph_clock_enable(RCC_GPIOA);

  /* Set GPIO5 (in GPIO port A) to 'output push-pull'. */
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);

  SET_HEARTBEAT_DDR;
}

static void test_user_button_initialization(void)
{
  /* Enable GPIOC clock. */
  rcc_periph_clock_enable(RCC_GPIOC);

  /* Set GPIO13 (in GPIO port C) as input and set the pull-up. */
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO13);

  /*reset pending exti events */
  exti_reset_request(EXTI13);
  /*reset pending exti interrupts */
  nvic_clear_pending_irq(NVIC_EXTI15_10_IRQ);
  exti_select_source(EXTI13, GPIOC);
  exti_enable_request(EXTI13);
  exti_set_trigger(EXTI13, EXTI_TRIGGER_FALLING);
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);// Enable Limits pins Interrupt
}

void test_sw_debounce(void)
{
  test_user_button_initialization();
}

void exti15_10_isr(void)
{
  exti_reset_request(EXTI13);
  nvic_clear_pending_irq(NVIC_EXTI15_10_IRQ);

  //test_interrupt_signalling(1);
  enable_debounce_timer();
}


void test_interrupt_signalling(uint32_t num_signals)
{
    uint32_t j;
    uint32_t i;
    for(j = 0; j < num_signals; j++)
    {
    	gpio_set(GPIOA, GPIO5);	/* LED on/off */
    	for (i = 0; i < 100000; i++)
    	{	/* Wait a bit. */
    		__asm__("nop");
    	}
    	gpio_clear(GPIOA, GPIO5);	/* LED on/off */
    	for (i = 0; i < 100000; i++)
    	{	/* Wait a bit. */
    		__asm__("nop");
    	}
    }
}

void test_led_toggle(void)
{

	if (gpio_get(GPIOA, GPIO5) != 0)
		gpio_clear(GPIOA, GPIO5);	/* LED off */
	else
		gpio_set(GPIOA, GPIO5);	/* LED on */
}


#define HEARTBEAT_MAX_DELAY        500000
#define HEARTBEAT_FAULT_MAX_DELAY  200000
// Initialize Heart Beat variable to normal slow delay, so the led should blink slowly
uint32_t heartbeat_delay = HEARTBEAT_MAX_DELAY;

void test_heartbeat()
{
	if(heartbeat_delay == 0)
	{
        TOGGLE_HEARTBEAT_BIT;
        heartbeat_delay = HEARTBEAT_MAX_DELAY;
    }
    else
    {
        heartbeat_delay--;
    }
}

void test_fault_heartbeat()
{
	if(heartbeat_delay == 0)
	{
        TOGGLE_HEARTBEAT_BIT;
        heartbeat_delay = HEARTBEAT_FAULT_MAX_DELAY;
    }
    else
    {
        heartbeat_delay--;
    }
}

void nmi_handler(void)
{
    while(true)
    {
        test_fault_heartbeat();
    }
}

void hard_fault_handler(void)
{
    while(true)
    {
        test_fault_heartbeat();
    }
}

