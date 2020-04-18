//Quick Config
const int closedSwitchPin = 8;
const int openSwitchPin = 9;
const int relayOnePin = 11;
const int relayTwoPin = 12;
const int statusLedPin = 13;

const int commAddress = 0x04;

//Include shit
#include <Wire.h>
#include <EEPROM.h>

//Global Variables
unsigned long motorTimeout = 7000;
unsigned long previousTime = 0;
bool error;

//Functions
void errorOccured() {
  //To be called when an error is detected. Will permenantly disable device until reset signal recieved.
  error = true;
  EEPROM.write(2, 1);
  sendData(0x6);
}
void resetError() {
  //Resets Error
  error = false;
  EEPROM.write(2, 0);
}

void sendData(byte data) {
  //0x3 - Pong
  //0x4 - Shutter Open
  //0x5 - Shutter Closed
  //0x6 - Shutter Error
  //0x7 - Shutter Movement Completed
  Wire.write(data);
}

//Declare Classes
class Button {
  bool _pressed;
  int _pin;
  public:
    Button(int pin) : _pin(pin) {
      pinMode(_pin, OUTPUT);
      digitalWrite(_pin, LOW);
    };
    pressed() {
      if (digitalRead(_pin) == HIGH) {
        _pressed = true;
      } else {
        _pressed = false;
      }
      return _pressed;
    }
};

class Motor {
  int _relayPinOne;
  int _relayPinTwo;
  Button _closeSwitch;
  Button _openSwitch;

  bool _running;
  bool _opening;
  bool _open;
  unsigned long _startTime;
  
  public:
    Motor(int relayOne, int relayTwo, Button closeSwitch, Button openSwitch) : _relayPinOne(relayOne), _relayPinTwo(relayTwo), _closeSwitch(closeSwitch), _openSwitch(openSwitch), _running(false), _opening(false), _startTime(0) {
      pinMode(_relayPinOne, OUTPUT);
      pinMode(_relayPinTwo, OUTPUT);
      digitalWrite(_relayPinOne, HIGH);
      digitalWrite(_relayPinTwo, HIGH);
      //Check if vane is open or closed on initalisation, if not sure then run open command until fully open.
      if (_closeSwitch.pressed() == true) {
        _open = false;
      }
      if (_openSwitch.pressed() == true) {
        _open = true;
      }
      if (_openSwitch.pressed() == false && _closeSwitch.pressed() == false) {
        _open = false;
        openShutter();
      }
      
    };
    void closeShutter() {
      if (!_open) return;
      if (error) return;
      _startTime = millis();
      _running = true;
      _opening = false;
      digitalWrite(_relayPinOne, HIGH);
      digitalWrite(_relayPinTwo, LOW);
    };
    
    void openShutter() {
      if (_open) return;
      if (error) return;
      _startTime = millis();
      _running = true;
      _opening = true;
      digitalWrite(_relayPinOne, LOW);
      digitalWrite(_relayPinTwo, HIGH);
    };

    void msCheck(unsigned long currentTime) {

      if (_running) { //Check motor is running
        if (_opening) { //Check if motor if opening or closing, and act accordingly.
          if (_openSwitch.pressed() == true || currentTime - _startTime >= motorTimeout) {
            _open = true;
            _running = false;
            digitalWrite(_relayPinOne, HIGH);
            digitalWrite(_relayPinTwo, HIGH);
            sendData(0x7);
          };
          if (currentTime - _startTime >= motorTimeout) {
            errorOccured();
          };
        } else {
          if (_closeSwitch.pressed() == true || currentTime - _startTime >= motorTimeout) {
            _open = false;
            _running = false;
            digitalWrite(_relayPinOne, HIGH);
            digitalWrite(_relayPinTwo, HIGH);
            sendData(0x7);
          };
          if (currentTime - _startTime >= motorTimeout) {
            errorOccured();
          };
        };
      };
    };
    getStatus() { 
      return _open;
    }
};

class Led {
  int _pin;
  bool _on;
  
  public: 
    Led(int pin) : _pin(pin), _on(false) {
      pinMode(_pin, OUTPUT);
      digitalWrite(_pin, LOW);
    };
    void off() {
      _on = false;
      digitalWrite(_pin, LOW);
    };
    void on() {
      _on = true;
      digitalWrite(_pin, HIGH);
    };
    void toggle() {
      if (_on) {
        off();
      } else {
        on();
      }
    };
    isOn() {
      return _on;  
    };
};

//Instantiate Things
Button switchOne(closedSwitchPin);
Button switchTwo(openSwitchPin);
Motor mainMotor(relayOnePin, relayTwoPin, switchOne, switchTwo);
Led statusLed(statusLedPin);

//I2C Event Handlers
void receiveData(int byteCount) {
  //a (a.k.a 97) - 'ping', expect 0x3 in return for 'pong'.
  //b (98) - 'open shutter'
  //c (99) - 'close shutter'
  //d (100) - 'shutter status'
  //e (101) - 'reset from error'
  
  while(Wire.available()) {
    byte received = Wire.read();
    
    if (received == 97) { //If I receive the letter 'a' I have been pinged.
      sendData(0x3);
    }
    if (received == 98) { //Open Shutter
      mainMotor.openShutter();
    }
    if (received == 99) { //Close Shutter
      mainMotor.closeShutter();
    }
    if (received == 100) { //Shutter Status
      if (error) sendData(0x6);
      if (mainMotor.getStatus() == true) { //If true shutter is open.
        sendData(0x4);
      } else {
        sendData(0x5);
      }
    }
    if (received == 101) { //Reset from Error
      resetError();
    }
  }
}


//Setup
void setup() {        
  //Load Error Status on startup.
  if (EEPROM.read(2) == 1) {
    errorOccured();
  };
  //init i2c
  Wire.begin(commAddress);
  //Event Shit
  Wire.onReceive(receiveData);
}

void loop() {
  mainMotor.msCheck(millis());
  if (error) {
    unsigned long currentTime = millis();
    if (currentTime - previousTime >= 1000) {
      previousTime = currentTime;
      statusLed.toggle();
    }
  } else if (statusLed.isOn() == false) {
    statusLed.on();
  }
}
