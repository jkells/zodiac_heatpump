#ifndef __PACKET_INCLUDED__
#define __PACKET_INCLUDED__

#include <Arduino.h>

#include "const.h"

struct Packet {
    uint8_t buffer[PACKET_BUFFER_SIZE];
    int length;
};

extern bool packet_is_valid(struct Packet packet);
extern bool packet_is_status(struct Packet packet);

extern uint16_t packet_read_crc(struct Packet packet);
extern uint8_t packet_read_byte(struct Packet packet, int offset);
extern bool packet_read_power_on(struct Packet packet);
extern uint8_t packet_read_set_point(struct Packet packet);
extern uint8_t packet_read_water_temp(struct Packet packet);
extern boolean packet_read_flow(struct Packet packet);
extern boolean packet_read_warm_up(struct Packet packet);
extern boolean packet_read_heating(struct Packet packet);

extern struct Packet packet_create(bool powerOn, uint8_t temp);

#endif