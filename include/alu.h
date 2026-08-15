#ifndef ALU_H
#define ALU_H

#include "types.h"

// Core ALU execution function calculating 8-bit result and updated flags
byte alu_execute(byte a, byte b, opcode operation, byte* flags);

#endif // ALU_H
