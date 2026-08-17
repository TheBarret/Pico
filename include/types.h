#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

/* Layer 0: Core Data Types & Constants */

/* Type Definitions */
typedef uint8_t byte;
typedef uint16_t address;
typedef uint8_t opcode;
typedef int8_t signed_byte;

/* Register Pair Structure (16-bit) */
typedef struct {
    byte low;
    byte high;
} pair;

/* Fixed Constants */
#define RAM_SIZE 65536
#define REGISTER_COUNT 8
#define STACK_PAGE 0x0100

/* Flag Bitmasks */
#define FLAG_Z (1 << 0)
#define FLAG_C (1 << 1)
#define FLAG_S (1 << 2)
#define FLAG_O (1 << 3)


/* Layer 0: Exported Interface */

/* CPU State Structure Definition */
struct CPU_State {
    /* Memory */
    byte* ram;                    /* Dynamic allocation for 64KB */
    address pc;                   /* Program counter */
    uint8_t sp;                   /* 8-bit Stack pointer (wraps within 0x0100-0x01FF page) */

    /* Registers */
    byte regs[REGISTER_COUNT];    /* R0 through R7 */
    byte acc;                     /* Accumulator */

    /* Flags & Control */
    byte flags;                   /* Bitmasked flags (FLAG_Z, FLAG_C, FLAG_S, FLAG_O) */
    unsigned int halted : 1;      /* Execution halt flag */
    unsigned int interrupt_enabled : 1;
    unsigned long long cycles;    /* Cycle counter */
};

/* Pico Bus Control Registers */
#define BUS_BASE        0xFE00
#define BUS_SELECT      0xFE00
#define BUS_TYPE        0xFE01
#define BUS_STATUS      0xFE02
#define BUS_COMMAND     0xFE03
#define BUS_DATA        0xFE04
#define BUS_INT_SOURCE  0xFE05

/* BUS_STATUS values */
#define BUS_READY       0x00
#define BUS_BUSY        0x01
#define BUS_ERROR       0x02
#define BUS_ABSENT      0xFF

#endif /* TYPES_H */
