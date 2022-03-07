#include <LINE_Beacon_ESP32.h>
LINEBeacon Beacon("your HardwareID"); //HWID
void setup() {
  //Beacon.debug = false; // print log from serial 115200 default true
  Beacon.start(); // start beacon
  //Beacon.setPower(7); // set power  0-7 (-12dbm to 9dbm) default 0 read more https://docs.espressif.com/projects/esp-idf/en/v4.1/api-reference/bluetooth/controller_vhci.html#_CPPv417esp_power_level_t
}
void loop() {
  //without delay() function
  //Beacon.setBatteryLevel(11); // set battery level 0-11 default 0 read more https://developers.line.biz/en/docs/messaging-api/beacon-device-spec/#line-beacon-packet
  Beacon.interval(5000); // interval in 5 sec
}
