#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>

BLEHIDDevice* hid;
BLECharacteristic* inputKeyboard;
bool connected = false;

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    connected = true;
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    connected = false;
    Serial.println("Disconnected");
    pServer->startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("esp32");

  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  hid = new BLEHIDDevice(pServer);

  inputKeyboard = hid->inputReport(1);

  hid->hidInfo(0x00, 0x01);

  const uint8_t reportMap[] = {
    0x05,0x01,
    0x09,0x06,
    0xA1,0x01,
    0x05,0x07,
    0x19,0xE0,
    0x29,0xE7,
    0x15,0x00,
    0x25,0x01,
    0x75,0x01,
    0x95,0x08,
    0x81,0x02,
    0x95,0x01,
    0x75,0x08,
    0x81,0x01,
    0x95,0x05,
    0x75,0x01,
    0x05,0x08,
    0x19,0x01,
    0x29,0x05,
    0x91,0x02,
    0x95,0x01,
    0x75,0x03,
    0x91,0x01,
    0x95,0x06,
    0x75,0x08,
    0x15,0x00,
    0x25,0x65,
    0x05,0x07,
    0x19,0x00,
    0x29,0x65,
    0x81,0x00,
    0xC0
  };

  hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));

  hid->startServices();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(0x03C1);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->start();

  Serial.println("BLE HID Keyboard Ready");
}

void loop() {

  if (connected) {

    static unsigned long lastSend = 0;

    if (millis() - lastSend > 5000) {

      lastSend = millis();

      uint8_t report[8] = {0};

      report[2] = 0x28;

      inputKeyboard->setValue(report,8);
      inputKeyboard->notify();

      delay(10);

      report[2] = 0;

      inputKeyboard->setValue(report,8);
      inputKeyboard->notify();

      Serial.println("Enter Sent");
    }
  }

  delay(10);
}
