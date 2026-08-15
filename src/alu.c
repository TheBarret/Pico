#include <stdbool.h>

#include "../include/alu.h"
#include "../include/decoder.h"


byte alu_execute(byte a, byte b, opcode operation, byte* flags) {
    if (!flags) return 0;

    uint16_t temp_16 = 0;
    byte result = 0;
    byte new_flags = *flags;

    // 1. ADD / INC Operations
    if ((operation >= 0x70 && operation <= 0x78) || (operation >= 0xC0 && operation <= 0xC7)) {
        byte operand_b = (operation >= 0xC0 && operation <= 0xC7) ? 1 : b;
        temp_16 = (uint16_t)a + (uint16_t)operand_b;
        result = (byte)(temp_16 & 0xFF);

        // FLAG_C: Carry out of bit 7
        if (temp_16 > 0xFF) new_flags |= FLAG_C;
        else                new_flags &= ~FLAG_C;

        // FLAG_O (ADD): Overflow if both inputs share the same sign, but result sign differs
        bool a_neg = (a & 0x80) != 0;
        bool b_neg = (operand_b & 0x80) != 0;
        bool res_neg = (result & 0x80) != 0;

        if ((a_neg == b_neg) && (res_neg != a_neg)) {
            new_flags |= FLAG_O;
        } else {
            new_flags &= ~FLAG_O;
        }
    }
    // 2. SUB / CMP / DEC Operations
    else if ((operation >= 0x80 && operation <= 0x88) ||
             (operation >= 0xA8 && operation <= 0xAF) ||
             (operation == OP_CMP_IMM) ||
             (operation >= 0xC8 && operation <= 0xCF)) {

        byte operand_b = (operation >= 0xC8 && operation <= 0xCF) ? 1 : b;
        temp_16 = (uint16_t)a - (uint16_t)operand_b;
        result = (byte)(temp_16 & 0xFF);

        // FLAG_C: Borrow required (a < b unsigned)
        if (a < operand_b) new_flags |= FLAG_C;
        else               new_flags &= ~FLAG_C;

        // FLAG_O (SUB): Overflow if inputs have different signs, and result sign differs from 'a'
        bool a_neg = (a & 0x80) != 0;
        bool b_neg = (operand_b & 0x80) != 0;
        bool res_neg = (result & 0x80) != 0;

        if ((a_neg != b_neg) && (res_neg != a_neg)) {
            new_flags |= FLAG_O;
        } else {
            new_flags &= ~FLAG_O;
        }
    }
    // 3. AND / TEST
    else if ((operation >= 0x90 && operation <= 0x97) || (operation >= 0xB8 && operation <= 0xBF)) {
        result = a & b;
        new_flags &= ~(FLAG_C | FLAG_O);
    }
    // 4. OR
    else if (operation >= 0x98 && operation <= 0x9F) {
        result = a | b;
        new_flags &= ~(FLAG_C | FLAG_O);
    }
    // 5. XOR
    else if (operation >= 0xA0 && operation <= 0xA7) {
        result = a ^ b;
        new_flags &= ~(FLAG_C | FLAG_O);
    }
    // 6. Shift Left
    else if (operation == OP_SHL) {
        temp_16 = (uint16_t)a << 1;
        result = (byte)(temp_16 & 0xFF);
        if (a & 0x80) new_flags |= FLAG_C;
        else          new_flags &= ~FLAG_C;
        new_flags &= ~FLAG_O;
    }
    // 7. Shift Right
    else if (operation == OP_SHR) {
        result = a >> 1;
        if (a & 0x01) new_flags |= FLAG_C;
        else          new_flags &= ~FLAG_C;
        new_flags &= ~FLAG_O;
    }
    // 8. Rotate Left
    else if (operation == OP_ROL) {
        byte old_carry = (new_flags & FLAG_C) ? 1 : 0;
        result = (byte)((a << 1) | old_carry);
        if (a & 0x80) new_flags |= FLAG_C;
        else          new_flags &= ~FLAG_C;
        new_flags &= ~FLAG_O;
    }
    else {
        return 0;
    }

    // Common Post-Calculation Flags: FLAG_Z & FLAG_S
    if (result == 0) new_flags |= FLAG_Z;
    else              new_flags &= ~FLAG_Z;

    if (result & 0x80) new_flags |= FLAG_S;
    else                new_flags &= ~FLAG_S;

    *flags = new_flags;
    return result;
}
