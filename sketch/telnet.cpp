#include "telnet.h"

#include <WiFi.h>

#include "const.h"
#include "heatpump.h"
#include "hexdump.h"
#include "packet.h"
#include "rs485.h"

WiFiServer _server(23);
WiFiClient _client;

bool _debugMode = true;
int temp = 32;

void telnet_setup() {
    _server.begin();
}

void telnet_loop() {
    WiFiClient client = _server.available();
    if (client) {
        _client = client;
    }

    if (_client) {
        char cmd = _client.read();
        if (cmd == 'd') {
            _debugMode = !_debugMode;
            _client.print("Debug Mode: ");
            _client.println(_debugMode);
        }

        if (cmd == 'o') {
            _client.println("Sending On Command");
            heatpump_power_on();
        }

        if (cmd == 'x') {
            _client.println("Sending Off Command");
            heatpump_power_off();
        }

        if (cmd == 'h') {
            temp = 32;
            _client.println("Sending Set Point Command");
            heatpump_set_point(temp);
        }

        if (cmd == '+') {
            if (temp <= 32) {
                temp++;
            }
            _client.println("Sending Set Point Command");
            heatpump_set_point(temp);
        }

        if (cmd == '-') {
            if (temp >= 12) {
                temp--;
            }
            _client.println("Sending Set Point Command");
            heatpump_set_point(temp);
        }
    }
}

void telnet_debug_message(String message) {
    if (_client) {
        _client.println(message);
    }
}

void telnet_process_packet(struct Packet packet) {
    if (_client && _debugMode) {
        _client.println("-----------------------------");
        _client.println("Packet Dump:");
        HexDump(_client, packet.buffer, packet.length);

        if (packet.length <= 3) {
            return;
        }

        _client.print("CRC         : ");
        _client.print(packet_read_crc(packet));
        _client.println(packet_is_valid(packet) ? " Valid" : " Invalid");
        _client.print("Packet Type   : ");
        if (packet_is_status(packet)) {
            _client.println("Status");
            _client.print("Power       : ");
            _client.println(packet_read_power_on(packet) ? "ON" : "OFF");
            _client.print("Set Point   : ");
            _client.println(packet_read_set_point(packet));
            _client.print("Water Temp  : ");
            _client.println(packet_read_water_temp(packet));
            _client.print("Flow        : ");
            _client.println(packet_read_flow(packet) ? "YES" : "NO");
            _client.print("Ambient Temp: ");
            _client.println(packet_read_byte(packet, 6));
            _client.print("Heating     : ");
            _client.println(packet_read_heating(packet) ? "YES" : "NO");
            _client.print("Warm Up     : ");
            _client.println(packet_read_warm_up(packet) ? "YES" : "NO");
        } else if (packet.buffer[2] == 0x0C) {
            _client.println("Command");
        } else {
            _client.println("Unknown");
        }

        _client.println();
        _client.println();
    }
}
