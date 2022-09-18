/*
  Read analog value and send over BLE on standard battery monitor characteristic
*/

#include <ArduinoBLE.h>
#define UPDATE_THRESH 2
#define UPDATE_INTERVAL 10

#define UUID_Service 76ad7aaa-3782-11ed-a261-0242ac120002
#define UUID_Char_1 76ad7aa1-3782-11ed-a261-0242ac120002
#define UUID_Char_2 76ad7aa2-3782-11ed-a261-0242ac120002

 // Bluetooth® Low Energy Battery Service
BLEService batteryService("180F");
BLEService joystickService("76ad7aaa-3782-11ed-a261-0242ac120002");

// Bluetooth® Low Energy Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes

BLEUnsignedIntCharacteristic x_readingChar("76ad7aa1-3782-11ed-a261-0242ac120002", BLERead | BLENotify);
BLEUnsignedIntCharacteristic y_readingChar("76ad7aa2-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

int x_prevReading = 0;  // last battery level reading from analog input
int y_prevReading = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the analog reading, in ms

void setup() {
  Serial.begin(9600);    // initialize serial communication
  //while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

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
  BLE.addService(joystickService); // Add the joystick service
  
  batteryLevelChar.writeValue(x_prevReading); // set initial value for this characteristic
  batteryLevelChar.writeValue(y_prevReading); // set initial value for this characteristic

  /* Start advertising Bluetooth® Low Energy.  It will start continuously transmitting Bluetooth® Low Energy
     advertising packets and will be visible to remote Bluetooth® Low Energy central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
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
      long currentMillis = millis();
      // if 200ms have passed, check the battery level:
      if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
        previousMillis = currentMillis;
        updateAnalogReading();
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateAnalogReading() {
  
  /* Read the current voltage level on the A0, A1 analog input pins.
     This is used here to read Joystick pot values
  */
  int x_readingRaw = analogRead(A0);
  int y_readingRaw = analogRead(A1);
  int x_reading = map(x_readingRaw, 0, 1023, 0, 100);
  int y_reading = map(y_readingRaw, 0, 1023, 0, 100);

  if (abs(x_reading-x_prevReading)>UPDATE_THRESH) // if the analog reading level has changed beyond preset threshold
  {      
    Serial.print("x_reading: "); // print it
    Serial.println(x_reading);
    x_readingChar.writeValue((byte)x_reading);  // and update the characteristic
    x_prevReading = x_reading;           // save the level for next comparison
  }
  
  if (abs(y_reading-y_prevReading)>UPDATE_THRESH) // if the analog reading level has changed beyond preset threshold
  {      
    Serial.print("y_reading: "); // print it
    Serial.println(y_reading);
    y_readingChar.writeValue((byte)y_reading); // Update characteristic
    y_prevReading = y_reading;           // save the level for next comparison
  }
}
