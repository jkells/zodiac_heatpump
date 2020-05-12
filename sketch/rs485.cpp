#include "rs485.h"

#include "const.h"
#include "packet.h"

void rs485_setup() {
    pinMode(PIN_DE, OUTPUT);
    pinMode(PIN_RE, OUTPUT);
    rs485_disable_transmit();
    Serial2.begin(9600, SERIAL_8N1, PIN_RX2, PIN_TX2);
}

void rs485_enable_transmit() {
    digitalWrite(PIN_DE, HIGH);
    digitalWrite(PIN_RE, HIGH);
}

void rs485_disable_transmit() {
    digitalWrite(PIN_DE, LOW);
    digitalWrite(PIN_RE, LOW);
}

struct Packet rs485_read_packet() {
    struct Packet packet;
    packet.length = 0;
    while (Serial2.available()) {
        packet.buffer[packet.length] = Serial2.read();
        packet.length = (packet.length + 1) % PACKET_BUFFER_SIZE;
    }

    return packet;
}

void rs485_transmit_packet(struct Packet packet) {
    rs485_enable_transmit();
    delay(50);
    Serial2.write(packet.buffer, packet.length);
    delay(50);
    rs485_disable_transmit();
}
