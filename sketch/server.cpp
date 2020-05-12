#include <WebServer.h>

#include "heatpump.h"

WebServer server(80);

String bool_string(boolean value) {
    return value ? String("true") : String("false");
}

void get_status() {
    struct Status status = heatpump_get_status();
    String response = "{\n";
    response += ("  \"powerOn\": " + bool_string(status.powerOn) + ",\n");
    response += ("  \"heatingOn\": " + bool_string(status.heatingOn) + ",\n");
    response += ("  \"waterFlowing\": " + bool_string(status.waterFlowing) + ",\n");
    response += ("  \"setPointTemperature\": " + String(status.setPointTemperature) + ",\n");
    response += ("  \"waterTemperature\": " + String(status.waterTemperature) + "\n");
    response += ("}\n\n");
    server.send(200, "application/json", response);
}

void get_not_found() {
    server.send(404, "text/html", "Not Found");
}

void post_power() {
    if (!server.hasArg("value")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    bool powerOn = server.arg("value").equalsIgnoreCase("true");
    if (powerOn) {
        heatpump_power_on();
    } else {
        heatpump_power_off();
    }
    server.send(200, "text/plain", "OK");
}

void post_temperature() {
    if (!server.hasArg("value")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    int temp = server.arg("value").toInt();
    if (temp < 12 || temp > 32) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }
    heatpump_set_point(temp);
    server.send(200, "text/plain", "OK");
}

void server_setup() {
    server.on("/status", HTTP_GET, get_status);
    server.on("/power", HTTP_POST, post_power);
    server.on("/temperature", HTTP_POST, post_temperature);
    server.onNotFound(get_not_found);

    server.begin();
}

void server_loop() {
    server.handleClient();
}