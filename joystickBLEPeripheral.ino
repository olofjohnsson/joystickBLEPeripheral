/*
  Read analog value and send over BLE on standard battery monitor characteristic
*/

#include <ArduinoBLE.h>
#define UPDATE_THRESH 15
#define UPDATE_INTERVAL 10

#define UUID_Service 76ad7aaa-3782-11ed-a261-0242ac120002
#define UUID_Char_1 76ad7aa1-3782-11ed-a261-0242ac120002
#define UUID_Char_2 76ad7aa2-3782-11ed-a261-0242ac120002

#define BUTTON_2_PIN 2
#define BUTTON_3_PIN 3
#define BUTTON_4_PIN 4

bool stateChanged = false;

bool button_2_State = false;  
bool button_3_State = false; 
bool button_4_State = false; 
bool turningDirection = 0; //Default direction is left
bool runningDirection = 0;//Default is forward

int x_readingRaw = 0;
int y_readingRaw = 0;
int x_reading = 0;
int y_reading = 0;

 // Bluetooth® Low Energy Battery Service
BLEService batteryService("180F");
BLEService joystickService("76ad7aaa-3782-11ed-a261-0242ac120002");

// Bluetooth® Low Energy Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes

BLEUnsignedIntCharacteristic x_readingChar("76ad7aa1-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic y_readingChar("76ad7aa2-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic x_rawReadingChar("76ad7ab1-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic y_rawReadingChar("76ad7ab2-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEBooleanCharacteristic button2Char("76ad7aa3-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEBooleanCharacteristic button3Char("76ad7aa4-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEBooleanCharacteristic button4Char("76ad7aa5-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic turningDirectionChar("76ad7aa6-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic runningDirectionChar("76ad7aa7-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

int x_prevReading = 0;  // last battery level reading from analog input
int y_prevReading = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the analog reading, in ms

void setup() {
  Serial.begin(9600);    // initialize serial communication
  //while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected
  
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2_PIN), setButton_2_State, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3_PIN), setButton_3_State, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_4_PIN), setButton_4_State, CHANGE);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  /* Set a local name for the Bluetooth® Low Energy device
     This name will appear in advertising packets
     and can be used by remote devices to identify this Bluetooth® Low Energy device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("JoyStickReading");
  BLE.setAdvertisedService(batteryService); // add the service UUID
  batteryService.addCharacteristic(batteryLevelChar); // add the battery level characteristic
  BLE.addService(batteryService); // Add the battery service

  BLE.setAdvertisedService(joystickService); // add the service UUID
  joystickService.addCharacteristic(x_readingChar); // add the x reading characteristic
  joystickService.addCharacteristic(y_readingChar); // add the y reading characteristic
  joystickService.addCharacteristic(x_rawReadingChar); // add the x reading characteristic
  joystickService.addCharacteristic(y_rawReadingChar); // add the y reading characteristic
  joystickService.addCharacteristic(button2Char);
  joystickService.addCharacteristic(button3Char);
  joystickService.addCharacteristic(button4Char);
  joystickService.addCharacteristic(turningDirectionChar);
  joystickService.addCharacteristic(runningDirectionChar);
  BLE.addService(joystickService); // Add the joystick service
  
  batteryLevelChar.writeValue(x_prevReading); // set initial value for this characteristic
  batteryLevelChar.writeValue(y_prevReading); // set initial value for this characteristic
  batteryLevelChar.writeValue(button_2_State);
  batteryLevelChar.writeValue(button_3_State);
  batteryLevelChar.writeValue(button_4_State);

  /* Start advertising Bluetooth® Low Energy.  It will start continuously transmitting Bluetooth® Low Energy
     advertising packets and will be visible to remote Bluetooth® Low Energy central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
}

void setButton_2_State()
{
  stateChanged = true;
  button_2_State = !button_2_State;
}

void setButton_3_State()
{
  stateChanged = true;
  button_3_State = !button_3_State;
}

void setButton_4_State()
{
  stateChanged = true;
  button_4_State = !button_4_State;
}

void printButtonState()
{
  Serial.print("Button2: ");
  Serial.println(button_2_State);
  button2Char.writeValue(button_2_State);

  Serial.print("Button3: ");
  Serial.println(button_3_State);
  button3Char.writeValue(button_3_State);

  Serial.print("Button4: ");
  Serial.println(button_4_State);
  button4Char.writeValue(button_4_State);
  
  Serial.print("\n");
  stateChanged = false;
}

void printDirectionState()
{
  Serial.print("runningDirection: ");
  Serial.println(runningDirection);
  runningDirectionChar.writeValue(runningDirection);

  Serial.print("turningDirection: ");
  Serial.println(turningDirection);
  turningDirectionChar.writeValue(turningDirection);
}

void updateAnalogReading() {
  
  /* Read the current voltage level on the A0, A1 analog input pins.
     This is used here to read Joystick pot values
  */
  x_readingRaw = analogRead(A0);
  y_readingRaw = analogRead(A2);
  if (x_readingRaw < 490)
  {
    x_reading = map(x_readingRaw, 0, 511, 255, 0);
    if (x_reading > 15)
    {  
      turningDirection = 0;
    }
  }
  if (x_readingRaw > 530)
  {
    x_reading = map(x_readingRaw, 512, 1024, 0, 255);
    if (x_reading > 15)
    {  
      turningDirection = 1;
    }
  }

  if (y_readingRaw < 490)
  {
    y_reading = map(y_readingRaw, 0, 511, 255, 0);
    if (y_reading > 15)
    {  
      runningDirection = 0;
    }
  }
  if (y_readingRaw > 530)
  {
    y_reading = map(y_readingRaw, 512, 1024, 0, 255);
    if (y_reading > 15)
    {  
      runningDirection = 1;
    }
  }

  if (abs(x_reading-x_prevReading)>UPDATE_THRESH) // if the analog reading level has changed beyond preset threshold
    {      
      Serial.print("x_reading: "); // print it
      Serial.println(x_reading);
      Serial.print("turningDirection: ");
      Serial.println((byte)turningDirection);
      x_readingChar.writeValue((byte)x_reading);  // and update the characteristic
      turningDirectionChar.writeValue((byte)turningDirection);
      x_prevReading = x_reading;           // save the level for next comparison
    }
  
  if (abs(y_reading-y_prevReading)>UPDATE_THRESH) // if the analog reading level has changed beyond preset threshold
  {      
    Serial.print("y_reading: "); // print it
    Serial.println(y_reading);
    Serial.print("runningDirection: ");
    Serial.println((byte)runningDirection);
    y_readingChar.writeValue((byte)y_reading); // Update characteristic
    runningDirectionChar.writeValue((byte)runningDirection);
    y_prevReading = y_reading;           // save the level for next comparison
  }
  x_reading = 0;
  y_reading = 0;
}

void loop() {
  // wait for a Bluetooth® Low Energy central
  BLEDevice central = BLE.central();
  
  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // check the battery level every 200ms
    // while the central is connected:
    while (central.connected()) {
      updateAnalogReading();
      if (stateChanged == true)
      {
        printButtonState();
        stateChanged = false;
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
