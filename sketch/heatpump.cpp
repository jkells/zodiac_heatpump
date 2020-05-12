#include "heatpump.h"

#include "const.h"
#include "packet.h"
#include "rs485.h"
#include "telnet.h"

static struct Status status = {
    false,
    false,
    false,
    0,
    0,
};

struct Packet sendQueue[SEND_QUEUE_SIZE];
int sendQueueIn = 0;
int sendQueueOut = 0;

void send_queue_push(struct Packet packet) {
    sendQueue[sendQueueIn] = packet;
    sendQueueIn = (sendQueueIn + 1) % SEND_QUEUE_SIZE;
}

void send_queue_push_3(struct Packet packet) {
    for (int i = 0; i < 3; i++) {
        send_queue_push(packet);
    }
}

struct Packet send_queue_pop() {
    if (sendQueueOut != sendQueueIn) {
        struct Packet packet = sendQueue[sendQueueOut];
        sendQueueOut = (sendQueueOut + 1) % SEND_QUEUE_SIZE;
        return packet;
    }
    struct Packet packet;
    packet.length = 0;
    return packet;
}

static long lastUpdateSent = millis();

void heatpump_power_on() {
    send_queue_push_3(packet_create(true, status.setPointTemperature));
}

void heatpump_power_off() {
    send_queue_push_3(packet_create(false, status.setPointTemperature));
}

void heatpump_set_point(uint8_t temp) {
    send_queue_push_3(packet_create(status.powerOn, temp));
}

void heatpump_process_packet(struct Packet packet) {
    if (!packet_is_valid(packet) || !(packet_is_status(packet))) {
        return;
    }

    bool powerOn = packet_read_power_on(packet);
    bool heatingOn = packet_read_heating(packet);
    bool waterFlowing = packet_read_flow(packet);
    uint8_t setPointTemperature = packet_read_set_point(packet);
    uint8_t waterTemperature = packet_read_water_temp(packet);

    status = {
        powerOn,
        heatingOn,
        waterFlowing,
        setPointTemperature,
        waterTemperature,
    };
}

void heatpump_loop() {
    // Don't try and send more than one message every 555ms
    // We pick a weird number so at least one of them wont overlap with
    // the status messages on the bus.
    if (millis() - lastUpdateSent < 555) {
        return;
    }

    struct Packet packet = send_queue_pop();
    if (packet.length) {
        rs485_transmit_packet(packet);
        lastUpdateSent = millis();
        telnet_debug_message("Sending Commannd");
    }
}

struct Status heatpump_get_status() {
    return status;
}