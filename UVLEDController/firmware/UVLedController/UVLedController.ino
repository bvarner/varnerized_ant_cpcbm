#include <AceButton.h>
#include <LcdMenu.h>
#include <AsyncTimer.h>

#define LCD_ROWS 2
#define LCD_COLS 16

const int BTN_DN = 2; // D2
const int BTN_UP = 3; // D3
const int BTN_SEL = 4; // D4

const int LED_OUT = 9;

// Forward Declarations
void handleEvent(ace_button::AceButton*, uint8_t, uint8_t);
void enterExposure();
void enterIntensity();
void startExposure();
void stopExposure();



int exposure = 10; // TODO: Load from EEPROM.
int intensity = 255; // TODO: Load from EEPROM.

AsyncTimer timer;

extern MenuItem mainMenu[];

MenuItem mainMenu[] = {ItemHeader(),
                       ItemCommand("Exposure Time", enterExposure),
                       ItemCommand("Intensity", enterIntensity),
                       ItemCommand("Start", startExposure),
                       ItemFooter()};
LcdMenu menu(LCD_ROWS, LCD_COLS);

ace_button::AceButton btnDown(BTN_DN);
ace_button::AceButton btnUp(BTN_UP);
ace_button::AceButton btnSelect(BTN_SEL);


// valid modes 0 (menu), 1 (set exposure), 2 (set intensity), 3 (do exposing)
int mode = 0;

void setup() {
  pinMode(BTN_DN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(LED_OUT, OUTPUT);
  analogWrite(LED_OUT, 0);

  ace_button::ButtonConfig * config = btnUp.getButtonConfig();
  config->setEventHandler(handleEvent);
  config->setFeature(ace_button::ButtonConfig::kFeatureRepeatPress);

  config = btnDown.getButtonConfig();
  config->setEventHandler(handleEvent);
  config->setFeature(ace_button::ButtonConfig::kFeatureRepeatPress);

  config = btnSelect.getButtonConfig();
  config->setEventHandler(handleEvent);
  config->setFeature(ace_button::ButtonConfig::kFeatureClick);

  menu.setupLcdWithMenu(0x27, mainMenu);
}

void loop() {
  // Poll for input.
  btnDown.check();
  btnUp.check();
  btnSelect.check();
  timer.handle();
}

void enterExposure() {
  menu.hide();
  mode = 1;
  // Update the LCD:
  menu.lcd->clear();
  menu.lcd->setCursor(0, 0);
  menu.lcd->print("Exposure:");
  menu.lcd->setCursor(4, 1);
  menu.lcd->print(exposure);
}

void enterIntensity() {
  menu.hide();
  mode = 2;

  // Update the LCD:
  menu.lcd->clear();
  menu.lcd->setCursor(0, 0);
  menu.lcd->print("Intensity:");
  menu.lcd->setCursor(4, 1);
  menu.lcd->print(intensity);
}

void startExposure() {
  menu.hide();
  mode = 3;

  menu.lcd->setCursor(4, 0);
  menu.lcd->print("Exposing");

  analogWrite(LED_OUT, intensity);  
  timer.setTimeout(stopExposure, exposure * 1000);
}

void stopExposure() {
  analogWrite(LED_OUT, 0);
  mode = 0;
  menu.show();
}

void handleEvent(ace_button::AceButton* source, uint8_t eventType, uint8_t state) {
  if (eventType == ace_button::AceButton::kEventPressed && mode == 3) {
      stopExposure();
      timer.cancelAll();
      return;
  }

  
  if (source == &btnUp) {
    switch (eventType) {
      case ace_button::AceButton::kEventPressed:
      case ace_button::AceButton::kEventRepeatPressed:
        if (mode == 0) {
          menu.up();
        } else if (mode == 1) {
          exposure += 1;
          enterExposure();
        } else if (mode == 2 && intensity < 255) {
          intensity += 1;
          enterIntensity();
        }
        break;
    }
  } else if (source == &btnDown) {
    switch (eventType) {
      case ace_button::AceButton::kEventPressed:
      case ace_button::AceButton::kEventRepeatPressed:
        if (mode == 0) {
          menu.down();
        } else if (mode == 1 && exposure > 1) {
          exposure -= 1;
          enterExposure();
        } else if (mode == 2 && intensity > 1) {
          intensity -= 1;
          enterIntensity();
        }
        break;
    }
  } else if (source == &btnSelect) {
    switch (eventType) {
      case ace_button::AceButton::kEventClicked:
        if (mode == 0) {
          menu.enter();
        } else if (mode == 1 || mode == 2) {
          mode = 0;
          menu.show();
        }
        break;
    }
  }
}
