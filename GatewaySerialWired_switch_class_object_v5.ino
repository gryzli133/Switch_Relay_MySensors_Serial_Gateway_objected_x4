

// Enable debug prints to serial monitor
#define MY_DEBUG 


// Enable and select radio type attached
//#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level. 
//#define MY_RF24_PA_LEVEL RF24_PA_LOW

// Enable serial gateway
#define MY_GATEWAY_SERIAL

// Define a lower baud rate for Arduino's running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
#if F_CPU == 8000000L
#define MY_BAUD_RATE 38400
#endif

// Flash leds on rx/tx/err
// #define MY_LEDS_BLINKING_FEATURE
// Set blinking period
// #define MY_DEFAULT_LED_BLINK_PERIOD 300

// Inverses the behavior of leds
// #define MY_WITH_LEDS_BLINKING_INVERSE

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
#define MY_INCLUSION_BUTTON_FEATURE

// Inverses behavior of inclusion button (if using external pullup)
//#define MY_INCLUSION_BUTTON_EXTERNAL_PULLUP

// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60 
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  3 

// Uncomment to override default HW configurations
//#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  6  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  5  // the PCB, on board LED

// Enable repeater functionality for this node
// #define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>  
#include <Bounce2.h>


class SwitchRelay
{ 

  uint8_t child_id;
  int buttonPin;
  int relayPin;
  bool relayON;
  bool relayOFF;
  Bounce debouncer = Bounce();

  public:
  SwitchRelay(int childId, int button, int relay, int debaunce, bool invertedRelay) : msg(childId, V_LIGHT)
  {
    child_id = childId;
    buttonPin = button;
    relayPin = relay;
    relayON = !invertedRelay;
    relayOFF = invertedRelay;
    pinMode(buttonPin, INPUT_PULLUP);
    debouncer.attach(buttonPin);
    debouncer.interval(debaunce);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, loadState(child_id)?relayON:relayOFF); 
  }
  
  MyMessage msg;
  
  void Update()
  {
    if (debouncer.update()) 
    {
      // Get the update value.
      int value = debouncer.read();
      // Send in the new value.
      if(value == LOW)
      {
         saveState(child_id, !loadState(child_id));
         digitalWrite(relayPin, loadState(child_id)?relayON:relayOFF);
         SyncController();
      }
    }    
  }

  void SyncController()
  {
    send(msg.set(loadState(child_id)));
  }
  
  void Present()
  {
    present(child_id, S_LIGHT);
  }

  void Receive(const MyMessage &message)
  {
    // We only expect one type of message from controller. But we better check anyway.
    if (message.type==V_LIGHT && message.sensor==child_id) 
    {
       // Change relay state
       digitalWrite(relayPin, message.getBool()?relayON:relayOFF);
       // Store state in eeprom
       saveState(message.sensor, message.getBool());
       // Write some debug info
       Serial.print("Incoming change for sensor:");
       Serial.print(message.sensor);
       Serial.print(", New status: ");
       Serial.println(message.getBool());
     } 
  }

  
};

// define your switch/relay objects here
// SwitchRelay(int childId, int button, int relay, int debaunce, bool invertedRelay)
SwitchRelay switch1(1, 4, A1, 50, 0);   // 0 = D3,  12 = D6 
SwitchRelay switch2(2, 5, A2, 50, 0);  // 10= SD3, 13 = D7
SwitchRelay switch3(3, 6, A3, 50, 0);   // 4 = D2,  14 = D5
SwitchRelay switch4(4, 7, A4, 50, 0);    // 2 = D4,  5  = D1


void setup() 
{ 
  // Setup locally attached sensors
  delay(10000);
  switch1.SyncController(); // send actual value to controller
  switch2.SyncController();
  switch3.SyncController();
  switch4.SyncController();
}
void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay as class object", "1.0");
  switch1.Present();
  switch2.Present();
  switch3.Present();
  switch4.Present();
}

void loop() 
{ 
  switch1.Update();
  switch2.Update();
  switch3.Update();
  switch4.Update();
}

void receive(const MyMessage &message) {
  switch1.Receive(message);
  switch2.Receive(message);
  switch3.Receive(message);
  switch4.Receive(message);
}
