/*
  settings.c - memory(flash or eeprom) configuration handling
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

settings_t settings;

void write_global_settings(void);
uint8_t read_global_settings(void);

#ifdef NUCLEO
#define __flash
#endif
const __flash settings_t defaults = {\
    .pulse_microseconds = DEFAULT_STEP_PULSE_MICROSECONDS,
    .stepper_idle_lock_time = DEFAULT_STEPPER_IDLE_LOCK_TIME,
    .step_invert_mask = DEFAULT_STEPPING_INVERT_MASK,
    .dir_invert_mask = DEFAULT_DIRECTION_INVERT_MASK,
    .status_report_mask = DEFAULT_STATUS_REPORT_MASK,
    .junction_deviation = DEFAULT_JUNCTION_DEVIATION,
    .arc_tolerance = DEFAULT_ARC_TOLERANCE,
    .rpm_max = DEFAULT_SPINDLE_RPM_MAX,
    .rpm_min = DEFAULT_SPINDLE_RPM_MIN,
    .homing_dir_mask = DEFAULT_HOMING_DIR_MASK,
    .homing_feed_rate = DEFAULT_HOMING_FEED_RATE,
    .homing_seek_rate = DEFAULT_HOMING_SEEK_RATE,
    .homing_debounce_delay = DEFAULT_HOMING_DEBOUNCE_DELAY,
    .homing_pulloff = DEFAULT_HOMING_PULLOFF,
    .spindle_pwm_period = DEFAULT_SPINDLE_PWM_PERIOD,
    .spindle_pwm_max_time_on = DEFAULT_SPINDLE_PWM_MAX_TIME_ON,
    .spindle_pwm_min_time_on = DEFAULT_SPINDLE_PWM_MIN_TIME_ON,
    .spindle_pwm_enable_at_start = DEFAULT_SPINDLE_PWM_ENABLE_AT_START,
    .flags = (DEFAULT_REPORT_INCHES << BIT_REPORT_INCHES) | \
             (DEFAULT_LASER_MODE << BIT_LASER_MODE) | \
             (DEFAULT_INVERT_ST_ENABLE << BIT_INVERT_ST_ENABLE) | \
             (DEFAULT_HARD_LIMIT_ENABLE << BIT_HARD_LIMIT_ENABLE) | \
             (DEFAULT_HOMING_ENABLE << BIT_HOMING_ENABLE) | \
             (DEFAULT_SOFT_LIMIT_ENABLE << BIT_SOFT_LIMIT_ENABLE) | \
             (DEFAULT_INVERT_LIMIT_PINS << BIT_INVERT_LIMIT_PINS) | \
             (DEFAULT_INVERT_PROBE_PIN << BIT_INVERT_PROBE_PIN),
    .steps_per_mm[X_AXIS] = DEFAULT_X_STEPS_PER_MM,
    .steps_per_mm[Y_AXIS] = DEFAULT_Y_STEPS_PER_MM,
    .steps_per_mm[Z_AXIS] = DEFAULT_Z_STEPS_PER_MM,
    .max_rate[X_AXIS] = DEFAULT_X_MAX_RATE,
    .max_rate[Y_AXIS] = DEFAULT_Y_MAX_RATE,
    .max_rate[Z_AXIS] = DEFAULT_Z_MAX_RATE,
    .acceleration[X_AXIS] = DEFAULT_X_ACCELERATION,
    .acceleration[Y_AXIS] = DEFAULT_Y_ACCELERATION,
    .acceleration[Z_AXIS] = DEFAULT_Z_ACCELERATION,
    .max_travel[X_AXIS] = (-DEFAULT_X_MAX_TRAVEL),
    .max_travel[Y_AXIS] = (-DEFAULT_Y_MAX_TRAVEL),
    .max_travel[Z_AXIS] = (-DEFAULT_Z_MAX_TRAVEL),
    .homing_debug = DEFAULT_HOMING_DEBUG_EN,
	.spindle_pwm_ramping_divisions = DEFAULT_PWM_RAMPING_DIVS
};

#ifdef NUCLEO
// Method to store Grbl global settings struct and version number into EFLASH
// NOTE: This function can only be called in IDLE state.
void write_global_settings()
{
  uint32_t status = flash_verify_erase_need((char *) EFLASH_MAIN_BASE_ADDRESS, (char*)&settings, ((unsigned int)sizeof(settings_t)));

  if (status == 0)
  {
     //write directly into main-sector
     flash_put_char(EFLASH_MAIN_BASE_ADDRESS, SETTINGS_VERSION);
     memcpy_to_flash_with_checksum(EFLASH_ADDR_GLOBAL_MAIN, (char*)&settings, sizeof(settings_t));
  }
  else
  {
     //clean copy sector with a delete
     delete_copy_sector();
     //write into copy-sector the global settings
     flash_put_char(EFLASH_COPY_BASE_ADDRESS, SETTINGS_VERSION);
     memcpy_to_flash_with_checksum(EFLASH_ADDR_GLOBAL_COPY, (char*)&settings, sizeof(settings_t));

     //copy into copy-sector the rest of the main sector relevant parts
     copy_from_main_to_copy(((uint32_t)EFLASH_ADDR_PARAMETERS_OFFSET), ((uint32_t)EFLASH_ERASE_AND_RESTORE_OFFSET));
     //update status since main sector has been copied
     update_main_sector_status(MAIN_SECTOR_COPIED);

     restore_default_sector_status();
  }
}


// Method to store coord data parameters into EFLASH
void settings_write_coord_data(uint8_t coord_select, float *coord_data)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize();
  #endif
  uint32_t addr = coord_select*(sizeof(float)*(N_AXIS+1)) + EFLASH_ADDR_PARAMETERS_MAIN;
  uint32_t addr_copy = coord_select*(sizeof(float)*(N_AXIS+1)) + EFLASH_ADDR_PARAMETERS_COPY;
  uint32_t status = flash_verify_erase_need((char *) addr, (char*)coord_data, ((unsigned int)sizeof(float)*N_AXIS));

  if (status == 0)
  {
	  memcpy_to_flash_with_checksum(addr,(char*)coord_data, sizeof(float)*N_AXIS);
  }
  else
  {
	  //clean copy sector with a delete
	  delete_copy_sector();
	  //write into copy-sector the global settings
	  memcpy_to_flash_with_checksum(addr_copy, (char*)coord_data, sizeof(float)*N_AXIS);

	  //copy into copy-sector the rest of the main sector relevant parts
	  copy_from_main_to_copy(((uint32_t)0), EFLASH_ADDR_PARAMETERS_OFFSET + coord_select*(sizeof(float)*(N_AXIS + 1)));
	  copy_from_main_to_copy(EFLASH_ADDR_PARAMETERS_OFFSET + ((uint32_t)(coord_select+1)*sizeof(float)*(N_AXIS + 1)), ((uint32_t)EFLASH_ERASE_AND_RESTORE_OFFSET));

	  //update status since main sector has been copied
      update_main_sector_status(MAIN_SECTOR_COPIED);

	  restore_default_sector_status();
  }
}

// Method to store startup lines into EEPROM/FLASH
void settings_store_startup_line(uint8_t n, char *line)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize(); // A startup line may contain a motion and be executing. 
  #endif
  uint32_t addr = n*(LINE_BUFFER_SIZE+4)+EFLASH_ADDR_STARTUP_BLOCK_MAIN;
  uint32_t addr_copy = n*(LINE_BUFFER_SIZE+4) + EFLASH_ADDR_STARTUP_BLOCK_COPY;
  uint32_t status = flash_verify_erase_need((char *) addr, (char*)line, ((unsigned int)LINE_BUFFER_SIZE));

  if (status == 0)
  {
	  memcpy_to_flash_with_checksum(addr,(char*)line, LINE_BUFFER_SIZE);
  }
  else
  {
	  //clean copy sector with a delete
	  delete_copy_sector();
	  //write into copy-sector the global settings
	  memcpy_to_flash_with_checksum(addr_copy, (char*)line, LINE_BUFFER_SIZE);

	  //copy into copy-sector the rest of the main sector relevant parts
	  copy_from_main_to_copy(((uint32_t)0), EFLASH_ADDR_STARTUP_BLOCK_OFFSET + n*(LINE_BUFFER_SIZE+4) - 1);
	  copy_from_main_to_copy(((uint32_t)n*(LINE_BUFFER_SIZE+4)+LINE_BUFFER_SIZE)+EFLASH_ADDR_STARTUP_BLOCK_OFFSET, ((uint32_t)EFLASH_ERASE_AND_RESTORE_OFFSET));

	  //update status since main sector has been copied
      update_main_sector_status(MAIN_SECTOR_COPIED);

	  restore_default_sector_status();
  }
}

// Method to store build info into EEPROM
void settings_store_build_info(char *line)
{
  // Build info can only be stored when state is IDLE.
  uint32_t addr = EFLASH_ADDR_BUILD_INFO_MAIN;
  uint32_t addr_copy = EFLASH_ADDR_BUILD_INFO_COPY;
  uint32_t status = flash_verify_erase_need((char *) addr, (char*)line, ((unsigned int)LINE_BUFFER_SIZE));

  if (status == 0)
  {
	  memcpy_to_flash_with_checksum(addr,(char*)line, LINE_BUFFER_SIZE);
  }
  else
  {
	  //clean copy sector with a delete
	  delete_copy_sector();
	  //write into copy-sector the global settings
	  memcpy_to_flash_with_checksum(addr_copy, (char*)line, LINE_BUFFER_SIZE);

	  //copy into copy-sector the rest of the main sector relevant parts
	  copy_from_main_to_copy(((uint32_t)0), addr);

	  //update status since main sector has been copied
      update_main_sector_status(MAIN_SECTOR_COPIED);

	  restore_default_sector_status();
  }
}

#else

// Method to store startup lines into EEPROM
void settings_store_startup_line(uint8_t n, char *line)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize(); // A startup line may contain a motion and be executing. 
  #endif
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  memcpy_to_eeprom_with_checksum(addr,(char*)line, LINE_BUFFER_SIZE);
}


// Method to store build info into EEPROM
// NOTE: This function can only be called in IDLE state.
void settings_store_build_info(char *line)
{
  // Build info can only be stored when state is IDLE.
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_BUILD_INFO,(char*)line, LINE_BUFFER_SIZE);
}

// Method to store coord data parameters into EEPROM
void settings_write_coord_data(uint8_t coord_select, float *coord_data)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize();
  #endif
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS;
  memcpy_to_eeprom_with_checksum(addr,(char*)coord_data, sizeof(float)*N_AXIS);
}
// Method to store Grbl global settings struct and version number into EEPROM
// NOTE: This function can only be called in IDLE state.
void write_global_settings()
{
  eeprom_put_char(0, SETTINGS_VERSION);
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_GLOBAL, (char*)&settings, sizeof(settings_t));
}


#endif


// Method to restore EEPROM-saved Grbl global settings back to defaults. 
void settings_restore(uint8_t restore_flag) {  
  if (restore_flag & SETTINGS_RESTORE_DEFAULTS) {
    settings = defaults;
    write_global_settings();
  }


  if (restore_flag & SETTINGS_RESTORE_PARAMETERS) {
    uint8_t idx;
    float coord_data[N_AXIS] = {0.0};
#ifndef NUCLEO //Actually memset seems unuseful, init to zero in the previous instruction
    memset(&coord_data, 0, sizeof(coord_data));
#endif
    for (idx=0; idx <= SETTING_INDEX_NCOORD; idx++) { settings_write_coord_data(idx, coord_data); }
  }
  if (restore_flag & SETTINGS_RESTORE_STARTUP_LINES) {
#ifdef NUCLEO //Checksum not implemented for startup lines
	#if N_STARTUP_LINE > 0
    flash_put_char(EFLASH_ADDR_STARTUP_BLOCK_MAIN, 0);
    #endif
    #if N_STARTUP_LINE > 1
    flash_put_char(EFLASH_ADDR_STARTUP_BLOCK_MAIN+(LINE_BUFFER_SIZE+4), 0);
    #endif
#else
	#if N_STARTUP_LINE > 0
    eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK, 0);
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+1, 0); // Checksum
    #endif
    #if N_STARTUP_LINE > 1
    eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+1), 0);
      eeprom_put_char(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+2), 0); // Checksum
    #endif
#endif
  }
  
  if (restore_flag & SETTINGS_RESTORE_BUILD_INFO)
  {
#ifdef NUCLEO //Checksum not implemented for build info
	  flash_put_char(EFLASH_ADDR_BUILD_INFO_MAIN , 0);
#else
	  eeprom_put_char(EEPROM_ADDR_BUILD_INFO , 0);
    eeprom_put_char(EEPROM_ADDR_BUILD_INFO+1 , 0); // Checksum
#endif
  }
}

#ifdef NUCLEO
// Reads startup line from FLASH. Updated pointed line string data.
uint8_t settings_read_startup_line(uint8_t n, char *line)
{
  uint32_t addr = n*(LINE_BUFFER_SIZE+4)+EFLASH_ADDR_STARTUP_BLOCK_MAIN;
  if (!(memcpy_from_flash_with_checksum((char*)line, addr, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_startup_line(n, line);
    return(false);
  }
  return(true);
}


// Reads startup line from FLASH. Updated pointed line string data.
uint8_t settings_read_build_info(char *line)
{
  if (!(memcpy_from_flash_with_checksum((char*)line, EFLASH_ADDR_BUILD_INFO_MAIN, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_build_info(line);
    return(false);
  }
  return(true);
}


// Read selected coordinate data from FLASH. Updates pointed coord_data value.
uint8_t settings_read_coord_data(uint8_t coord_select, float *coord_data)
{
  uint32_t addr = coord_select*(sizeof(float)*(N_AXIS+1)) + EFLASH_ADDR_PARAMETERS_MAIN;
  if (!(memcpy_from_flash_with_checksum((char*)coord_data, addr, sizeof(float)*N_AXIS))) {
    // Reset with default zero vector
    clear_vector_float(coord_data);
    settings_write_coord_data(coord_select,coord_data);
    return(false);
  }
  return(true);
}


// Reads Grbl global settings struct from FLASH.
uint8_t read_global_settings(void) {
  // Check version-byte of flash
  uint8_t version = flash_get_char(EFLASH_ADDR_VERSION_MAIN);
  if (version == SETTINGS_VERSION) {
    // Read settings-record and check checksum
    if (!(memcpy_from_flash_with_checksum((char*)&settings, EFLASH_ADDR_GLOBAL_MAIN, sizeof(settings_t)))) {
      return(false);
    }
  } else {
    return(false);
  }
  return(true);
}



#else
// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_startup_line(uint8_t n, char *line)
{
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  if (!(memcpy_from_eeprom_with_checksum((char*)line, addr, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_startup_line(n, line);
    return(false);
  }
  return(true);
}

// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_build_info(char *line)
{
  if (!(memcpy_from_eeprom_with_checksum((char*)line, EEPROM_ADDR_BUILD_INFO, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_build_info(line);
    return(false);
  }
  return(true);
}


// Read selected coordinate data from EEPROM. Updates pointed coord_data value.
uint8_t settings_read_coord_data(uint8_t coord_select, float *coord_data)
{
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS;
  if (!(memcpy_from_eeprom_with_checksum((char*)coord_data, addr, sizeof(float)*N_AXIS))) {
    // Reset with default zero vector
    clear_vector_float(coord_data); 
    settings_write_coord_data(coord_select,coord_data);
    return(false);
  }
  return(true);
}  


// Reads Grbl global settings struct from EEPROM.
uint8_t read_global_settings(void) {
  // Check version-byte of eeprom
  uint8_t version = eeprom_get_char(0);
  if (version == SETTINGS_VERSION) {
    // Read settings-record and check checksum
    if (!(memcpy_from_eeprom_with_checksum((char*)&settings, EEPROM_ADDR_GLOBAL, sizeof(settings_t)))) {
      return(false);
    }
  } else {
    return(false); 
  }
  return(true);
}
#endif //ifdef NUCLEO


// A helper method to set settings from command line
uint8_t settings_store_global_setting(uint8_t parameter, float value) {
  if (value < 0.0) { return(STATUS_NEGATIVE_VALUE); } 
  if (parameter >= AXIS_SETTINGS_START_VAL) {
    // Store axis configuration. Axis numbering sequence set by AXIS_SETTING defines.
    // NOTE: Ensure the setting index corresponds to the report.c settings printout.
    parameter -= AXIS_SETTINGS_START_VAL;
    uint8_t set_idx = 0;
    while (set_idx < AXIS_N_SETTINGS) {
      if (parameter < N_AXIS) {
        // Valid axis setting found.
        switch (set_idx) {
          case 0:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.max_rate[parameter] > (MAX_STEP_RATE_HZ*60.0)) { return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.steps_per_mm[parameter] = value;
            break;
          case 1:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.steps_per_mm[parameter] > (MAX_STEP_RATE_HZ*60.0)) {  return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.max_rate[parameter] = value;
            break;
          case 2: settings.acceleration[parameter] = value*60*60; break; // Convert to mm/min^2 for grbl internal use.
          case 3: settings.max_travel[parameter] = -value; break;  // Store as negative for grbl internal use.
        }
        break; // Exit while-loop after setting has been configured and proceed to the EEPROM write call.
      } else {
        set_idx++;
        // If axis index greater than N_AXIS or setting index greater than number of axis settings, error out.
        if ((parameter < AXIS_SETTINGS_INCREMENT) || (set_idx == AXIS_N_SETTINGS)) { return(STATUS_INVALID_STATEMENT); }
        parameter -= AXIS_SETTINGS_INCREMENT;
      }
    }
  } else {
    // Store non-axis Grbl settings
    uint8_t int_value = trunc(value);
    uint16_t int_value16 = trunc(value);
    uint32_t int_value32 = trunc(value);
    switch(parameter) {
      case 0: 
        if (int_value < 3) { return(STATUS_SETTING_STEP_PULSE_MIN); }
        settings.pulse_microseconds = int_value; break;
      case 1: settings.stepper_idle_lock_time = int_value; break;
      case 2: 
        settings.step_invert_mask = int_value; 
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 3: 
        settings.dir_invert_mask = int_value; 
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 4: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_ST_ENABLE; }
        else { settings.flags &= ~BITFLAG_INVERT_ST_ENABLE; }
        break;
      case 5: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_LIMIT_PINS; }
        else { settings.flags &= ~BITFLAG_INVERT_LIMIT_PINS; }
        break;
      case 6: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_PROBE_PIN; }
        else { settings.flags &= ~BITFLAG_INVERT_PROBE_PIN; }
        probe_configure_invert_mask(false);
        break;
      case 10: settings.status_report_mask = int_value; break;
      case 11: settings.junction_deviation = value; break;
      case 12: settings.arc_tolerance = value; break;
      case 13:
        if (int_value) { settings.flags |= BITFLAG_REPORT_INCHES; }
        else { settings.flags &= ~BITFLAG_REPORT_INCHES; }
        system_flag_wco_change(); // Make sure WCO is immediately updated.
        break;
      case 20:
        if (int_value) { 
          if (bit_isfalse(settings.flags, BITFLAG_HOMING_ENABLE)) { return(STATUS_SOFT_LIMIT_ERROR); }
          settings.flags |= BITFLAG_SOFT_LIMIT_ENABLE; 
        } else { settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; }
        break;
      case 21:
        if (int_value) { settings.flags |= BITFLAG_HARD_LIMIT_ENABLE; }
        else { settings.flags &= ~BITFLAG_HARD_LIMIT_ENABLE; }
        limits_init(); // Re-init to immediately change. NOTE: Nice to have but could be problematic later.
        break;
      case 22:
        if (int_value) { settings.flags |= BITFLAG_HOMING_ENABLE; }
        else { 
          settings.flags &= ~BITFLAG_HOMING_ENABLE; 
          settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; // Force disable soft-limits.
        }
        break;
      case 23: settings.homing_dir_mask = int_value; break;
      case 24: settings.homing_feed_rate = value; break;
      case 25: settings.homing_seek_rate = value; break;
      case 26: settings.homing_debounce_delay = int_value16; break;
      case 27: settings.homing_pulloff = value; break;
      case 30: settings.rpm_max = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 31: settings.rpm_min = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 32:
        #ifdef VARIABLE_SPINDLE
          if (int_value) { settings.flags |= BITFLAG_LASER_MODE; }
          else { settings.flags &= ~BITFLAG_LASER_MODE; }
        #else
          return(STATUS_SETTING_DISABLED_LASER);
        #endif
        break;
      case 40: settings.spindle_pwm_period = int_value32; break;
      case 41: settings.spindle_pwm_max_time_on = int_value32; break;
      case 42: settings.spindle_pwm_min_time_on = int_value32; break;
      case 43:
		  if (int_value)
		  {
			  settings.spindle_pwm_enable_at_start |= BITFLAG_PWM_EN_AT_START;
		  }
		  else
		  {
			  settings.spindle_pwm_enable_at_start &= ~BITFLAG_PWM_EN_AT_START;
		  }
		  break;
      case 44:
		  if (int_value32 <= MAX_RAMPING_DIVISIONS)
		  {
			  settings.spindle_pwm_ramping_divisions = int_value32;
		  }
		  else
		  {
			  settings.spindle_pwm_ramping_divisions = DEFAULT_PWM_RAMPING_DIVS;
		  }
		  break;
      case 95:
    	  if (int_value)
    	  {
    		  settings.homing_debug |= BITFLAG_HOMING_DEBUG;
    	  }
    	  else
    	  {
    		  settings.homing_debug &= ~BITFLAG_HOMING_DEBUG;
    	  }
    	  break;
      default: 
        return(STATUS_INVALID_STATEMENT);
    }
  }
  write_global_settings();
  return(STATUS_OK);
}


// Initialize the config subsystem
void settings_init(void) {
  restore_default_sector_status();
  if(!read_global_settings()) {
    report_status_message(STATUS_SETTING_READ_FAIL);
    settings_restore(SETTINGS_RESTORE_ALL); // Force restore all EEPROM data.
    report_grbl_settings();
  }
}


// Returns step pin mask according to Grbl internal axis indexing.
uint16_t get_step_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == X_AXIS ) { return((1<<X_STEP_BIT)); }
  if ( axis_idx == Y_AXIS ) { return((1<<Y_STEP_BIT)); }
  return((1<<Z_STEP_BIT));
}


// Returns direction pin mask according to Grbl internal axis indexing.
uint16_t get_direction_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == X_AXIS ) { return((1<<X_DIRECTION_BIT)); }
  if ( axis_idx == Y_AXIS ) { return((1<<Y_DIRECTION_BIT)); }
  return((1<<Z_DIRECTION_BIT));
}


// Returns limit pin mask according to Grbl internal axis indexing.
uint16_t get_limit_pin_mask(uint8_t axis_idx)
{
  if ( axis_idx == X_AXIS ) { return((1<<X_LIMIT_BIT)); }
  if ( axis_idx == Y_AXIS ) { return((1<<Y_LIMIT_BIT)); }
  return((1<<Z_LIMIT_BIT));
}
