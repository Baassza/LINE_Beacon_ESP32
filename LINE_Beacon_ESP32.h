#include <arduino.h>
#include "bt.h"
#include "esp_gap_ble_api.h"

class LINEBeacon
{
private:
  int power = 0;
  uint8_t battery_level = 0x00;
  unsigned long previousMillis = 0;
  uint8_t HWID[5];
  uint16_t UUID_FOR_LINECORP = 0xFE6F;
  int MAX_SIMPLEBEACON_DEVICEMESSAGE_SIZE = 13;
  uint8_t deviceMessageSize = 1;
  int deviceMessage[13];
  uint8_t MAX_BLE_ADVERTISING_DATA_SIZE = 31;
  uint16_t HCI_LE_Set_Advertising_Data = (0x08 << 10) | 0x0008;
  uint16_t HCI_LE_Set_Advertising_Enable = (0x08 << 10) | 0x000A;
  void _dump(const char *title, uint8_t *data, size_t dataSize);
  void putUint8(uint8_t **bufferPtr, uint8_t data);
  void putUint16LE(uint8_t **bufferPtr, uint16_t data);
  void putArray(uint8_t **bufferPtr, const void *data, size_t dataSize);
  void executeBluetoothHCICommand(uint16_t opCode, const uint8_t *hciData, uint8_t hciDataSize);
  void updateAdvertisingData();
  void enableBluetoothAdvertising();
  String getBatteryLevelsString();

public:
  bool debug = true;
  LINEBeacon(String HandwareID);
  void start();
  void interval(int time);
  void setBatteryLevel(int flag);
  void setPower(int level);
};
