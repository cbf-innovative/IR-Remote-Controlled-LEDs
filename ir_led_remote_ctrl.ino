#include <IRremote.hpp>

#define IR_RCVR_IN 2

// Define pins for the 4 leds
const uint8_t led_pins[] = { 10, 11, 12, 13 };

// Define debounce period (milliseconds)
#define DEBOUNCE_DELAY 300

struct IRMapping {
  uint8_t protocol;
  uint16_t address;  // Remote address (ID)
  uint32_t code;     // Button code from the remote
  uint8_t pin;
};

const IRMapping remoteMappings[] = {
  // NEC remote - 1
  // NOTE : Replace with your own remote codes !!!!
  // See code on your serial monitor
  { NEC, 0x80, 0x12, 0xFF },
  { NEC, 0x80, 0x07, led_pins[0] },  //  led 1
  { NEC, 0x80, 0x0A, led_pins[1] },  //  led 2
  { NEC, 0x80, 0x1B, led_pins[2] },  //  led 3
  { NEC, 0x80, 0x1F, led_pins[3] },  //  led 4

  // TV Remote
  { NEC, 0x04, 0x08, 0xFF },
  { NEC, 0x04, 0x56, led_pins[0] },  //  led 1
  { NEC, 0x04, 0x5C, led_pins[1] },  //  led 2
  { NEC, 0x04, 0xF7, led_pins[2] },  //  led 3
  { NEC, 0x04, 0x13, led_pins[3] },  //  led 4
};

const uint8_t numMappings = sizeof(remoteMappings) / sizeof(remoteMappings[0]);

// Track led states
bool LEDState[] = { true, true, true, true };

// Last time a button was pressed (for debounce)
unsigned long lastPressTime = 0;

void lookupEvent(uint8_t protocol, uint16_t address, uint32_t code, uint8_t *pin) {
  for (uint8_t i = 0; i < numMappings; i++) {
    if (remoteMappings[i].protocol == protocol && remoteMappings[i].address == address && remoteMappings[i].code == code) {
      *pin = remoteMappings[i].pin;
      return;
    }
  }
  *pin = 0xFF;  // No match found
}

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RCVR_IN);

  // Initialize led pins
  // Make sure all leds are initially OFF
  for (int i = 0; i < 4; i++) {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], LEDState[i] ? HIGH : LOW);
  }
}

void loop() {
  if (IrReceiver.decode()) {
    unsigned long currentMillis = millis();

    // Check if enough time has passed for debounce
    if (currentMillis - lastPressTime >= DEBOUNCE_DELAY) {
      uint8_t protocol = IrReceiver.decodedIRData.protocol;
      uint16_t address = IrReceiver.decodedIRData.address;
      uint32_t command = IrReceiver.decodedIRData.command;

      // Print details of the received signal
      Serial.print("Received signal - Protocol: ");
      if (protocol == NEC) {
        Serial.print("NEC");
      } else if (protocol == RC5) {
        Serial.print("RC5");
      } else if (protocol == RC6) {
        Serial.print("RC6");
      }
      Serial.print(", Address: 0x");
      Serial.print(address, HEX);
      Serial.print(", Command: 0x");
      Serial.println(command, HEX);

      // Look up the event for this protocol, address, and command
      uint8_t pin = 0;
      lookupEvent(protocol, address, command, &pin);

      if (pin != 0xFF) {
        // Toggle the led state
        int ledIndex = -1;
        for (int i = 0; i < 4; i++) {
          if (led_pins[i] == pin) {
            ledIndex = i;
            break;
          }
        }

        // If a valid led pin is found, toggle its state
        if (ledIndex != -1) {
          LEDState[ledIndex] = !LEDState[ledIndex];            // Toggle the state
          digitalWrite(pin, LEDState[ledIndex] ? HIGH : LOW);  // Set led to ON/OFF
          Serial.println("Toggled led Pin: " + String(pin) + " State: " + (LEDState[ledIndex] ? "ON" : "OFF"));
        }
      } else
      {
        for (int i = 0; i < 4; i++) {
          digitalWrite(led_pins[i], LOW);
          LEDState[i] = true;
        }
      }
    }
    // Update the last press time
    lastPressTime = currentMillis;
  } 
  IrReceiver.resume();
}
