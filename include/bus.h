#ifndef BUS_H
#define BUS_H

#include "types.h"
#include "state.h"

/* Device contract: every device implements these */
typedef struct {
    byte (*get_type)(void);
    byte (*get_status)(void);
    void (*handle_command)(byte cmd);
    byte (*read_data)(void);
    void (*write_data)(byte value);
} BusDevice;

/* Bus controller functions */
//void bus_init(void);
void bus_init(struct CPU_State* cpu);
void bus_register_device(byte id, BusDevice* dev);
void bus_select(struct CPU_State* cpu, byte id);
byte bus_read_type(struct CPU_State* cpu);
byte bus_read_status(struct CPU_State* cpu);
void bus_write_command(struct CPU_State* cpu, byte cmd);
byte bus_read_data(struct CPU_State* cpu);
void bus_write_data(struct CPU_State* cpu, byte value);
byte bus_read_int_source(struct CPU_State* cpu);

#endif /* BUS_H */
