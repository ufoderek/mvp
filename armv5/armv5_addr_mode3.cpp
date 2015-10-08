#include <armv5.h>
#include <bit_opt.h>

void ARMV5::addr_mode3_imm()
{
    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);
    bool l = inst & M(20);
    uint8_t rn = SMM(inst, 19, 16);
    uint8_t immedH = SMM(inst, 11, 8);
    bool s = inst & M(6);
    bool h = inst & M(5);
    uint8_t immedL = SMM(inst, 3, 0);

    uint8_t offset_8 = 0;
    uint32_t rn_val = 0;
    uint32_t rn_val_old = 0;
    uint32_t addr = 0;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rn_val_old = rn_val;

    if (rn == 15) {
        printb(core_id, d_armv5_decode, "addr_mode3_reg rn = 15");
    }

    if (p && !w) {
        offset_8 = (immedH << 4) | immedL;

        if (u) {
            addr = rn_val + offset_8;
        }

        else {
            addr = rn_val - offset_8;
        }
    }

    else if (p && w) {
        offset_8 = (immedH << 4) | immedL;

        if (u) {
            addr = rn_val + offset_8;
        }

        else {
            addr = rn_val - offset_8;
        }

        rfWrite(addr, rn, CPSR_MODE(rf.cpsr));
    }

    else if (!p && !w) {
        addr = rn_val;
        offset_8 = (immedH << 4) | immedL;

        if (u) {
            rn_val += offset_8;
        }

        else {
            rn_val -= offset_8;
        }

        rfWrite(rn_val, rn, CPSR_MODE(rf.cpsr));
    }

    else {
        printb(core_id, d_armv5_decode, "addr_mode3_imm decode error");
    }

    uint8_t tmp = 0;

    if (l) {
        tmp |= M(2);
    }

    if (s) {
        tmp |= M(1);
    }

    if (h) {
        tmp |= M(0);
    }

    switch (tmp) {
        case B8(001):
            arm_strh(addr, rn_val_old);
            break;
        case B8(010):
            arm_ldrd(addr, rn_val_old);
            break;
        case B8(011):
            arm_strd(addr, rn_val_old);
            break;
        case B8(101):
            arm_ldrh(addr, rn_val_old);
            break;
        case B8(110):
            arm_ldrsb(addr, rn_val_old);
            break;
        case B8(111):
            arm_ldrsh(addr, rn_val_old);
            break;
        default:
            printb(core_id, d_armv5_decode, "addr_mode3_imm decode error");
    }
}

void ARMV5::addr_mode3_reg()
{
    uint8_t SBZ = SMM(inst, 11, 8);

    if (SBZ != 0) {
        printb(core_id, d_armv5_decode, "addr_mode3_reg check error");
    }

    bool p = inst & M(24);
    bool u = inst & M(23);
    bool w = inst & M(21);
    bool l = inst & M(20);
    uint8_t rn = SMM(inst, 19, 16);
    bool s = inst & M(6);
    bool h = inst & M(5);
    uint8_t rm = SMM(inst, 3, 0);

    uint32_t rn_val = 0;
    uint32_t rm_val = 0;
    uint32_t rn_val_old = 0;
    uint32_t addr = 0;

    rfRead(&rn_val, rn, CPSR_MODE(rf.cpsr));
    rfRead(&rm_val, rm, CPSR_MODE(rf.cpsr));
    rn_val_old = rn_val;

    if (rn == 15) {
        printb(core_id, d_armv5_decode, "addr_mode3_reg rn = 15");
    }

    if (rm == 15) {
        printb(core_id, d_armv5_decode, "addr_mode3_reg rm = 15");
    }

    if (p && !w) {
        if (u) {
            addr = rn_val + rm_val;
        }

        else {
            addr = rn_val - rm_val;
        }
    }

    else if (p && w) {
        if (u) {
            addr = rn_val + rm_val;
        }

        else {
            addr = rn_val - rm_val;
        }

        rfWrite(addr, rn, CPSR_MODE(rf.cpsr));
    }

    else if (!p && !w) {
        addr = rn_val;

        if (u) {
            rn_val += rm_val;
        }

        else {
            rn_val -= rm_val;
        }

        rfWrite(rn_val, rn, CPSR_MODE(rf.cpsr));
    }

    else {
        printb(core_id, d_armv5_decode, "addr_mode3_reg p-bit w-bit decode error");
    }

    uint8_t tmp = 0;

    if (l) {
        tmp |= M(2);
    }

    if (s) {
        tmp |= M(1);
    }

    if (h) {
        tmp |= M(0);
    }

    switch (tmp) {
        case B8(001):
            arm_strh(addr, rn_val_old);
            break;
        case B8(010):
            arm_ldrd(addr, rn_val_old);
            break;
        case B8(011):
            arm_strd(addr, rn_val_old);
            break;
        case B8(101):
            arm_ldrh(addr, rn_val_old);
            break;
        case B8(110):
            arm_ldrsb(addr, rn_val_old);
            break;
        case B8(111):
            arm_ldrsh(addr, rn_val_old);
            break;
        default:
            printb(core_id, d_armv5_decode, "addr_mode3_reg l-bit s-bit h-bit decode error");
    }
}

