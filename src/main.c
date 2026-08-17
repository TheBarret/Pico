#include <stdio.h>
#include <assert.h>

#include "../include/state.h"
#include "../include/memory.h"
#include "../include/types.h"
#include "../include/decoder.h"
#include "../include/alu.h"
#include "../include/execute.h"
#include "../include/host.h"
#include "../include/bus.h"
#include "../devices/stddev.h"


int main(void) {
        printf("Loading Pico8...\n");
        struct CPU_State* cpu = cpu_create();
        cpu_reset(cpu);

        printf("Loading Pico Bus...\n");
        bus_init(cpu);

        printf("Registering stdio...\n");
        bus_register_device(1, stdio_device_get());

        // todo: firmware

        cpu_destroy(cpu);
        printf("Finished\n");
        return 0;
}
