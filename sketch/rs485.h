#ifndef __RS485_INCLUDED__
#define __RS485_INCLUDED__

#include <Arduino.h>

#include "packet.h"

extern void rs485_setup();
extern void rs485_enable_transmit();
extern void rs485_disable_transmit();
extern Packet rs485_read_packet();
extern void rs485_transmit_packet(struct Packet packet);

#endif