#include <Regexp.h>
#include <FiniteStateMachine.h>
#include <SoftwareSerial.h>

void turnOff();
void turnOn();
void checkMsg();

// Setup
byte TIMER;
byte SWITCH_PIN = 12;
String PHONE_NUMBERS[] = {"+37067309726"};
enum OPTIONS {eOFF, eON};
String ON = String("on");
String OFF = String("off");

//byte ON;
//byte OFF;

// TODO implement 
struct SETUP {
  byte TIMER;
  byte PIN;
  String PHONE_NUMBERS[] = {};
  String ON;
  String OFF;  
};
// FSM
State on = State(turnOn, checkMsg, turnOff);
State off = State(turnOff, checkMsg, turnOn);
FSM fsm = FSM(off);
// SIM900 serial
SoftwareSerial SIM900(7, 8);

// Globals
struct SMS {
   String phone_number;
   String body;
};

boolean DEBUG = true;

void setup() {
  if (DEBUG) {
    Serial.begin(57600);
    while (!Serial) {}
  }
  SIM900.begin(19200);
  // Turn on message receiving
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  fsm.update();
}

void loop() {
  fsm.update();
  // Timer
  if (DEBUG) {
    delay(2000);
    // Interact with sim, DEBUG
    String serial_input = check_serial();
    if (serial_input != "") {
      send_command_to_sim900(serial_input);
    }
  }
}

void turnOn() {
  if (DEBUG) {
    Serial.println("turnOn");
  }
  digitalWrite(SWITCH_PIN, HIGH);
  // TODO Turn on timer
}

void turnOff() {
  if (DEBUG) {
    Serial.println("turnOff");
  }
  digitalWrite(SWITCH_PIN, LOW);
  // TODO Turn off timer
}

void checkMsg() {
  Serial.println("checkMsg");
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
  if (DEBUG) {
    char buffer[128];
    sprintf(buffer, "switchState: opt %d", opt);
    Serial.println(buffer);
  }
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
      if (DEBUG)
        Serial.println("switchState: wrong option");
      break;
  } 
}

int optionLinker(String str) {
  if (DEBUG) {
    Serial.println("optionLinker: str=" + str);
  }
  // poor linker logic
  if (ON.equals(str)) {
    if (DEBUG)
      Serial.println("optionLinker: ON");
    return eON;
  } else if(str == OFF) {
    if (DEBUG)
      Serial.println("optionLinker: OFF");
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
    
    // Serial.println(phone);
    // Serial.println(msg);

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
   if (sim_output != "" && DEBUG) {
    Serial.print(sim_output);
   }
   return sim_output;
}

// Check if serial available, DEBUG
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

// Sends commands to SIM900 module
void send_command_to_sim900(String cmd) {
    SIM900.print(cmd);
}
