#include <SDConfigFile.h>
#include <SPI.h>
#include <SD.h>

const char CONFIG_FILE_NAME[] = "config.txt";
boolean DEBUG = true;
const int CS = 10;
char *numbers;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  pinMode(CS, OUTPUT);
  SDConfigFile cfg;
  const uint8_t CONFIG_LINE_LENGTH = 127; 

  Serial.println("Calling SD.begin()...");
  if (!SD.begin(CS)) {
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  card formatting.");
    return;
  }
  Serial.println("...succeeded.");
  // Open the configuration file.
  if (!cfg.begin(CONFIG_FILE_NAME, CONFIG_LINE_LENGTH)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(CONFIG_FILE_NAME);
    return;
  }

  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    if (cfg.nameIs("TIMER")) {
      // get TIMER config
      int timer = cfg.getIntValue();
      if (DEBUG) {
        Serial.print("Timer: ");
        Serial.println(timer);
      }
    } else if (cfg.nameIs("SWITCH_PIN")) {
      int pin = cfg.getIntValue();
      if (DEBUG) {
        Serial.print("Switch pin: ");
        Serial.println(pin);
      }
    } else if (cfg.nameIs("PHONE_NUMBERS")) {
      numbers = cfg.copyValue();
      if (DEBUG) {
        Serial.print("Phone numbers: ");
        Serial.println(numbers);
      }
    } else {
      if (DEBUG) {
        Serial.print("Unknown name in config: ");
        Serial.println(cfg.getName());
        // TODO implement phone number parser
      }
    }
  }
  // clean up
  cfg.end();
}


void loop(void) {

}

