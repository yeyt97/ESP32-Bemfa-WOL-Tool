#include <WiFi.h>

class WifiHelper {
private:
    const char* ssid;
    const char* psw;
    uint32_t lastWiFiCheckTick;
    const uint8_t ledPin;
public:
    WifiHelper(const char* ssid, const char* psw, uint8_t ledPin) : ssid(ssid), psw(psw), ledPin(ledPin){
        pinMode(ledPin,OUTPUT);
        digitalWrite(ledPin,HIGH);
        connect();
        lastWiFiCheckTick = millis();
    }

    wl_status_t connect() {
        Serial.printf("WIFI: Try to connect to %s\r\n", ssid);
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        return WiFi.begin(ssid, psw);
    }

    void tick() {
        if (WiFi.status() != WL_CONNECTED) {
            digitalWrite(ledPin, HIGH);
            if (millis() - lastWiFiCheckTick > 3000) {
                Serial.println("Wifi: Disconnected...");
                connect();
                lastWiFiCheckTick = millis();
            }
        } else {
            digitalWrite(ledPin, LOW);
        }
    }
};