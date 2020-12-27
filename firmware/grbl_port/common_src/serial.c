/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of grbl_port_opencm3 project, derived from the Grbl work.

  Copyright (c) 2017-2020 The Ant Team
  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

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

#include "grbl.h"
#ifdef NUCLEO
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>

#endif

#ifdef USE_RX_DMA
uint8_t serial_rx_dma_data;
#endif

#define RX_RING_BUFFER (RX_BUFFER_SIZE+1)
#define TX_RING_BUFFER (TX_BUFFER_SIZE+1)


uint8_t serial_rx_buffer[RX_RING_BUFFER];
uint32_t serial_rx_buffer_head = 0;
volatile uint32_t serial_rx_buffer_tail = 0;

uint8_t serial_tx_buffer[TX_RING_BUFFER];
uint32_t serial_tx_buffer_head = 0;
volatile uint32_t serial_tx_buffer_tail = 0;


// Returns the number of bytes available in the RX serial buffer.
uint32_t serial_get_rx_buffer_available()
{
  uint32_t rtail = serial_rx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head >= rtail) { return(RX_BUFFER_SIZE - (serial_rx_buffer_head-rtail)); }
  return((rtail-serial_rx_buffer_head-1));
}
  

// Returns the number of bytes used in the RX serial buffer.
// NOTE: Deprecated. Not used unless classic status reports are enabled in config.h.
uint32_t serial_get_rx_buffer_count()
{
  uint32_t rtail = serial_rx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head >= rtail) { return(serial_rx_buffer_head-rtail); }
  return (RX_BUFFER_SIZE - (rtail-serial_rx_buffer_head));
}


// Returns the number of bytes used in the TX serial buffer.
// NOTE: Not used except for debugging and ensuring no TX bottlenecks.
uint32_t serial_get_tx_buffer_count()
{
  uint32_t ttail = serial_tx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_tx_buffer_head >= ttail) { return(serial_tx_buffer_head-ttail); }
  return (TX_BUFFER_SIZE - (ttail-serial_tx_buffer_head));
}


void serial_init()
{
#ifdef NUCLEO
	/* Enable GPIOD clock for SERIAL_USART. */
	rcc_periph_clock_enable(SERIAL_USART_RCC_GPIO);

	rcc_periph_reset_pulse(RST_USART2);

	/* Enable clocks for SERIAL_USART. */
	rcc_periph_clock_enable(SERIAL_USART_RCC);
	
	/* Setup GPIO pins for SERIAL_USART transmit and receive. */
	gpio_mode_setup(SERIAL_USART_GPIO_GROUP, GPIO_MODE_AF, GPIO_PUPD_NONE, SERIAL_USART_GPIOS);

	/* Setup SERIAL_USART TX pin as alternate function. */
	gpio_set_af(SERIAL_USART_GPIO_GROUP, SERIAL_USART_GPIO_AF, SERIAL_USART_GPIOS);
	
	usart_disable(SERIAL_USART);
	/* Setup SERIAL_USART parameters. */
	// Set baud rate
	usart_set_baudrate(SERIAL_USART, BAUD_RATE);
	usart_set_databits(SERIAL_USART, 8);
	usart_set_stopbits(SERIAL_USART, USART_STOPBITS_1);
	usart_set_mode(SERIAL_USART, USART_MODE_TX_RX);
	usart_set_parity(SERIAL_USART, USART_PARITY_NONE);
	usart_set_flow_control(SERIAL_USART, USART_FLOWCONTROL_NONE);

#ifdef USE_RX_DMA
	usart_enable_rx_dma(SERIAL_USART);
	serial_rx_dma_init();
#else
	usart_enable_rx_interrupt(SERIAL_USART);
#endif
	/* Finally enable the USART. */
	usart_enable(SERIAL_USART);
	
	nvic_clear_pending_irq(NVIC_DMA1_STREAM5_IRQ);
	nvic_clear_pending_irq(SERIAL_USART_IRQ);
	nvic_enable_irq(SERIAL_USART_IRQ);
	
#else
  // Set baud rate
  #if BAUD_RATE < 57600
    uint16_t UBRR0_value = ((F_CPU / (8L * BAUD_RATE)) - 1)/2 ;
    UCSR0A &= ~(1 << U2X0); // baud doubler off  - Only needed on Uno XXX
  #else
    uint16_t UBRR0_value = ((F_CPU / (4L * BAUD_RATE)) - 1)/2;
    UCSR0A |= (1 << U2X0);  // baud doubler on for high baud rates, i.e. 115200
  #endif
  UBRR0H = UBRR0_value >> 8;
  UBRR0L = UBRR0_value;
            
  // enable rx, tx, and interrupt on complete reception of a byte
  UCSR0B |= (1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0);
	  
  // defaults to 8-bit, no parity, 1 stop bit
#endif
}


// Writes one byte to the TX serial buffer. Called by main program.
// TODO: Check if we can speed this up for writing strings, rather than single bytes.
void serial_write(uint8_t data) {
  // Calculate next head
  uint32_t next_head = serial_tx_buffer_head + 1;
  if (next_head == TX_RING_BUFFER) { next_head = 0; }

  // Wait until there is space in the buffer
  while (next_head == serial_tx_buffer_tail) { 
    // TODO: Restructure st_prep_buffer() calls to be executed here during a long print.    
    if (sys_rt_exec_state & EXEC_RESET) { return; } // Only check for abort to avoid an endless loop.
    test_fault_heartbeat();
  }

  // Store data and advance head
  serial_tx_buffer[serial_tx_buffer_head] = data;
  serial_tx_buffer_head = next_head;
#ifdef NUCLEO
	USART_CR1(SERIAL_USART) |= USART_CR1_TXEIE;
#else
  // Enable Data Register Empty Interrupt to make sure tx-streaming is running
  UCSR0B |=  (1 << UDRIE0); 
#endif
}


// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read()
{
  uint32_t tail = serial_rx_buffer_tail; // Temporary serial_rx_buffer_tail (to optimize for volatile)

  if (serial_rx_buffer_head == tail) {
    return SERIAL_NO_DATA;
  } else {
    uint8_t data = serial_rx_buffer[tail];

    tail++;
    if (tail == RX_RING_BUFFER) { tail = 0; }
    serial_rx_buffer_tail = tail;

    return data;
  }
}

#ifdef NUCLEO
void SERIAL_USART_ISR(void)
{
#ifndef USE_RX_DMA
	if(((USART_SR(SERIAL_USART) & USART_SR_RXNE)  & USART_CR1(SERIAL_USART)) != 0)
	{		
      uint8_t data = (uint8_t)(USART_DR(SERIAL_USART) & USART_DR_MASK);
      uint32_t next_head;
	  
	  // Pick off realtime command characters directly from the serial stream. These characters are
  // not passed into the main buffer, but these set system state flag bits for realtime execution.
	  switch (data) {
		case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
    case CMD_STATUS_REPORT: system_set_exec_state_flag(EXEC_STATUS_REPORT); break; // Set as true
    case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // Set as true
    case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // Set as true
    default :
      if (data > 0x7F) { // Real-time control characters are extended ACSII only.
        switch(data) {
          case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // Set as true
          case CMD_JOG_CANCEL:   
            if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
              system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
            }
            break; 
          #ifdef DEBUG
            case CMD_DEBUG_REPORT: {uint8_t sreg = SREG; cli(); bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); SREG = sreg;} break;
          #endif
          case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
          case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
          case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
          case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
          case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
          case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
          case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
          case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
          case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
          case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
          case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
          case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
          case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
          case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
          case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
          #ifdef ENABLE_M7
            case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
          #endif
        }
        // Throw away any unfound extended-ASCII character by not passing it to the serial buffer.
      } else { // Write character to buffer
		  next_head = serial_rx_buffer_head + 1;
        if (next_head == RX_RING_BUFFER) { next_head = 0; }
		
		  // Write data to buffer unless it is full.
		  if (next_head != serial_rx_buffer_tail) {
			serial_rx_buffer[serial_rx_buffer_head] = data;
			serial_rx_buffer_head = next_head;
		  }
	  }
	}
}
#endif //USE_RX_DMA
	if(((USART_SR(SERIAL_USART) & USART_SR_TXE)  & USART_CR1(SERIAL_USART)) != 0)
	{
		uint32_t tail = serial_tx_buffer_tail; // Temporary serial_tx_buffer_tail (to optimize for volatile)

		// Send a byte from the buffer
		USART_DR(SERIAL_USART) = (serial_tx_buffer[tail] & USART_DR_MASK);
		// Update tail position
		tail++;
		if (tail == TX_RING_BUFFER) { tail = 0; }
		serial_tx_buffer_tail = tail;
		// Turn off Data Register Empty Interrupt to stop tx-streaming if this concludes the transfer
		if (tail == serial_tx_buffer_head) { USART_CR1(SERIAL_USART) &= ~USART_CR1_TXEIE; }
	}
}
#else
// Data Register Empty Interrupt handler
ISR(SERIAL_UDRE)
{
  uint32_t tail = serial_tx_buffer_tail; // Temporary serial_tx_buffer_tail (to optimize for volatile)
  
    // Send a byte from the buffer	
    UDR0 = serial_tx_buffer[tail];
  
    // Update tail position
    tail++;
    if (tail == TX_RING_BUFFER) { tail = 0; }
  
    serial_tx_buffer_tail = tail;
  
  // Turn off Data Register Empty Interrupt to stop tx-streaming if this concludes the transfer
  if (tail == serial_tx_buffer_head) { UCSR0B &= ~(1 << UDRIE0); }
}

ISR(SERIAL_RX)
{
  uint8_t data = UDR0;
  uint32_t next_head;
  
  // Pick off realtime command characters directly from the serial stream. These characters are
  // not passed into the main buffer, but these set system state flag bits for realtime execution.
  switch (data) {
    case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
    case CMD_STATUS_REPORT: system_set_exec_state_flag(EXEC_STATUS_REPORT); break; // Set as true
    case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // Set as true
    case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // Set as true
    default :
      if (data > 0x7F) { // Real-time control characters are extended ACSII only.
        switch(data) {
          case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // Set as true
          case CMD_JOG_CANCEL:   
            if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
              system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
            }
            break; 
          #ifdef DEBUG
            case CMD_DEBUG_REPORT: {uint8_t sreg = SREG; cli(); bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); SREG = sreg;} break;
          #endif
          case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
          case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
          case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
          case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
          case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
          case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
          case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
          case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
          case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
          case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
          case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
          case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
          case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
          case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
          case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
          #ifdef ENABLE_M7
            case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
          #endif
        }
        // Throw away any unfound extended-ASCII character by not passing it to the serial buffer.
      } else { // Write character to buffer
      next_head = serial_rx_buffer_head + 1;
        if (next_head == RX_RING_BUFFER) { next_head = 0; }
    
      // Write data to buffer unless it is full.
      if (next_head != serial_rx_buffer_tail) {
        serial_rx_buffer[serial_rx_buffer_head] = data;
        serial_rx_buffer_head = next_head;    
          } 
      }
  }
}
#endif

void serial_reset_read_buffer() 
{
  serial_rx_buffer_tail = serial_rx_buffer_head;
}

#ifdef NUCLEO
#ifdef USE_RX_DMA
void serial_rx_dma_init(void)
{
	/* Enable clocks for DMA1. */
	rcc_periph_clock_enable(SERIAL_DMA_RCC);

	/* Disable DMA stream and reset it. */
	dma_disable_stream(SERIAL_DMA, SERIAL_DMA_STREAM);
	dma_stream_reset(SERIAL_DMA, SERIAL_DMA_STREAM);

	/* DMA stream settings. */
	dma_set_transfer_mode(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
	dma_set_memory_size(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(SERIAL_DMA, SERIAL_DMA_STREAM);
	dma_disable_peripheral_increment_mode(SERIAL_DMA, SERIAL_DMA_STREAM);
	dma_enable_circular_mode(SERIAL_DMA, SERIAL_DMA_STREAM);
	dma_channel_select(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_CHSEL_4);
	dma_set_memory_burst(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_MBURST_SINGLE);
	dma_set_peripheral_burst(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_SxCR_PBURST_SINGLE);
	dma_set_initial_target(SERIAL_DMA, SERIAL_DMA_STREAM, (uint8_t)0);
	dma_disable_double_buffer_mode(SERIAL_DMA, SERIAL_DMA_STREAM);
    dma_set_dma_flow_control(SERIAL_DMA, SERIAL_DMA_STREAM); //unsure about this
    dma_enable_direct_mode(SERIAL_DMA, SERIAL_DMA_STREAM);
    dma_set_peripheral_address(SERIAL_DMA, SERIAL_DMA_STREAM, (uint32_t)(&(USART_DR(SERIAL_USART))));
	dma_set_memory_address(SERIAL_DMA, SERIAL_DMA_STREAM, (uint32_t)(&serial_rx_dma_data));
	dma_set_number_of_data(SERIAL_DMA, SERIAL_DMA_STREAM, (uint16_t)1);

	/* Clear transfer complete interrupt flag */
	dma_clear_interrupt_flags(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_TCIF);
	/* Enable Transfer Complete interrupt for real-time commands. */
	dma_enable_transfer_complete_interrupt(SERIAL_DMA, SERIAL_DMA_STREAM);

	/* Enable DMA stream. */
	dma_enable_stream(SERIAL_DMA, SERIAL_DMA_STREAM);

	nvic_enable_irq(SERIAL_DMA_IRQ);
}

static inline void dma_clear_interrupt_flags_private(uint32_t dma, uint8_t stream,
	       uint32_t interrupts)
{
  /* Get offset to interrupt flag location in stream field */
  uint32_t flags = (interrupts << DMA_ISR_OFFSET(stream));
  /* First four streams are in low register. Flag clear must be set then
   * reset.
   */
  if (stream < 4) {
    DMA_LIFCR(dma) = flags;
  } else {
    DMA_HIFCR(dma) = flags;
  }
}

void SERIAL_DMA_ISR()
{
  uint32_t next_head;
  /* Clear transfer complete interrupt flag */
  dma_clear_interrupt_flags_private(SERIAL_DMA, SERIAL_DMA_STREAM, DMA_TCIF);

    switch (serial_rx_dma_data)
    {
		case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
		case CMD_STATUS_REPORT: system_set_exec_state_flag(EXEC_STATUS_REPORT); break; // Set as true
		case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // Set as true
		case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // Set as true
		default :
		  if (serial_rx_dma_data > 0x7F) { // Real-time control characters are extended ACSII only.
			switch(serial_rx_dma_data) {
			  case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // Set as true
			  case CMD_JOG_CANCEL:
				if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
				  system_set_exec_state_flag(EXEC_MOTION_CANCEL);
				}
			break;
			  #ifdef DEBUG
				case CMD_DEBUG_REPORT: {uint8_t sreg = SREG; cli(); bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); SREG = sreg;} break;
			  #endif
			  case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
			  case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
			  case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
			  case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
			  case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
			  case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
			  case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
			  case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
			  case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
			  case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
			  case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
			  case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
			  case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
			  case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
			  case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
			  #ifdef ENABLE_M7
				case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
			  #endif
			}
			// Throw away any unfound extended-ASCII character by not passing it to the serial buffer.
		  } else { // Write character to buffer
			next_head = serial_rx_buffer_head + 1;
			if (next_head == RX_RING_BUFFER) { next_head = 0; }

			// Write data to buffer unless it is full.
			if (next_head != serial_rx_buffer_tail) {
			  serial_rx_buffer[serial_rx_buffer_head] = serial_rx_dma_data;
			  serial_rx_buffer_head = next_head;
			}
			}
    }
}
#endif
#endif
