#ifndef __HEATPUMP_INCLUDED__
#define __HEATPUMP_INCLUDED__

#include <Arduino.h>

struct Status {
    boolean powerOn;
    boolean heatingOn;
    boolean waterFlowing;
    uint8_t setPointTemperature;
    uint8_t waterTemperature;
};

extern void heatpump_loop();
extern void heatpump_power_on();
extern void heatpump_power_off();
extern void heatpump_set_point(uint8_t temp);

extern struct Status heatpump_get_status();
extern void heatpump_process_packet(struct Packet packet);

#endif