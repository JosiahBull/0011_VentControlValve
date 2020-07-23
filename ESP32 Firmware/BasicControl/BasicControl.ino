#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <EEPROM.h>
#include "AsyncJson.h"

// Quick Config
const char* ssid = "BullFamilyWifi";
const char* password = "blessedtobless";
const int closedSwitchPin = 33;
const int openSwitchPin = 18;
const int relayOnePin = 4;
const int relayTwoPin = 2;
//const int statusLedPin = 13;

//Global Variables
unsigned long motorTimeout = 8500;
bool error;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Functions
void errorOccured() {
  //To be called when an error is detected. Will permenantly disable device until reset signal recieved.
  error = true;
  EEPROM.write(2, 1);
}
void resetError() {
  //Resets Error
  error = false;
  EEPROM.write(2, 0);
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
    bool pressed() {
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
          };
          if (currentTime - _startTime >= motorTimeout) {
            errorOccured();
          };
        };
      };
    };
    bool getStatus() { 
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
    bool isOn() {
      return _on;  
    };
};

//Instantiate Things
Button switchOne(closedSwitchPin);
Button switchTwo(openSwitchPin);
Motor mainMotor(relayOnePin, relayTwoPin, switchOne, switchTwo);
//Led statusLed(statusLedPin);
 
void setup(){  
  // Serial port for debugging purposes
  Serial.begin(115200);
  //Load Error Status on startup.
  if (EEPROM.read(2) == 1) {
    errorOccured();
  };

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  } 
  
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  // Gets the status page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Status Request Received");
    request->send(200, "application/json", "{\"shutterOpen\":\"" + String(mainMotor.getStatus()) + "\", \"errorStatus\":\"" + String(error) + "\"}");
  });

  // Route to open shutter
  server.on("/open", HTTP_POST, [](AsyncWebServerRequest *request){
    mainMotor.openShutter();
    request->send(200, "application/json", "{\"shutterOpen\":\"" + String(mainMotor.getStatus()) + "\", \"errorStatus\":\"" + String(error) + "\"}");
  });
  
  // Route to close shutter
  server.on("/close", HTTP_POST, [](AsyncWebServerRequest *request){
    mainMotor.closeShutter(); 
    request->send(200, "application/json", "{\"shutterOpen\":\"" + String(mainMotor.getStatus()) + "\", \"errorStatus\":\"" + String(error) + "\"}");
  });

  //Route to reset the shutter from an Error
   server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    resetError();
    request->send(200, "application/json", "{\"shutterOpen\":\"" + String(mainMotor.getStatus()) + "\", \"errorStatus\":\"" + String(error) + "\"}");
  });
  
  // Start server
  server.begin();
}
 
void loop(){
  mainMotor.msCheck(millis());
  delay(250);
}
