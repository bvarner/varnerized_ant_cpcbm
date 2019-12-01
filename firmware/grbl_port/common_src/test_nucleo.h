/*
  test_nucleo.h - test functions for grbl_port_opencm3 on nucleo
  Part of grbl_port_opencm3 project.

  Copyright (c) 2017 Angelo Di Chello

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

#ifndef _TEST_NUCLEO_H_
#define _TEST_NUCLEO_H_

#include <grbl.h>

void test_initialization(void);
void test_sw_debounce(void);
void test_interrupt_signalling(uint32_t num_signals);
void test_led_toggle(void);
void test_heartbeat(void);
void test_fault_heartbeat(void);

#endif /* _TEST_NUCLEO_H_ */
