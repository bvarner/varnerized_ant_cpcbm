/*
  spindle_control.c - spindle control methods
  Part of grbl_port_opencm3 project, derived from the Grbl work.

  Copyright (c) 2017-2020 The Ant Team
  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
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

/* Implementation of Bresenham's Line Algorithm, negative gradient.
 * See the plotLine function description.
 */
static void plotLineLow(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t * y_result)
{
	uint32_t x;
    int32_t dx = (int32_t)(x1 - x0);
    int32_t dy = (int32_t)(y1 - y0);

    int32_t yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    int32_t D = (2 * dy) - dx;
    int32_t y = y0;

    for(x = x0; x < x1; x++)
    {
        y_result[x] = y;
        if(D > 0)
        {
            y = y + yi;
            D = D + (2 * (dy - dx));
        }
        else
        {
            D = D + 2*dy;
        }
    }
}

/* Implementation of Bresenham's Line Algorithm, positive gradient.
 * See the plotLine function description.
 */
static void plotLineHigh(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t * y_result)
{
	uint32_t y;
	int32_t dx = (int32_t)(x1 - x0);
	int32_t dy = (int32_t)(y1 - y0);

	int32_t xi = 1;
	int32_t x = x0;
    if(dx < 0)
    {
        xi = -1;
        dx = -dx;
        x -= 1;
    }
    int32_t D = (2 * dx) - dy;

    for(y = y0; y < y1; y++)
    {
        if(D > 0)
        {
            y_result[x] = y;
            x = x + xi;
            D = D + (2 * (dx - dy));
        }
        else
        {
            D = D + 2*dx;
        }
    }
}

/* Implementation of Bresenham's Line Algorithm
 * This function is used for pwm spindle ramping, to calculate the points of a line, where
 * each y point is a pwm that is applied in a increasing or decreasing linear progression.
 * For further information see: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */
static void plotLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t * y_result)
{
	if(abs(y1 - y0) < abs(x1 - x0))
	{
        if(x0 > x1)
            plotLineLow(x1, y1, x0, y0, y_result);
        else
            plotLineLow(x0, y0, x1, y1, y_result);
	}
    else
    {
        if(y0 > y1)
            plotLineHigh(x1, y1, x0, y0, y_result);
        else
            plotLineHigh(x0, y0, x1, y1, y_result);
    }
}

#ifdef VARIABLE_SPINDLE
  static float pwm_gradient; // Pre-calulated value to speed up rpm to PWM conversions.
#endif


#ifdef NUCLEO

#include <libopencm3/stm32/timer.h>

void spindle_init()
{    
  //prescaler to get 1 us granularity
	uint32_t prescaler = (SPINDLE_TIMER_BUS_FREQ / 1000000)-1;

  #ifdef VARIABLE_SPINDLE

  // Configure variable spindle PWM and enable pin, if required. On the Uno, PWM and enable are
  // combined unless configured otherwise.
    SET_SPINDLE_PWM_DDR; // Configure as PWM output pin.
    #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
      SET_SPINDLE_ENABLE_DDR; // Configure as output pin.
  #else  
      SET_SPINDLE_DIRECTION_DDR; // Configure as output pin.
  #endif
  
    pwm_gradient = SPINDLE_PWM_RANGE/(settings.rpm_max-settings.rpm_min);

  #else

    // Configure no variable spindle and only enable pin.
    SET_SPINDLE_ENABLE_DDR; // Configure as output pin.
    SET_SPINDLE_DIRECTION_DDR; // Configure as output pin.
  
  #endif
    /* Enable SPINDLE_TIMER clock. */
    rcc_periph_clock_enable(SPINDLE_TIMER_RCC);
    rcc_periph_reset_pulse(SPINDLE_TIMER_RST);
    /* Continous mode. */
    timer_continuous_mode(SPINDLE_TIMER);
    /* Timer global mode:
     * - No divider
     * - Alignment edge
     * - Direction up
     */
    timer_set_mode(SPINDLE_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_clock_division(SPINDLE_TIMER, 0);            // Adjusts speed of timer
   	timer_set_master_mode(SPINDLE_TIMER, TIM_CR2_MMS_UPDATE);   // Master Mode Selection
   	/* ARR reload enable. */
    timer_enable_preload(SPINDLE_TIMER);

    /* Set output compare mode */
	timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_PWM_TYPE);
	timer_enable_oc_preload(SPINDLE_TIMER, SPINDLE_TIMER_CHAN);           // Sets OCxPE in TIMx_CCMRx
	timer_set_oc_polarity_high(SPINDLE_TIMER, SPINDLE_TIMER_CHAN);        // set desired polarity in TIMx_CCER
	timer_set_oc_value(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, 0);
	timer_enable_oc_output(SPINDLE_TIMER, SPINDLE_TIMER_CHAN);
    timer_set_prescaler(SPINDLE_TIMER, prescaler);// set to get 1 us granularity


	gpio_mode_setup(SPINDLE_GPIO_GROUP, GPIO_MODE_AF, GPIO_PUPD_NONE, SPINDLE_GPIO);
	gpio_set_af(SPINDLE_GPIO_GROUP, SPINDLE_GPIO_AF, SPINDLE_GPIO);
    
    spindle_set_state(SPINDLE_DISABLE,0.0);
}


uint8_t spindle_get_state()
{
	#ifdef VARIABLE_SPINDLE
    #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
		  // No spindle direction output pin. 
			#ifdef INVERT_SPINDLE_ENABLE_PIN
			  if (bit_isfalse(GET_SPINDLE_ENABLE)) { return(SPINDLE_STATE_CW); }
	    #else
	 			if (bit_istrue(GET_SPINDLE_ENABLE)) { return(SPINDLE_STATE_CW); }
	    #endif
    #else
      if (TIM_CCMR1(SPINDLE_TIMER) & SPINDLE_TIMER_PWM_TYPE) { // Check if PWM is enabled.
        if (GET_SPINDLE_DIRECTION_PIN) { return(SPINDLE_STATE_CCW); }
        else { return(SPINDLE_STATE_CW); }
      }
    #endif
	#else
		#ifdef INVERT_SPINDLE_ENABLE_PIN
		  if (bit_isfalse(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) { 
		#else
		  if (bit_istrue(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) {
		#endif
      if (GET_SPINDLE_DIRECTION_PIN) { return(SPINDLE_STATE_CCW); }
      else { return(SPINDLE_STATE_CW); }
    }
	#endif
	return(SPINDLE_STATE_DISABLE);
} 




// Disables the spindle and sets PWM output to zero when PWM variable spindle speed is enabled.
// Called by various main program and ISR routines. Keep routine small, fast, and efficient.
// Called by spindle_init(), spindle_set_speed(), spindle_set_state(), and mc_reset().
void spindle_stop()
{
  // On the Nucleo, spindle enable and PWM are shared. Other CPUs have separate enable pin.
  #ifdef VARIABLE_SPINDLE
    /* Disable PWM. Output voltage is zero. */
    timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_OFF_TYPE);
    /* Counter disable. */
    timer_disable_counter(SPINDLE_TIMER);
    #if defined(USE_SPINDLE_DIR_AS_ENABLE_PIN)
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SET_SPINDLE_ENABLE;  // Set pin to high
      #else
        UNSET_SPINDLE_ENABLE; // Set pin to low
      #endif
    #endif
  #else
    #ifdef INVERT_SPINDLE_ENABLE_PIN
        SET_SPINDLE_ENABLE;  // Set pin to high
    #else
        UNSET_SPINDLE_ENABLE; // Set pin to low
    #endif
  #endif  
}

// Immediately sets spindle running state with direction and spindle rpm via PWM, if enabled.
// Called by g-code parser spindle_sync(), parking retract and restore, g-code program end,
// sleep, and spindle stop override.
#ifdef VARIABLE_SPINDLE
  void spindle_set_state(uint8_t state, float rpm)
#else
  void _spindle_set_state(uint8_t state)
#endif
{
  uint16_t current_pwm;
  uint32_t spindle_pwm_range;
  uint32_t pwm_prog_y[MAX_RAMPING_DIVISIONS];
  uint32_t ramping_divs = settings.spindle_pwm_ramping_divisions;

  /* Ramps for laser mode is managed elsewhere. */
  if(bit_istrue(settings.flags,BITFLAG_LASER_MODE))
  {
	  ramping_divs = 0;
  }

  if (sys.abort) { return; } // Block during abort.
  if (state == SPINDLE_DISABLE && bit_istrue(settings.flags,BITFLAG_LASER_MODE)) { // Halt or set spindle direction and rpm.
#ifdef VARIABLE_SPINDLE
    sys.spindle_speed = 0.0;
#endif
    spindle_stop();
  }
  else
  {
#ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
    if(state == SPINDLE_ENABLE_CW)
    {
        UNSET_SPINDLE_DIRECTION_BIT;
    }
    else
    {
        SET_SPINDLE_DIRECTION_BIT;
    }
#endif

#ifdef VARIABLE_SPINDLE

    /* PWM settings done in the init, shall not need to be repeated. */
    timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_PWM_TYPE);
    timer_set_period(SPINDLE_TIMER, settings.spindle_pwm_period);
        

    spindle_pwm_range = (settings.spindle_pwm_max_time_on-settings.spindle_pwm_min_time_on);

    if (rpm < 0.0 || spindle_pwm_range <= 0.0 || state == SPINDLE_DISABLE) { rpm = 0.0;  } // RPM should never be negative, but check anyway.
    if ( rpm < SPINDLE_MIN_RPM ) { rpm = 0; }
    else if ( rpm > SPINDLE_MAX_RPM ) { rpm = SPINDLE_MAX_RPM; }

    current_pwm = spindle_compute_pwm_value(rpm);
    timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_PWM_TYPE);
    /* Counter enable. */
    timer_enable_counter(SPINDLE_TIMER);

    plotLine(0, TIM_CCR1(SPINDLE_TIMER), ramping_divs, current_pwm, pwm_prog_y);

	for(uint32_t i = 0; i < ramping_divs; i++)
	{
		timer_set_oc_value(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, pwm_prog_y[i]);// Set PWM pin output
		delay_ms(DEFAULT_PWM_RAMPING_DELAY);
	}

    timer_set_oc_value(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, current_pwm);// Set PWM pin output

	// if spindle enable and PWM are shared, unless otherwise specified.
	#if defined(USE_SPINDLE_DIR_AS_ENABLE_PIN)
	  #ifdef INVERT_SPINDLE_ENABLE_PIN
		UNSET_SPINDLE_ENABLE;  // Set pin to low
	  #else
		SET_SPINDLE_ENABLE;  // Set pin to high
	  #endif
	#endif

#else
  // NOTE: Without variable spindle, the enable bit should just turn on or off, regardless
  // if the spindle speed value is zero, as its ignored anyhow.
  #ifdef INVERT_SPINDLE_ENABLE_PIN
	UNSET_SPINDLE_ENABLE;  // Set pin to low
  #else
	SET_SPINDLE_ENABLE;  // Set pin to high
  #endif
#endif

	if (state == SPINDLE_DISABLE)
	{ // Halt or set spindle direction and rpm.
	#ifdef VARIABLE_SPINDLE
	    sys.spindle_speed = 0.0;
	#endif
	    spindle_stop();
	}

  }
  
  sys.report_ovr_counter = 0; // Set to report change immediately
}

#ifdef VARIABLE_SPINDLE
  // Sets spindle speed PWM output and enable pin, if configured. Called by spindle_set_state()
  // and stepper ISR. Keep routine small and efficient.
  void spindle_set_speed(uint16_t pwm_value)
  {
     timer_set_oc_value(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, pwm_value);// Set PWM output level.
    #ifdef SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
      if (pwm_value == SPINDLE_PWM_OFF_VALUE) {
        spindle_stop();
      } else {
        // Ensure PWM output is enabled.
        timer_enable_oc_output(SPINDLE_TIMER, SPINDLE_TIMER_CHAN);
        timer_enable_break_main_output(SPINDLE_TIMER);

        /* Counter enable. */
        timer_enable_counter(SPINDLE_TIMER); 
        #ifdef INVERT_SPINDLE_ENABLE_PIN
        UNSET_SPINDLE_ENABLE;  // Set pin to low
      #else
        SET_SPINDLE_ENABLE;  // Set pin to high
        #endif
      }
    #else
      if (pwm_value == SPINDLE_PWM_OFF_VALUE) {
        //timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_OFF_TYPE); // Disable PWM. Output voltage is zero.
    	timer_set_oc_value(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_PWM_OFF_VALUE);// Set PWM min level. TODO: If this is ok for both laser and spindle remove previous line.
      } else {
        timer_set_oc_mode(SPINDLE_TIMER, SPINDLE_TIMER_CHAN, SPINDLE_TIMER_PWM_TYPE);; // Ensure PWM output is enabled.
      }
    #endif
  }
#endif

#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE

  // Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
  uint16_t spindle_compute_pwm_value(float rpm) // Nucleo PWM register is 16-bit.
  {
    uint16_t pwm_value;
    rpm *= (0.010*sys.spindle_speed_ovr); // Scale by spindle speed override value.
    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if ((settings.rpm_min >= settings.rpm_max) || (rpm >= RPM_MAX)) {
      rpm = RPM_MAX;
      pwm_value = settings.spindle_pwm_max_time_on;
    } else if (rpm <= RPM_MIN) {
      if (rpm == 0.0) { // S0 disables spindle
        pwm_value = SPINDLE_PWM_OFF_VALUE;
      } else {
        rpm = RPM_MIN;
        pwm_value = settings.spindle_pwm_min_time_on;
      }
    } else {
      // Compute intermediate PWM value with linear spindle speed model via piecewise linear fit model.
      #if (N_PIECES > 3)
        if (rpm > RPM_POINT34) {
          pwm_value = floor(RPM_LINE_A4*rpm - RPM_LINE_B4);
        } else
      #endif
      #if (N_PIECES > 2)
        if (rpm > RPM_POINT23) {
          pwm_value = floor(RPM_LINE_A3*rpm - RPM_LINE_B3);
        } else
      #endif
      #if (N_PIECES > 1)
        if (rpm > RPM_POINT12) {
          pwm_value = floor(RPM_LINE_A2*rpm - RPM_LINE_B2);
        } else
      #endif
      {
        pwm_value = floor(RPM_LINE_A1*rpm - RPM_LINE_B1);
      }
    }
    sys.spindle_speed = rpm;
    return(pwm_value);
  }

#else

  // Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
  uint16_t spindle_compute_pwm_value(float rpm) // Nucleo PWM register is 16-bit.
  {
    uint16_t pwm_value;
    rpm *= (0.010*sys.spindle_speed_ovr); // Scale by spindle speed override value.
    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if ((settings.rpm_min >= settings.rpm_max) || (rpm >= settings.rpm_max)) {
      // No PWM range possible. Set simple on/off spindle control pin state.
      sys.spindle_speed = settings.rpm_max;
      pwm_value = settings.spindle_pwm_max_time_on;
    } else if (rpm <= settings.rpm_min) {
      if (rpm == 0.0) { // S0 disables spindle
        sys.spindle_speed = 0.0;
        pwm_value = SPINDLE_PWM_OFF_VALUE;
      } else { // Set minimum PWM output
        sys.spindle_speed = settings.rpm_min;
        pwm_value = settings.spindle_pwm_min_time_on;
      }
    } else {
      // Compute intermediate PWM value with linear spindle speed model.
      // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
      sys.spindle_speed = rpm;
      pwm_value = floor((rpm-settings.rpm_min)*pwm_gradient) + settings.spindle_pwm_min_time_on;
    }
    return(pwm_value);
  }

#endif


#else

void spindle_init()
{    
  // Configure variable spindle PWM and enable pin, if requried. On the Uno, PWM and enable are
  // combined unless configured otherwise.
  #ifdef VARIABLE_SPINDLE
    SPINDLE_PWM_DDR |= (1<<SPINDLE_PWM_BIT); // Configure as PWM output pin.
    #if defined(CPU_MAP_ATMEGA2560) || defined(USE_SPINDLE_DIR_AS_ENABLE_PIN)
      SPINDLE_ENABLE_DDR |= (1<<SPINDLE_ENABLE_BIT); // Configure as output pin.
    #endif     
  // Configure no variable spindle and only enable pin.
  #else  
    SPINDLE_ENABLE_DDR |= (1<<SPINDLE_ENABLE_BIT); // Configure as output pin.
  #endif
  
  #ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
    SPINDLE_DIRECTION_DDR |= (1<<SPINDLE_DIRECTION_BIT); // Configure as output pin.
  #endif
  spindle_set_state(SPINDLE_DISABLE,0.0);
}


  #ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
  
    // Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
    uint8_t spindle_compute_pwm_value(float rpm) // 328p PWM register is 8-bit.
    {
      uint8_t pwm_value;
      rpm *= (0.010*sys.spindle_speed_ovr); // Scale by spindle speed override value.
      // Calculate PWM register value based on rpm max/min settings and programmed rpm.
      if ((settings.rpm_min >= settings.rpm_max) || (rpm >= RPM_MAX)) {
        rpm = RPM_MAX;
        pwm_value = SPINDLE_PWM_MAX_VALUE;
      } else if (rpm <= RPM_MIN) {
        if (rpm == 0.0) { // S0 disables spindle
          pwm_value = SPINDLE_PWM_OFF_VALUE;
        } else {
          rpm = RPM_MIN;
          pwm_value = SPINDLE_PWM_MIN_VALUE;
        }
      } else {
        // Compute intermediate PWM value with linear spindle speed model via piecewise linear fit model.
        #if (N_PIECES > 3)
          if (rpm > RPM_POINT34) {
            pwm_value = floor(RPM_LINE_A4*rpm - RPM_LINE_B4);
          } else 
        #endif
        #if (N_PIECES > 2)
          if (rpm > RPM_POINT23) {
            pwm_value = floor(RPM_LINE_A3*rpm - RPM_LINE_B3);
          } else 
        #endif
        #if (N_PIECES > 1)
          if (rpm > RPM_POINT12) {
            pwm_value = floor(RPM_LINE_A2*rpm - RPM_LINE_B2);
          } else 
        #endif
        {
          pwm_value = floor(RPM_LINE_A1*rpm - RPM_LINE_B1);
        }
      }
      sys.spindle_speed = rpm;
      return(pwm_value);
    }
    
  #else 
  
    // Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
    uint8_t spindle_compute_pwm_value(float rpm) // 328p PWM register is 8-bit.
    {
      uint8_t pwm_value;
      rpm *= (0.010*sys.spindle_speed_ovr); // Scale by spindle speed override value.
      // Calculate PWM register value based on rpm max/min settings and programmed rpm.
      if ((settings.rpm_min >= settings.rpm_max) || (rpm >= settings.rpm_max)) {
        // No PWM range possible. Set simple on/off spindle control pin state.
        sys.spindle_speed = settings.rpm_max;
        pwm_value = SPINDLE_PWM_MAX_VALUE;
      } else if (rpm <= settings.rpm_min) {
        if (rpm == 0.0) { // S0 disables spindle
          sys.spindle_speed = 0.0;
          pwm_value = SPINDLE_PWM_OFF_VALUE;
        } else { // Set minimum PWM output
          sys.spindle_speed = settings.rpm_min;
          pwm_value = SPINDLE_PWM_MIN_VALUE;
        }
      } else { 
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
        pwm_value = floor((rpm-settings.rpm_min)*pwm_gradient) + SPINDLE_PWM_MIN_VALUE;
      }
      return(pwm_value);
    }
    
  #endif
  
uint8_t spindle_get_state()
{
	#ifdef VARIABLE_SPINDLE
    #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
		  // No spindle direction output pin. 
			#ifdef INVERT_SPINDLE_ENABLE_PIN
			  if (bit_isfalse(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) { return(SPINDLE_STATE_CW); }
	    #else
	 			if (bit_istrue(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) { return(SPINDLE_STATE_CW); }
	    #endif
    #else
      if (SPINDLE_TCCRA_REGISTER & (1<<SPINDLE_COMB_BIT)) { // Check if PWM is enabled.
        if (SPINDLE_DIRECTION_PORT & (1<<SPINDLE_DIRECTION_BIT)) { return(SPINDLE_STATE_CCW); }
        else { return(SPINDLE_STATE_CW); }
      }
    #endif
	#else
		#ifdef INVERT_SPINDLE_ENABLE_PIN
		  if (bit_isfalse(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) { 
		#else
		  if (bit_istrue(SPINDLE_ENABLE_PORT,(1<<SPINDLE_ENABLE_BIT))) {
		#endif
      if (SPINDLE_DIRECTION_PORT & (1<<SPINDLE_DIRECTION_BIT)) { return(SPINDLE_STATE_CCW); }
      else { return(SPINDLE_STATE_CW); }
    }
	#endif
	return(SPINDLE_STATE_DISABLE);
}


void spindle_stop()
{
  // On the Uno, spindle enable and PWM are shared. Other CPUs have seperate enable pin.
  #ifdef VARIABLE_SPINDLE
    TCCRA_REGISTER &= ~(1<<COMB_BIT); // Disable PWM. Output voltage is zero.
    #if defined(CPU_MAP_ATMEGA2560) || defined(USE_SPINDLE_DIR_AS_ENABLE_PIN)
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);  // Set pin to high
      #else
        SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT); // Set pin to low
      #endif
    #endif
  #else
    #ifdef INVERT_SPINDLE_ENABLE_PIN
      SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);  // Set pin to high
    #else
      SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT); // Set pin to low
    #endif
  #endif  
}

void spindle_set_state(uint8_t state, float rpm_in)
{
  float rpm = rpm_in;
  uint8_t previous_pwm = OCR_REGISTER;

  uint8_t dx = 10;
  int8_t dy;
  int8_t D;

  // Halt or set spindle direction and rpm. 
  if (state == SPINDLE_DISABLE) {
	  rpm = 0;
  }


    #ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
      if (state == SPINDLE_ENABLE_CW) {
        SPINDLE_DIRECTION_PORT &= ~(1<<SPINDLE_DIRECTION_BIT);
      } else {
        SPINDLE_DIRECTION_PORT |= (1<<SPINDLE_DIRECTION_BIT);
      }
    #endif

    #ifdef VARIABLE_SPINDLE
      // TODO: Install the optional capability for frequency-based output for servos.
      #ifdef CPU_MAP_ATMEGA2560
      	TCCRA_REGISTER = (1<<COMB_BIT) | (1<<WAVE1_REGISTER) | (1<<WAVE0_REGISTER);
        TCCRB_REGISTER = (TCCRB_REGISTER & 0b11111000) | 0x02 | (1<<WAVE2_REGISTER) | (1<<WAVE3_REGISTER); // set to 1/8 Prescaler
        OCR4A = 0xFFFF; // set the top 16bit value
        uint16_t current_pwm;
      #else
        TCCRA_REGISTER = (1<<COMB_BIT) | (1<<WAVE1_REGISTER) | (1<<WAVE0_REGISTER);
        TCCRB_REGISTER = (TCCRB_REGISTER & 0b11111000) | 0x02; // set to 1/8 Prescaler
        uint8_t current_pwm;
      #endif

      if (rpm <= 0.0) { rpm = 0; } // RPM should never be negative, but check anyway.

        #define SPINDLE_RPM_RANGE (SPINDLE_MAX_RPM-SPINDLE_MIN_RPM)
        if ( rpm < SPINDLE_MIN_RPM ) { rpm = 0; } 
        else { 
          rpm -= SPINDLE_MIN_RPM; 
          if ( rpm > SPINDLE_RPM_RANGE ) { rpm = SPINDLE_RPM_RANGE; } // Prevent integer overflow
        }
        current_pwm = floor( rpm*(PWM_MAX_VALUE/SPINDLE_RPM_RANGE) + 0.5);
        #ifdef MINIMUM_SPINDLE_PWM
          if (current_pwm < MINIMUM_SPINDLE_PWM) { current_pwm = MINIMUM_SPINDLE_PWM; }
        #endif
        OCR_REGISTER = current_pwm; // Set PWM pin output

        plotLine(0, previous_pwm, 10, current_pwm);

        // On the Uno, spindle enable and PWM are shared, unless otherwise specified.
        #if defined(CPU_MAP_ATMEGA2560) || defined(USE_SPINDLE_DIR_AS_ENABLE_PIN) 
          #ifdef INVERT_SPINDLE_ENABLE_PIN
            SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);
          #else
            SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);
          #endif
        #endif
      
    #endif
    #if (defined(USE_SPINDLE_DIR_AS_ENABLE_PIN) && \
        !defined(SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED)) || !defined(VARIABLE_SPINDLE)
      // NOTE: Without variable spindle, the enable bit should just turn on or off, regardless
      // if the spindle speed value is zero, as its ignored anyhow.      
      #ifdef INVERT_SPINDLE_ENABLE_PIN
        SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);
      #else
        SPINDLE_ENABLE_PORT |= (1<<SPINDLE_ENABLE_BIT);
      #endif
    #endif

	if (state == SPINDLE_DISABLE) {
		spindle_stop();
	}
}

#endif //NUCLEO_F401


// G-code parser entry-point for setting spindle state. Forces a planner buffer sync and bails 
// if an abort or check-mode is active.
#ifdef VARIABLE_SPINDLE
  void spindle_sync(uint8_t state, float rpm)
  {
    if (sys.state == STATE_CHECK_MODE) { return; }
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    spindle_set_state(state,rpm);
  }
#else
  void _spindle_sync(uint8_t state)
  {
    if (sys.state == STATE_CHECK_MODE) { return; }
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    _spindle_set_state(state);
  }
#endif
