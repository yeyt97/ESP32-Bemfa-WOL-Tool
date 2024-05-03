#include "bemfa_client.hpp"
#include "WakeOnLan.h"
#include "wifi_helper.hpp"

BemfaClient* bemfaClient;
WifiHelper* wifiHelper;

WiFiUDP UDP;
WakeOnLan WOL(UDP);

const int LED_Pin = 2;

const char *SSID = "xxxx";  // wifi名称
const char *PSW = "xxxx";  // wifi密码

const char *UID = "xxxx";  // 巴法云的UID
const char *TOPIC = "xxxx";  // 巴法云中创建的TOPIC(不是昵称)

const char *TARGET_MAC_ADDR = "xx:xx:xx:xx:xx:xx";  // 要启动的电脑的MAC地址

void blinkTwice() {
    digitalWrite(LED_Pin, HIGH);
    delay(200);
    digitalWrite(LED_Pin, LOW);
    delay(200);
    digitalWrite(LED_Pin, HIGH);
    delay(200);
    digitalWrite(LED_Pin, LOW);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Beginning...");
    wifiHelper = new WifiHelper(SSID, PSW, LED_Pin);
    bemfaClient = new BemfaClient(UID, TOPIC);
    bemfaClient->addCommand("on", []{
        Serial.println("Send magic packet");
        WOL.setRepeat(3, 100);
        WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
        WOL.sendMagicPacket(TARGET_MAC_ADDR);
        blinkTwice();
    });
}

void loop() {
    wifiHelper->tick();
    bemfaClient->tick();
}