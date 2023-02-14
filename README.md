# Overview
Project running on NANO 33 BLE board with nRF52840 MCU. Central is responible for electronics interfacing motor controller (PWM)

## Characteristics of interest
BLEUnsignedIntCharacteristic x_readingChar("76ad7aa1-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEUnsignedIntCharacteristic y_readingChar("76ad7aa2-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEUnsignedIntCharacteristic x_rawReadingChar("76ad7ab1-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEUnsignedIntCharacteristic y_rawReadingChar("76ad7ab2-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEBooleanCharacteristic button2Char("76ad7aa3-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEBooleanCharacteristic button3Char("76ad7aa4-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEBooleanCharacteristic button4Char("76ad7aa5-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEUnsignedIntCharacteristic turningDirectionChar("76ad7aa6-3782-11ed-a261-0242ac120002", BLERead | BLENotify);

BLEUnsignedIntCharacteristic runningDirectionChar("76ad7aa7-3782-11ed-a261-0242ac120002", BLERead | BLENotify);