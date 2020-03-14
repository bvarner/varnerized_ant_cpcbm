/*
  serial.h - Low level functions for sending and recieving bytes via the serial port
  Part of grbl_port_opencm3 project, derived from the Grbl work.

  Copyright (c) 2017-2020 Angelo Di Chello
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

#ifndef SERIAL_H
#define SERIAL_H


#ifndef RX_BUFFER_SIZE
  #define RX_BUFFER_SIZE 1024
#endif
#ifndef TX_BUFFER_SIZE
  #define TX_BUFFER_SIZE 256
#endif

#define SERIAL_NO_DATA 0xff


void serial_init(void);

// Writes one byte to the TX serial buffer. Called by main program.
void serial_write(uint8_t data);

// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read(void);

// Reset and empty data in read buffer. Used by e-stop and reset.
void serial_reset_read_buffer(void);

// Returns the number of bytes available in the RX serial buffer.
uint32_t serial_get_rx_buffer_available(void);

// Returns the number of bytes used in the RX serial buffer.
// NOTE: Deprecated. Not used unless classic status reports are enabled in config.h.
uint32_t serial_get_rx_buffer_count(void);

// Returns the number of bytes used in the TX serial buffer.
// NOTE: Not used except for debugging and ensuring no TX bottlenecks.
uint32_t serial_get_tx_buffer_count(void);


#ifdef USE_RX_DMA
void serial_rx_dma_init(void);
#endif

#endif /* SERIAL_H */
