/*
  cpu_map_nucleof401.h - CPU and pin mapping configuration file for NUCLEO F401 board.
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

/* This cpu_map file serves as a central pin mapping settings file for ST Nucleo F401 */

#ifndef CPU_MAP_NUCLEOF401_H
#define CPU_MAP_NUCLEOF401_H

#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/timer.h>

//#define TEST_NUCLEO_EXTI_PINS

//#define BASIC_CPU_SPEED
#ifdef BASIC_CPU_SPEED
#define F_CPU (16000000)
#define PSC_MUL_FACTOR 1
#else
#define F_CPU (48000000)
#define PSC_MUL_FACTOR 3
#endif

#ifdef GRBL_PLATFORM
#error "cpu_map already defined: GRBL_PLATFORM=" GRBL_PLATFORM
#endif

#define GRBL_PLATFORM "NucleoF401"

#define NUCLEO // Nucleo
#define USE_RX_DMA

#ifdef USE_RX_DMA
#define SERIAL_DMA              DMA1
#define SERIAL_DMA_RCC          RCC_DMA1
#define SERIAL_DMA_STREAM       DMA_STREAM5
#define SERIAL_DMA_ISR          dma1_stream5_isr
#define SERIAL_DMA_IRQ          NVIC_DMA1_STREAM5_IRQ
#endif

#define SERIAL_USART            USART2
#define SERIAL_USART_BASE       USART2_BASE
#define SERIAL_USART_RCC        RCC_USART2
#define SERIAL_USART_RCC_GPIO   RCC_GPIOA
#define SERIAL_USART_GPIO_GROUP GPIOA
#define SERIAL_USART_GPIO_AF    GPIO_AF7
#define SERIAL_USART_GPIOS      (GPIO2 | GPIO3)
#define SERIAL_USART_ISR        usart2_isr
#define SERIAL_USART_IRQ        NVIC_USART2_IRQ

#define STEPPER_GPIOS_RCC      (RCC_GPIOA | RCC_GPIOB)

#define STEPPER_MAIN_TIMER     TIM4
#define MAIN_TIMER_ISR         tim4_isr
#define MAIN_TIMER_RCC         RCC_TIM4
#define MAIN_TIMER_IRQ         NVIC_TIM4_IRQ
#define MAIN_TIMER_RST         RST_TIM4

#define STEPPER_PULSE_TIMER    TIM2
#define PULSE_TIMER_ISR        tim2_isr
#define PULSE_TIMER_IRQ        NVIC_TIM2_IRQ
#define PULSE_TIMER_RCC        RCC_TIM2
#define PULSE_TIMER_RST         RST_TIM2

#define SW_DEBOUNCE_TIMER      TIM5
#define SW_DEBOUNCE_TIMER_RCC  RCC_TIM5
#define SW_DEBOUNCE_TIMER_IRQ  NVIC_TIM5_IRQ
#define SW_DEBOUNCE_TIMER_ISR  tim5_isr
#define SW_DEBOUNCE_TIMER_RST  RST_TIM5

#define SPINDLE_TIMER          TIM3
#define SPINDLE_TIMER_RCC      RCC_TIM3
#define SPINDLE_TIMER_RST      RST_TIM3
#define SPINDLE_TIMER_CHAN     TIM_OC1
#define SPINDLE_TIMER_PWM_TYPE TIM_OCM_PWM1
#define SPINDLE_TIMER_OFF_TYPE TIM_OCM_FORCE_LOW
#define SPINDLE_GPIO_GROUP     GPIOA
#define SPINDLE_GPIO_AF        GPIO_AF2
#define SPINDLE_GPIO           GPIO6
#define SPINDLE_TIMER_BUS_FREQ rcc_apb1_frequency

#define COOLANT_RCC            RCC_GPIOC

#define MAIN_SECTOR 2
#define COPY_SECTOR 3
#define EFLASH_MAIN_BASE_ADDRESS          0x08008000U
#define EFLASH_COPY_BASE_ADDRESS          0x0800C000U

// Serial port pins
// #define SERIAL_RX USART0_RX_vect
// #define SERIAL_UDRE USART0_UDRE_vect

// Increase Buffers to make use of extra SRAM
//#define RX_BUFFER_SIZE		256
//#define TX_BUFFER_SIZE		128
//#define BLOCK_BUFFER_SIZE	36
//#define LINE_BUFFER_SIZE	100

// Define step pulse output pins. NOTE: All step bit pins must be on the same port.
#define STEP_X_DDR        GPIOA_MODER
#define STEP_X_PORT       GPIOA_ODR
#define X_STEP_BIT        10 // NucleoF401 Digital PA10
#define STEP_MASK_X_DDR   (1<<(X_STEP_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define STEP_X_DDR_RESET_MASK   (0x3<<(X_STEP_BIT*2))
#define STEP_MASK_X       (1<<X_STEP_BIT)     // X step mask bit

#define STEP_Y_DDR            GPIOB_MODER
#define STEP_Y_PORT           GPIOB_ODR
#define Y_STEP_BIT        3 // NucleoF401 Digital PB3
#define STEP_MASK_Y_DDR       (1<<(Y_STEP_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define STEP_Y_DDR_RESET_MASK (0x3<<(Y_STEP_BIT*2))
#define STEP_MASK_Y           (1<<Y_STEP_BIT)

#define STEP_Z_DDR            GPIOB_MODER
#define STEP_Z_PORT           GPIOB_ODR
#define Z_STEP_BIT        5 // NucleoF401 Digital PB5
#define STEP_MASK_Z_DDR       (1<<(Z_STEP_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define STEP_Z_DDR_RESET_MASK (0x3<<(Z_STEP_BIT*2))
#define STEP_MASK_Z           (1<<Z_STEP_BIT)

#define STEP_MASK (STEP_MASK_X | STEP_MASK_Y | STEP_MASK_Z)

// Define step direction output pins. NOTE: All direction pins must be on the same port.
#define DIRECTION_Z_DDR       GPIOA_MODER
#define DIRECTION_Z_PORT      GPIOA_ODR
#define Z_DIRECTION_BIT       8   // NucleoF401 Digital PA8
#define DIRECTION_MASK_Z_DDR  (1<<(Z_DIRECTION_BIT*2)) // Z DIR Mask direction bits
#define DIRECTION_Z_DDR_RESET_MASK  (0x3<<(Z_DIRECTION_BIT*2)) // Z DIR Mask direction bits
#define DIRECTION_MASK_Z      (1<<Z_DIRECTION_BIT) // Z DIR MASK bit

#define DIRECTION_X_DDR            GPIOB_MODER
#define DIRECTION_X_PORT           GPIOB_ODR
#define X_DIRECTION_BIT        4  // NucleoF401 Digital PB4
#define DIRECTION_MASK_X_DDR       (1<<(X_DIRECTION_BIT*2)) // All direction bits
#define DIRECTION_X_DDR_RESET_MASK (0x3<<(X_DIRECTION_BIT*2)) // All direction bits
#define DIRECTION_MASK_X           (1<<X_DIRECTION_BIT)         // XY DIR MASK bits

#define DIRECTION_Y_DDR            GPIOB_MODER
#define DIRECTION_Y_PORT           GPIOB_ODR
#define Y_DIRECTION_BIT        10 // NucleoF401 Digital PB10
#define DIRECTION_MASK_Y_DDR       (1<<(Y_DIRECTION_BIT*2)) // All direction bits
#define DIRECTION_Y_DDR_RESET_MASK (0x3<<(Y_DIRECTION_BIT*2)) // All direction bits
#define DIRECTION_MASK_Y           (1<<Y_DIRECTION_BIT)         // XY DIR MASK bits

// Define stepper driver enable/disable output pin.
#define STEPPERS_DISABLE_DDR        GPIOA_MODER
#define STEPPERS_DISABLE_PORT       GPIOA_ODR
#define STEPPERS_DISABLE_BIT        9 // NucleoF401 Digital PA9
#define STEPPERS_DISABLE_MASK_DDR   (1<<(STEPPERS_DISABLE_BIT*2))
#define STEPPERS_DISABLE_MASK       (1<<STEPPERS_DISABLE_BIT)

// Define homing/hard limit switch input pins and limit interrupt vectors. 
// NOTE: All limit bit pins must be on the same port
#define LIMIT_X_GPIO             GPIOC
#define LIMIT_X_DDR              GPIOC_MODER
#define LIMIT_X_PORT             GPIOC_ODR
#define LIMIT_X_PIN              GPIOC_IDR
#define LIMIT_X_PU               GPIOC_PUPDR
#define X_LIMIT_BIT               7 // NucleoF401 Digital PC7
#define LIMIT_X_PU_MASK          (0x1<<(X_LIMIT_BIT*2)) // X limit pull-up mask
#define LIMIT_X_PU_RESET_MASK    (0x3<<(X_LIMIT_BIT*2)) // X limit dir mask
#define LIMIT_X_MASK             (1<<X_LIMIT_BIT) // X limit bits
#define LIMIT_X_EXTI             EXTI7
#define LIMIT_X_ISR              exti9_5_isr
#define LIMIT_X_INT              NVIC_EXTI9_5_IRQ

#define LIMIT_Y_GPIO              GPIOB
#define LIMIT_Y_DDR               GPIOB_MODER
#define LIMIT_Y_PORT              GPIOB_ODR
#define LIMIT_Y_PIN               GPIOB_IDR
#define LIMIT_Y_PU                GPIOB_PUPDR
#define Y_LIMIT_BIT               6 // NucleoF401 Digital PB6
#define LIMIT_Y_PU_MASK           (0x1<<(Y_LIMIT_BIT*2)) // Y limit pull-up mask
#define LIMIT_Y_PU_RESET_MASK     ((0x3<<(Y_LIMIT_BIT*2))) // Y limit dir mask
#define LIMIT_Y_MASK              (1<<Y_LIMIT_BIT) // Y limit bits
#define LIMIT_Y_EXTI              EXTI6
#define LIMIT_Y_ISR              exti9_5_isr
#define LIMIT_Y_INT              NVIC_EXTI9_5_IRQ

#define LIMIT_Z_GPIO              GPIOB
#define LIMIT_Z_DDR               GPIOB_MODER
#define LIMIT_Z_PORT              GPIOB_ODR
#define LIMIT_Z_PIN               GPIOB_IDR
#define LIMIT_Z_PU                GPIOB_PUPDR
#define Z_LIMIT_BIT               0 // NucleoF401 Digital PB0
#define LIMIT_Z_PU_MASK           (0x1<<(Z_LIMIT_BIT*2)) // Z limit pull-up mask
#define LIMIT_Z_PU_RESET_MASK     ((0x3<<(Z_LIMIT_BIT*2))) // Z limit dir mask
#define LIMIT_Z_MASK              (1<<Z_LIMIT_BIT) // Y limit bits
#define LIMIT_Z_EXTI              EXTI0
#define LIMIT_Z_ISR              exti0_isr
#define LIMIT_Z_INT              NVIC_EXTI0_IRQ

#define LIMIT_MASK                (LIMIT_X_MASK | LIMIT_Y_MASK | LIMIT_Z_MASK)
//#define INVERT_LIMIT_PIN_MASK     (LIMIT_MASK)
#define LIMIT_X_EXTI_CLEAR       (LIMIT_X_EXTI | LIMIT_Y_EXTI)
#define LIMIT_Y_EXTI_CLEAR       (LIMIT_X_EXTI | LIMIT_Y_EXTI)
#define LIMIT_Z_EXTI_CLEAR       (LIMIT_Z_EXTI)

/* Interrupt defines for LIMIT PINS */
#define LIMIT_INT_vect            (LIMIT_X_EXTI | LIMIT_Y_EXTI | LIMIT_Z_EXTI)

// Define user-control CONTROLs (cycle start, reset, feed hold) input pins.
// NOTE: All CONTROLs pins must be on the same port and not on a port with other input pins (limits).
#define RESET_CONTROL_GPIO              GPIOB
#define RESET_CONTROL_DDR               GPIOB_MODER
#define RESET_CONTROL_PORT              GPIOB_ODR
#define RESET_CONTROL_PIN               GPIOB_IDR
#define RESET_CONTROL_PU                GPIOB_PUPDR
#define RESET_BIT                       2 // NucleoF401 Digital PB2
#define RESET_CONTROL_PU_MASK           (0x1<<(RESET_BIT*2)) // Reset pull-up mask
#define RESET_CONTROL_PU_RESET_MASK     (0x3<<(RESET_BIT*2)) // Reset dir mask
#define RESET_CONTROL_MASK              (1<<RESET_BIT)
/* Interrupt defines for RESET CONTROL PIN */
#define RESET_CONTROL_INT               NVIC_EXTI2_IRQ  // Pin change interrupt enable pin
#define RESET_CONTROL_INT_vect          (EXTI2) 
#define RESET_CONTROL_ISR               exti2_isr        // Pin change interrupt register

#define FEED_HOLD_CONTROL_GPIO          GPIOA
#define FEED_HOLD_CONTROL_DDR           GPIOA_MODER
#define FEED_HOLD_CONTROL_PORT          GPIOA_ODR
#define FEED_HOLD_CONTROL_PIN           GPIOA_IDR
#define FEED_HOLD_CONTROL_PU            GPIOA_PUPDR
#define FEED_HOLD_BIT                   1 // NucleoF401 Digital PA1
#define FEED_HOLD_PU_MASK               (0x1<<(FEED_HOLD_BIT*2)) // Feed Hold pull-up mask
#define FEED_HOLD_PU_RESET_MASK         (0x3<<(FEED_HOLD_BIT*2)) // Feed Hold pull-up reset mask
#define FEED_HOLD_MASK                  (1<<FEED_HOLD_BIT)
/* Interrupt defines for FEED-HOLD CONTROL PIN */
#define FEED_HOLD_CONTROL_INT           NVIC_EXTI1_IRQ  // Pin change interrupt enable pin
#define FEED_HOLD_CONTROL_INT_vect      (EXTI1) 
#define FEED_HOLD_CONTROL_ISR           exti1_isr           // Pin change interrupt register

#define CYCLE_START_CONTROL_GPIO          GPIOA
#define CYCLE_START_CONTROL_DDR           GPIOA_MODER
#define CYCLE_START_CONTROL_PORT          GPIOA_ODR
#define CYCLE_START_CONTROL_PIN           GPIOA_IDR
#define CYCLE_START_CONTROL_PU            GPIOA_PUPDR
#define CYCLE_START_BIT                   4 // NucleoF401 Digital PA4
#define CYCLE_START_PU_MASK               (0x1<<(CYCLE_START_BIT*2)) // CYCLE_START pull-up mask
#define CYCLE_START_PU_RESET_MASK         (0x3<<(CYCLE_START_BIT*2)) // CYCLE_START dir mask
#define CYCLE_START_MASK                  (1<<CYCLE_START_BIT)
/* Interrupt defines for CYCLE START CONTROL PIN */
#define CYCLE_START_CONTROL_INT           NVIC_EXTI4_IRQ  // Pin change interrupt enable pin
#define CYCLE_START_CONTROL_INT_vect      (EXTI4) 
#define CYCLE_START_CONTROL_ISR           exti4_isr // Pin change interrupt register

#define SAFETY_DOOR_CONTROL_GPIO          GPIOC
#define SAFETY_DOOR_CONTROL_DDR           GPIOC_MODER
#define SAFETY_DOOR_CONTROL_PORT          GPIOC_ODR
#define SAFETY_DOOR_CONTROL_PIN           GPIOC_IDR
#define SAFETY_DOOR_CONTROL_PU            GPIOC_PUPDR
#define SAFETY_DOOR_BIT                   3 // NucleoF401 Digital PC3
#define SAFETY_DOOR_PU_MASK               (0x1<<(SAFETY_DOOR_BIT*2)) // SAFETY_DOOR pull-up mask
#define SAFETY_DOOR_PU_RESET_MASK         (0x3<<(SAFETY_DOOR_BIT*2)) // SAFETY_DOOR pull-up reset mask
#define SAFETY_DOOR_MASK                  (1<<SAFETY_DOOR_BIT)
/* Interrupt defines for SAFETY DOOR CONTROL PIN */
#define SAFETY_DOOR_CONTROL_INT           NVIC_EXTI3_IRQ  // Pin change interrupt enable pin
#define SAFETY_DOOR_CONTROL_INT_vect      (EXTI3) 
#define SAFETY_DOOR_CONTROL_ISR           exti3_isr // Pin change interrupt register

#define CONTROL_INT_vect  (RESET_CONTROL_INT_vect | FEED_HOLD_CONTROL_INT_vect | CYCLE_START_CONTROL_INT_vect | SAFETY_DOOR_CONTROL_INT_vect)

// Re-Define user-control controls (cycle start, reset, feed hold) input pins.
#define CONTROL_RESET_BIT         RESET_BIT
#define CONTROL_FEED_HOLD_BIT     FEED_HOLD_BIT
#define CONTROL_CYCLE_START_BIT   CYCLE_START_BIT
#define CONTROL_SAFETY_DOOR_BIT   SAFETY_DOOR_BIT
#define CONTROL_MASK ((1<<RESET_BIT)|(1<<FEED_HOLD_BIT)|(1<<CYCLE_START_BIT)|(1<<SAFETY_DOOR_BIT))
#define CONTROL_INVERT_MASK CONTROL_MASK // May be re-defined to only invert certain control pins.


// Define probe switch input pin.
#define PROBE_DDR       GPIOC_MODER
#define PROBE_PIN       GPIOC_IDR
#define PROBE_PORT      GPIOC_ODR
#define PROBE_PU        GPIOC_PUPDR
#define PROBE_BIT       0 // NucleoF401 Digital PC0
#define PROBE_PU_MASK       (0x1<<(PROBE_BIT*2))
#define PROBE_PU_RESET_MASK (0x3<<(PROBE_BIT*2))
#define PROBE_MASK      (1<<PROBE_BIT)
#define PROBE_RCC           RCC_GPIOC


// Define spindle enable and spindle direction output pins.
#define SPINDLE_ENABLE_DDR               GPIOA_MODER
#define SPINDLE_ENABLE_PORT              GPIOA_ODR
#define SPINDLE_ENABLE_BIT               6                           // NucleoF401 Digital Pin PA6
#define SPINDLE_ENABLE_MASK_DDR          (1<<(SPINDLE_ENABLE_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define SPINDLE_ENABLE_DDR_RESET_MASK    (0x3<<(SPINDLE_ENABLE_BIT*2))
#define SPINDLE_ENABLE_MASK              (1<<SPINDLE_ENABLE_BIT)     // SPINDLE_ENABLE mask bit

#define SPINDLE_DIRECTION_DDR               GPIOA_MODER
#define SPINDLE_DIRECTION_PORT              GPIOA_ODR
#define SPINDLE_DIRECTION_BIT               12 // NucleoF401 Digital Pin 12
#define SPINDLE_DIRECTION_MASK_DDR          (1<<(SPINDLE_DIRECTION_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define SPINDLE_DIRECTION_DDR_RESET_MASK    (0x3<<(SPINDLE_DIRECTION_BIT*2))
#define SPINDLE_DIRECTION_MASK              (1<<SPINDLE_DIRECTION_BIT)     // SPINDLE_DIRECTION_BIT mask bit


#define HEARTBEAT_DDR               GPIOA_MODER
#define HEARTBEAT_PORT              GPIOA_ODR
#define HEARTBEAT_BIT               5 // NucleoF401 Digital Pin 5
#define HEARTBEAT_MASK_DDR          (1<<(HEARTBEAT_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define HEARTBEAT_DDR_RESET_MASK    (0x3<<(HEARTBEAT_BIT*2))
#define HEARTBEAT_MASK              (1<<HEARTBEAT_BIT)     // SPINDLE_DIRECTION_BIT mask bit

// Start of PWM & Stepper Enabled Spindle
#ifdef VARIABLE_SPINDLE
  #define SPINDLE_PWM_MAX_VALUE     settings.spindle_pwm_max_time_on // Don't change. 328p fast PWM mode fixes top value as 255.
  #ifndef SPINDLE_PWM_MIN_VALUE
    #define SPINDLE_PWM_MIN_VALUE   settings.spindle_pwm_min_time_on   // Must be greater than zero.
  #endif
  #define SPINDLE_PWM_OFF_VALUE     (settings.spindle_pwm_min_time_on)
  #define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)
  
  #define SPINDLE_PWM_DDR               GPIOA_MODER
  #define SPINDLE_PWM_PORT              GPIOA_ODR
  #define SPINDLE_PWM_BIT               6 // NucleoF401 Digital Pin 6
  #define SPINDLE_PWM_MASK_DDR          (1<<(SPINDLE_PWM_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
  #define SPINDLE_PWM_DDR_RESET_MASK    (0x3<<(SPINDLE_PWM_BIT*2))
  #define SPINDLE_PWM_MASK              (1<<SPINDLE_PWM_BIT)     // SPINDLE_PWM mask bit
#endif // End of VARIABLE_SPINDLE

// Define flood and mist coolant enable output pins.
#define COOLANT_FLOOD_DDR               GPIOC_MODER
#define COOLANT_FLOOD_PORT              GPIOC_ODR
#define COOLANT_FLOOD_BIT               1 // NucleoF401 Digital Pin 1
#define COOLANT_FLOOD_DDR_MASK          (1<<(COOLANT_FLOOD_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define COOLANT_FLOOD_DDR_RESET_MASK    (0x3<<(COOLANT_FLOOD_BIT*2))
#define COOLANT_FLOOD_MASK              (1<<COOLANT_FLOOD_BIT)     // COOLANT_FLOOD mask bit
#ifdef ENABLE_M7 // Mist coolant disabled by default. See config.h to enable/disable.
#define COOLANT_MIST_DDR               GPIOC_MODER
#define COOLANT_MIST_PORT              GPIOC_ODR
#define COOLANT_MIST_BIT               2 // NucleoF401 Digital Pin 2
#define COOLANT_MIST_DDR_MASK          (1<<(COOLANT_MIST_BIT*2)) // All (step bits*2) because the direction/mode has 2 bits
#define COOLANT_MIST_DDR_RESET_MASK    (0x3<<(COOLANT_MIST_BIT*2))
#define COOLANT_MIST_MASK              (1<<COOLANT_MIST_BIT)     // COOLANT_MIST mask bit
#endif

#define SET_GPIOS_RCCS \
  do { \
    rcc_periph_clock_enable(RCC_GPIOA); \
    rcc_periph_clock_enable(RCC_GPIOB); \
  } while (0)

#define SET_STEP_DDR \
  do { \
    STEP_X_DDR &= ~STEP_X_DDR_RESET_MASK; \
    STEP_Y_DDR &= ~STEP_Y_DDR_RESET_MASK; \
    STEP_Z_DDR &= ~STEP_Z_DDR_RESET_MASK; \
    STEP_X_DDR |= STEP_MASK_X_DDR; \
    STEP_Y_DDR |= STEP_MASK_Y_DDR; \
    STEP_Z_DDR |= STEP_MASK_Z_DDR; \
  } while (0)

#define SET_DIRECTION_DDR \
  do { \
    DIRECTION_X_DDR &= ~DIRECTION_X_DDR_RESET_MASK; \
    DIRECTION_Y_DDR &= ~DIRECTION_Y_DDR_RESET_MASK; \
    DIRECTION_Z_DDR &= ~DIRECTION_Z_DDR_RESET_MASK; \
    DIRECTION_X_DDR |= DIRECTION_MASK_X_DDR; \
    DIRECTION_Y_DDR |= DIRECTION_MASK_Y_DDR; \
    DIRECTION_Z_DDR |= DIRECTION_MASK_Z_DDR; \
  } while (0)    

#define SET_STEP_BITS(stepbits) \
  do { \
    STEP_X_PORT = (STEP_X_PORT & ~STEP_MASK_X) | (stepbits & STEP_MASK_X); \
    STEP_Y_PORT = (STEP_Y_PORT & ~STEP_MASK_Y) | (stepbits & STEP_MASK_Y); \
    STEP_Z_PORT = (STEP_Z_PORT & ~STEP_MASK_Z) | (stepbits & STEP_MASK_Z); \
  } while (0)  

#define SET_STEPS(stepbits) \
  do { \
    STEP_X_PORT  = (stepbits & STEP_MASK_X); \
    STEP_Y_PORT = (stepbits & STEP_MASK_Y); \
    STEP_Z_PORT = (stepbits & STEP_MASK_Z); \
  } while (0)
	  
#define SAVE_STEP_BITS(stepbits) \
  do { st.step_bits = (STEP_X_PORT & ~STEP_MASK_X) | (STEP_Y_PORT & ~STEP_MASK_Y) | (STEP_Z_PORT & ~STEP_MASK_Z) | stepbits;} while (0)

#define SET_DIRECTION_BITS(dirbits) \
  do { \
    DIRECTION_Z_PORT  = (DIRECTION_Z_PORT & ~DIRECTION_MASK_Z) | (dirbits & DIRECTION_MASK_Z); \
    DIRECTION_X_PORT = (DIRECTION_X_PORT & ~DIRECTION_MASK_X) | (dirbits & DIRECTION_MASK_X); \
    DIRECTION_Y_PORT = (DIRECTION_Y_PORT & ~DIRECTION_MASK_Y) | (dirbits & DIRECTION_MASK_Y); \
  } while (0)  
	  
#define SET_LIMITS_RCC \
  do { \
    rcc_periph_clock_enable(RCC_GPIOB); \
    rcc_periph_clock_enable(RCC_GPIOC); \
    rcc_periph_clock_enable(RCC_SYSCFG); \
} while (0)

#define SET_SYSTEM_RCC \
  do { \
    rcc_periph_clock_enable(RCC_GPIOA); \
    rcc_periph_clock_enable(RCC_GPIOB); \
    rcc_periph_clock_enable(RCC_GPIOC); \
    rcc_periph_clock_enable(RCC_SYSCFG); \
} while (0)
	  
/* set limits pins as inputs */
#define SET_LIMITS_DDR \
  do { \
    LIMIT_X_DDR &= ~LIMIT_X_PU_RESET_MASK; \
    LIMIT_Y_DDR  &= ~LIMIT_Y_PU_RESET_MASK; \
    LIMIT_Z_DDR &= ~LIMIT_Z_PU_RESET_MASK; \
  } while (0)

/* unset pull-up for limits pin */
#define UNSET_LIMITS_PU \
  do { \
    LIMIT_X_PU  &= ~LIMIT_X_PU_RESET_MASK; \
    LIMIT_Y_PU  &= ~LIMIT_Y_PU_RESET_MASK; \
    LIMIT_Z_PU  &= ~LIMIT_Z_PU_RESET_MASK; \
    } while (0)

/* set pull-up for limits pin */
#define SET_LIMITS_PU \
  do { \
    LIMIT_X_PU  &= ~LIMIT_X_PU_RESET_MASK; \
    LIMIT_Y_PU  &= ~LIMIT_Y_PU_RESET_MASK; \
    LIMIT_Z_PU  &= ~LIMIT_Z_PU_RESET_MASK; \
    LIMIT_X_PU  |= LIMIT_X_PU_MASK; \
    LIMIT_Y_PU  |= LIMIT_Y_PU_MASK; \
    LIMIT_Z_PU  |= LIMIT_Z_PU_MASK; \
    } while (0)

/* get limits pin status */
#define GET_LIMIT_PIN \
  ((LIMIT_X_PIN & LIMIT_X_MASK) | (LIMIT_Y_PIN & LIMIT_Y_MASK) | (LIMIT_Z_PIN & LIMIT_Z_MASK))

/* get controls pin status */
#define GET_CONTROLS_PIN \
  ((RESET_CONTROL_PIN & RESET_CONTROL_MASK) | \
   (FEED_HOLD_CONTROL_PIN & FEED_HOLD_MASK) | \
   (CYCLE_START_CONTROL_PIN & CYCLE_START_MASK) | \
   (SAFETY_DOOR_CONTROL_PIN & SAFETY_DOOR_MASK))

/* set control pins as inputs */
#define SET_CONTROLS_DDR \
  do { \
    RESET_CONTROL_DDR &= ~RESET_CONTROL_PU_RESET_MASK; \
    FEED_HOLD_CONTROL_DDR  &= ~FEED_HOLD_PU_RESET_MASK; \
    CYCLE_START_CONTROL_DDR  &= ~CYCLE_START_PU_RESET_MASK; \
    SAFETY_DOOR_CONTROL_DDR  &= ~SAFETY_DOOR_PU_RESET_MASK; \
  } while (0)

/* unset pull-up for controls pin */
#define UNSET_CONTROLS_PU \
  do { \
    RESET_CONTROL_PU &= ~RESET_CONTROL_PU_RESET_MASK; \
    FEED_HOLD_CONTROL_PU  &= ~FEED_HOLD_PU_RESET_MASK; \
    CYCLE_START_CONTROL_PU  &= ~CYCLE_START_PU_RESET_MASK; \
    SAFETY_DOOR_CONTROL_PU  &= ~SAFETY_DOOR_PU_RESET_MASK; \
  } while (0)

/* set pull-up for controls pin */
#define SET_CONTROLS_PU \
  do { \
    RESET_CONTROL_PU        &= ~RESET_CONTROL_PU_RESET_MASK; \
    FEED_HOLD_CONTROL_PU    &= ~FEED_HOLD_PU_RESET_MASK; \
    CYCLE_START_CONTROL_PU  &= ~CYCLE_START_PU_RESET_MASK; \
    SAFETY_DOOR_CONTROL_PU  &= ~SAFETY_DOOR_PU_RESET_MASK; \
    RESET_CONTROL_PU        |= RESET_CONTROL_PU_MASK; \
    FEED_HOLD_CONTROL_PU    |= FEED_HOLD_PU_MASK; \
    CYCLE_START_CONTROL_PU  |= CYCLE_START_PU_MASK; \
    SAFETY_DOOR_CONTROL_PU  |= SAFETY_DOOR_PU_MASK; \
  } while (0)

#define GET_SPINDLE_DIRECTION_PIN \
  (SPINDLE_DIRECTION_PORT & SPINDLE_DIRECTION_MASK)

#define SET_SPINDLE_DIRECTION_DDR \
  do { \
    SPINDLE_DIRECTION_DDR &= ~SPINDLE_DIRECTION_DDR_RESET_MASK; \
    SPINDLE_DIRECTION_DDR |= SPINDLE_DIRECTION_MASK_DDR; \
  } while (0)
      
#define SET_SPINDLE_DIRECTION_BIT \
  do { \
    SPINDLE_DIRECTION_PORT |= SPINDLE_DIRECTION_MASK; \
  } while (0)  
         
#define UNSET_SPINDLE_DIRECTION_BIT \
  do { \
    SPINDLE_DIRECTION_PORT &= ~(SPINDLE_DIRECTION_MASK); \
  } while (0)  
      
#define SET_SPINDLE_PWM_DDR \
  do { \
    SPINDLE_PWM_DDR &= ~SPINDLE_PWM_DDR_RESET_MASK; \
    SPINDLE_PWM_DDR |= SPINDLE_PWM_MASK_DDR; \
  } while (0)
      
#define SET_SPINDLE_ENABLE_DDR \
  do { \
    SPINDLE_ENABLE_DDR &= ~SPINDLE_ENABLE_DDR_RESET_MASK; \
    SPINDLE_ENABLE_DDR |= SPINDLE_ENABLE_MASK_DDR; \
  } while (0)
      
#define GET_SPINDLE_ENABLE \
  (SPINDLE_ENABLE_PORT & SPINDLE_ENABLE_MASK)

/* Set spindle enable pin */
#define  SET_SPINDLE_ENABLE \
  do { \
    SPINDLE_ENABLE_PORT |= SPINDLE_ENABLE_MASK;  \
  } while (0)

/* Unset spindle enable pin */
#define  UNSET_SPINDLE_ENABLE \
  do { \
    SPINDLE_ENABLE_PORT &= ~SPINDLE_ENABLE_MASK;  \
  } while (0)

/* set probe pin as inputs */
#define SET_PROBE_DDR \
do{ \
	PROBE_DDR &= ~PROBE_PU_RESET_MASK; \
} while (0)

/* unset pull-up for limits pin */
#define UNSET_PROBE_PU \
do{ \
	PROBE_PU  &= ~PROBE_PU_RESET_MASK; \
} while (0)

/* set pull-up for limits pin */
#define SET_PROBE_PU \
do{ \
	PROBE_PU  &= ~PROBE_PU_RESET_MASK; \
	PROBE_PU  |= PROBE_PU_MASK; \
} while (0)

#define SET_COOLANT_FLOOD_DDR \
  do { \
    COOLANT_FLOOD_DDR &= ~COOLANT_FLOOD_DDR_RESET_MASK; \
    COOLANT_FLOOD_DDR |= COOLANT_FLOOD_DDR_MASK; \
  } while (0)

#define SET_COOLANT_FLOOD_BIT \
  do { \
    COOLANT_FLOOD_PORT |= COOLANT_FLOOD_MASK; \
  } while (0)

#define UNSET_COOLANT_FLOOD_BIT \
  do { \
    COOLANT_FLOOD_PORT &= ~(COOLANT_FLOOD_MASK); \
  } while (0)


#define SET_COOLANT_MIST_DDR \
  do { \
    COOLANT_MIST_DDR &= ~COOLANT_MIST_DDR_RESET_MASK; \
    COOLANT_MIST_DDR |= COOLANT_MIST_DDR_MASK; \
  } while (0)


#define SET_COOLANT_MIST_BIT \
  do { \
    COOLANT_MIST_PORT |= COOLANT_MIST_MASK; \
  } while (0)

#define UNSET_COOLANT_MIST_BIT \
  do { \
    COOLANT_MIST_PORT &= ~(COOLANT_MIST_MASK); \
  } while (0)


#define SET_HEARTBEAT_DDR \
  do { \
    HEARTBEAT_DDR &= ~HEARTBEAT_DDR_RESET_MASK; \
    HEARTBEAT_DDR |= HEARTBEAT_MASK_DDR; \
  } while (0)

#define TOGGLE_HEARTBEAT_BIT \
  do { \
    HEARTBEAT_PORT ^= HEARTBEAT_MASK; \
  } while (0)

#endif
