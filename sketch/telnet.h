#ifndef __TELNET_INCLUDED__
#define __TELNET_INCLUDED__

#include <Arduino.h>

extern void telnet_setup();
extern void telnet_loop();
extern void telnet_process_packet(struct Packet packet);
extern void telnet_debug_message(String message);

#endif