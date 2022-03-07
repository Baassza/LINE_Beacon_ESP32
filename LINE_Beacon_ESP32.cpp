#include <LINE_Beacon_ESP32.h>

void LINEBeacon::_dump(const char *title, uint8_t *data, size_t dataSize)
{
  if (debug)
  {
    Serial.printf("%s [%d]:", title, dataSize);
    for (size_t i = 0; i < dataSize; i++)
    {
      Serial.printf(" %02x", data[i]);
    }
    Serial.println();
  }
}
void LINEBeacon::putUint8(uint8_t **bufferPtr, uint8_t data)
{
  *(*bufferPtr)++ = data;
}
void LINEBeacon::putUint16LE(uint8_t **bufferPtr, uint16_t data)
{
  *(*bufferPtr)++ = lowByte(data);
  *(*bufferPtr)++ = highByte(data);
}
void LINEBeacon::putArray(uint8_t **bufferPtr, const void *data, size_t dataSize)
{
  memcpy(*bufferPtr, data, dataSize);
  (*bufferPtr) += dataSize;
}
void LINEBeacon::executeBluetoothHCICommand(uint16_t opCode, const uint8_t *hciData, uint8_t hciDataSize)
{
  uint8_t buf[5 + MAX_BLE_ADVERTISING_DATA_SIZE];
  uint8_t *bufPtr = buf;
  putUint8(&bufPtr, 1);
  putUint16LE(&bufPtr, opCode);
  putUint8(&bufPtr, hciDataSize);
  putArray(&bufPtr, hciData, hciDataSize);
  uint8_t bufSize = bufPtr - buf;
  while (!esp_vhci_host_check_send_available());
  esp_vhci_host_send_packet(buf, bufSize);
}

String LINEBeacon:: getBatteryLevelsString(){
	if(battery_level==0){
		return "external power";
	}else{
		return String(map(battery_level,1,11,0,100))+"%";
	}
}
void LINEBeacon::updateAdvertisingData()
{
  uint8_t data[MAX_BLE_ADVERTISING_DATA_SIZE];
  uint8_t *dataPtr = data;
  {
    putUint8(&dataPtr, 1 + 1);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_FLAG);
    putUint8(&dataPtr, ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
  }
  {
    putUint8(&dataPtr, 1 + 2);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_16SRV_CMPL);
    putUint16LE(&dataPtr, UUID_FOR_LINECORP);
  }
  {
    putUint8(&dataPtr, 1 + 9 + deviceMessageSize);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_SERVICE_DATA);
    putUint16LE(&dataPtr, UUID_FOR_LINECORP);
    putUint8(&dataPtr, 0x02);
    putArray(&dataPtr, HWID, 5);
    putUint8(&dataPtr, 0x7F);
    putArray(&dataPtr, deviceMessage, deviceMessageSize);
  }
  uint8_t dataSize = dataPtr - data;
  data[dataSize - 1] = battery_level;
  _dump("HWID", HWID, 5);
  _dump("simple beacon", data, dataSize);
  Serial.println("battery_level = "+getBatteryLevelsString()+"     power = "+String(power));
  Serial.println();
  uint8_t hciDataSize = 1 + MAX_BLE_ADVERTISING_DATA_SIZE;
  uint8_t hciData[hciDataSize];
  hciData[0] = dataSize;
  memcpy(hciData + 1, data, dataSize);
  executeBluetoothHCICommand(HCI_LE_Set_Advertising_Data, hciData, hciDataSize);
}
void LINEBeacon::enableBluetoothAdvertising()
{
  uint8_t enable = 1;
  executeBluetoothHCICommand(HCI_LE_Set_Advertising_Enable, &enable, 1);
}

LINEBeacon::LINEBeacon(String HandwareID)
{
	uint64_t AA = strtoll ( HandwareID.c_str(), NULL, 16); 
    uint8_t* B  = (uint8_t*) &AA;
    for (int i=4; i>=0; i--){
		HWID[4-i]=B[i];
	}
}
void LINEBeacon::start()
{
  if (debug)
  {
    Serial.begin(115200);
  }
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);
  btStart();
  enableBluetoothAdvertising();
}
void LINEBeacon::interval(int time)
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= time)
  {
    previousMillis = currentMillis;
    updateAdvertisingData();
  }
}
void LINEBeacon::setBatteryLevel(int flag)
{
  if (flag > 11)
  {
    flag = 11;
  }
  if (flag < 0)
  {
    flag = 0;
  }
  battery_level = flag;
}
void LINEBeacon::setPower(int level)
{
  power = level;
  switch (level)
  {
  case 0:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);
  case 1:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N9);
  case 2:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N6);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N6);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N6);
  case 3:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N3);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N3);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N3);
  case 4:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N0);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N0);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N0);
  case 5:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P3);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P3);
  case 6:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P6);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P6);
  case 7:
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
  default:
    if (level < 0)
    {
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);
      power = 0;
    }
    else if (level > 7)
    {
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
      esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
      power = 7;
    }
  }
}
