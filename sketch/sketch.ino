#include <Arduino.h>
#include <ArduinoOTA.h>

#include "const.h"
#include "esp_system.h"
#include "heatpump.h"
#include "rs485.h"
#include "server.h"
#include "telnet.h"

static long connectedTime = 0;

void setup() {
    WiFi.onEvent(network_wifi_event);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    rs485_setup();
    telnet_setup();
    server_setup();
    watchdog_setup();
    ArduinoOTA.begin();
}

void loop() {
    watchdog_loop();
    ArduinoOTA.handle();

    // For the first 15 seconds after connecting to WiFi do nothing.
    // give us a chance to flash it again OTA if we brick it.
    if (connectedTime == 0 || (millis() - connectedTime) < 15000) {
        return;
    }

    telnet_loop();
    server_loop();
    heatpump_loop();

    struct Packet packet = rs485_read_packet();
    if (packet.length > 0) {
        heatpump_process_packet(packet);
        telnet_process_packet(packet);
    }
}

void network_wifi_event(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("Connected to network.");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.print("Connected with IP address ");
            Serial.println(WiFi.localIP());
            connectedTime = millis();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("Lost network connection, attempting to reconnect...");
            WiFi.begin(WIFI_SSID, WIFI_PASS);
            break;
    }
}

static hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
    ets_printf("Watchdog restart!");
    esp_restart();
}

void watchdog_loop() {
    timerWrite(timer, 0);
}

void watchdog_setup() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, 1000 * WATCHDOG_TIMEOUT_MS, false);
    timerAlarmEnable(timer);
}
