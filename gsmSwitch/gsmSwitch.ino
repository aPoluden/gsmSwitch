#include <Timer.h>

#include <Regexp.h>
#include <FiniteStateMachine.h>
#include <SoftwareSerial.h>
#include <SDConfigFile.h>
#include <SPI.h>
#include <SD.h>

void turnOff();
void turnOn();
void checkMsg();

// Setup params
byte TIMER;
byte SWITCH_PIN;
String PHONE_NUMBERS[2];

// SD
const char CONFIG_FILE_NAME[] = "config.txt";
const byte CS = 10;
const byte CONFIG_LINE_LENGTH = 127;

long int minute = 60000;

// commands
enum OPTIONS {eOFF, eON};
String ON = String("on");
String OFF = String("off");

// FSM
State on = State(turnOn, checkMsg, turnOff);
State off = State(turnOff, checkMsg, turnOn);
FSM fsm = FSM(off);

Timer t;
SDConfigFile cfg;

// UART: SIM900 <--> ARDUINO
SoftwareSerial SIM900(7, 8);

struct SMS {
   String phone_number;
   String body;
};

void setup() {
  if (true) {
    Serial.begin(57600);
    while (!Serial) {}
  }  
  if (setupSD())
    setupSysParams(); // TODO
  pinMode(SWITCH_PIN, OUTPUT);
  SIM900.begin(19200);
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  fsm.update();
}

void loop() {
  fsm.update();
  t.update();
}

void turnOn() {
  digitalWrite(SWITCH_PIN, HIGH);
  t.after(TIMER * minute, turnOff);
}

void turnOff() {
  if (fsm.isInState(on))
    fsm.transitionTo(off);
  digitalWrite(SWITCH_PIN, LOW);
  // TODO Turn off timer
}

void checkMsg() {
  String sim900_output = check_sim900_output();
  if (sim900_output.length() != 0) {
    if (check_if_sms(sim900_output)) {
        struct SMS sms = parse_sms(sim900_output);
        if (authPhoneNumber(sms.phone_number)) {
          // links string to int
          int option = optionLinker(sms.body);
          switchState(option);
        }
    } else {
      // SIM900 tech. info
    }
  }
}

void switchState(int opt) {
  switch(opt) {
    case eON:
      if (fsm.isInState(off))
        fsm.transitionTo(on);
      break;
    case eOFF:
      if (fsm.isInState(on))
        fsm.transitionTo(off);
      break;
    default:
      break;
  } 
}

int optionLinker(String str) {
  // poor linker logic
  if (ON.equals(str)) {
    return eON;
  } else if(str == OFF) {
    return eOFF;
  } else {
    return -1; 
  }
}

// Validate senders phone number
boolean authPhoneNumber(String phone) {
  for (int i = 0; i <= sizeof(PHONE_NUMBERS); i++) {
    if (phone.equals(PHONE_NUMBERS[i])) {
        return true;
    }
  }
  return false;
}
// SMS parser
SMS parse_sms(String sms) {
    String phone = "";
    String msg = "";
    
    int phone_index_start = 9;
    int phone_index_finish = 21;
    int sms_index_start = 48;
    
    phone = sms.substring(phone_index_start, phone_index_finish);
    msg = sms.substring(sms_index_start);
    msg.replace(" ", "");
    msg.replace("\r\n", "");
    // Define and create struct obj
    struct SMS smsstrct;
    smsstrct.phone_number = phone;
    smsstrct.body = String(msg);
    return smsstrct;
}

// Check if SIM900 output is SMS message
boolean check_if_sms(String sim_output) {
  String sms_head = "+CMT";
  if (sim_output.indexOf(sms_head) > -1) {
    return true;
  } else {
    return false;
  }
}

// Check SIM900 serial bytes
String check_sim900_output() {
   String sim_output = "";
   while (SIM900.available()) {
    sim_output += (char)SIM900.read();
   }
   if (sim_output != "") {
    Serial.print(sim_output);
   }
   return sim_output;
}

// Check if serial available, DEBUG only
String check_serial() {
  String serial_request = "";
  // Check if available bytes on serial
  while (Serial.available())  {
    // get byte from serial:
    char inChar = Serial.read();
    if (inChar == '\n') {
      return serial_request;
    } else {
      serial_request += inChar;
    }
  }
  return serial_request;
}

// Setup SD card
boolean setupSD() {
  pinMode(CS, OUTPUT);
  if (!SD.begin(CS)) {
    Serial.println("sd open fail");
    return false;
  }
  // Open the configuration file.
  if (!cfg.begin(CONFIG_FILE_NAME, CONFIG_LINE_LENGTH)) {
    Serial.print("sd read fail");
    return false;
  }
  return true;
}

// Setup system params from SD card
void setupSysParams() {
  // setup default settings 
  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    if (cfg.nameIs("TIMER")) {
      // get TIMER config
      int timer = cfg.getIntValue();
      TIMER = timer;
    } else if (cfg.nameIs("SWITCH_PIN")) {
      int pin = cfg.getIntValue();
      SWITCH_PIN = pin;
    } else if (cfg.nameIs("PHONE_NUMBERS")) {
      char *phones = cfg.copyValue();
      setPhoneNumbers(phones);
    } else {}
  }
  // clean up
  cfg.end();
}

void setPhoneNumbers(char *phonesArr) {
  // sizeof(arr) returns number of bytes that arr occupies in memory
  int sizeOfArr = sizeof(PHONE_NUMBERS) / sizeof(String);
  char *p = phonesArr;
  char *str;
  byte index = 0;
  while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
   if (validatePhoneNumber(str)) {
    PHONE_NUMBERS[index] = str;
    index++;
    if (index >= sizeOfArr) {
      break;  
    }
   }
}

void setupDefaultSysParams() {
   // TODO implement setup default settings
}

boolean validatePhoneNumber(char *phone) {
  // TODO implement phone number valildation
  return true;
}

// Sends commands to SIM900 module
void send_command_to_sim900(String cmd) {
    SIM900.print(cmd);
}
