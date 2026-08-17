#ifndef STDDEV_H
#define STDDEV_H

#include "../include/bus.h"
/*
 *  Default stdio device
 */


#define DEVICE_TYPE_STDIO 0x01

/* Commands recognized by BUS_COMMAND (0xFE03) */
#define STDIO_CMD_FLUSH 0x01  /* Force flush host stdout buffer */
#define STDIO_CMD_CLEAR 0x02  /* Clear host terminal screen    */

/*
 * Returns the BusDevice interface structure for the stdio terminal.
 * Attach this to PICO-BUS using bus_register_device(id, stdio_device_get());
 */
BusDevice* stdio_device_get(void);

#endif /* STDDEV_H */
