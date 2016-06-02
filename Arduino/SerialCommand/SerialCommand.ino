
#include <SerialCommand.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <Timer.h>

#define arduinoLED 13   // Arduino LED on board

SoftwareSerial softSerial = SoftwareSerial(10, 11);
SerialCommand sCmd(softSerial);
bool checkPing = false;
bool pingReceived = false;
Timer t;

void setup() {
  pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
  digitalWrite(arduinoLED, LOW);    // default to LED off

  softSerial.begin(9600);

  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("C",   connectTo);
  sCmd.addCommand("D",   disconnectTo);
  sCmd.addCommand("P",   changePassword);
  sCmd.addCommand("I",  pingOn);
  sCmd.addCommand("N", pingOff);
  sCmd.addDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
  softSerial.println("Ready");
  t.every(1000, pingCheck);
}

void pingCheck() {
  if (!checkPing) return;
  softSerial.println("checkping");
  if (pingReceived) {
    pingReceived = false;
    return;
  } else {
    disconnectTo();
  }
}

void pingOn() {
  checkPing = true;
  pingReceived = true;
  softSerial.println("ping");
}

void pingOff() {
  checkPing = false;
  pingReceived = false;
  softSerial.println("pingOff");
}

void loop() {
  sCmd.readSerial();     // We don't do much, just process serial command
  t.update();
}

void changePassword() {
  char *arg;
  arg = sCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL) {    // As long as it existed, take it
    for (int i = 0; i < 8; i++) {
      EEPROM.write(i, arg[i]);
    }

    EEPROM.write(8, 0XEE);
  }
  else {
    EEPROM.write(8, 0);
  }
}

void disconnectTo() {
  digitalWrite(arduinoLED, LOW);
}

void connectTo() {
  char *arg;
  arg = sCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL) {    // As long as it existed, take it
    char password[9] = "";
    for (int i = 0; i < 8; i++) {
      password[i] = EEPROM.read(i);
    }

    password[8] = '\0';

    if (strlen(arg) > 8) {
      arg[8] = '\0';
    }

    unsigned char set = EEPROM.read(8);

    softSerial.println(password);
    softSerial.println(set, HEX);

    if (set == 0XEE) {
      if (strcmp(arg, password) == 0) {
        softSerial.println("C");
        digitalWrite(arduinoLED, HIGH);
      }
    } else {
      if (strcmp(arg, "12345678") == 0) {
        softSerial.println("C");
        digitalWrite(arduinoLED, HIGH);
      }
    }
  }
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized() {
  softSerial.println("What?");
}
