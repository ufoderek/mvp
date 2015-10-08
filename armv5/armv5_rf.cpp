#include<armv5.h>

/* initialize the register file */
void ARMV5::rfInit(uint32_t addr)
{
    for (int i = 0; i < 8; i++) {
        rf.regs[i] = 0;
    }

    for (int i = 0; i < 5; i++) {
        rf.reg_usr[i] = 0;
        rf.reg_fiq[i] = 0;
    }

    for (int i = 0; i < 6; i++) {
        rf.sp[i] = 0;
        rf.lr[i] = 0;
    }

    rf.pc = addr;
    rf.spsr[0] = 0x400001d0;    // USR & SYS
    rf.spsr[1] = 0x400001d3;    // SVC
    rf.spsr[2] = 0x400001d7;    // ABORT
    rf.spsr[3] = 0x400001db;    // UNDEF
    rf.spsr[4] = 0x400001d2;    // IRQ
    rf.spsr[5] = 0x400001d1;    // FIQ
    rf.cpsr = 0x400001d3;
    //rf.cpsr = (using_prog)? 0x400001d0 : 0x400001d3;
}

/* read the registers */
void ARMV5::rfRead(uint32_t* data, uint16_t index, uint8_t mode)
{
    switch (index) {
        case R00:
        case R01:
        case R02:
        case R03:
        case R04:
        case R05:
        case R06:
        case R07:
            *data = rf.regs[index];
            break;
        case R08:
        case R09:
        case R10:
        case R11:
        case R12:
            *data = (mode == MODE_FIQ) ? rf.reg_fiq[index - R08] : rf.reg_usr[index - R08];
            break;
        case SP:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    *data = rf.sp[0];
                    break;
                case MODE_SVC:
                    *data = rf.sp[1];
                    break;
                case MODE_ABORT:
                    *data = rf.sp[2];
                    break;
                case MODE_UNDEF:
                    *data = rf.sp[3];
                    break;
                case MODE_IRQ:
                    *data = rf.sp[4];
                    break;
                case MODE_FIQ:
                    *data = rf.sp[5];
                    break;
                default:
                    printb(core_id, d_armv5_rf, "register reading error, index = %u, mode = %u, inst: 0x%X", index, mode, inst);
                    break;
            }

            break;
        case LR:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    *data = rf.lr[0];
                    break;
                case MODE_SVC:
                    *data = rf.lr[1];
                    break;
                case MODE_ABORT:
                    *data = rf.lr[2];
                    break;
                case MODE_UNDEF:
                    *data = rf.lr[3];
                    break;
                case MODE_IRQ:
                    *data = rf.lr[4];
                    break;
                case MODE_FIQ:
                    *data = rf.lr[5];
                    break;
                default:
                    printb(core_id, d_armv5_rf, "register reading error, index = %u, mode = %u", index, mode);
                    break;
            }

            break;
        case PC:
            *data = rf.pc + 4;      // in programmer view, the PC value is (PC + 8)
            break;
        case CPSR:
            *data = rf.cpsr;
            break;
        case SPSR:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    *data = rf.spsr[0];
                    break;  // actually, spsr[0] doesn't exist
                case MODE_SVC:
                    *data = rf.spsr[1];
                    break;
                case MODE_ABORT:
                    *data = rf.spsr[2];
                    break;
                case MODE_UNDEF:
                    *data = rf.spsr[3];
                    break;
                case MODE_IRQ:
                    *data = rf.spsr[4];
                    break;
                case MODE_FIQ:
                    *data = rf.spsr[5];
                    break;
                default:
                    printb(core_id, d_armv5_rf, "register reading error, index = %u, mode = %u", index, mode);
                    break;
            }

            break;
        case FPS:
            *data = 0;
            break;
        default:
            printb(core_id, d_armv5_rf, "register reading error, index = %u, mode = %u", index, mode);
            break;
    }
}

/* write the registers */
void ARMV5::rfWrite(uint32_t data, uint16_t index, uint8_t mode)
{
    switch (index) {
        case R00:
        case R01:
        case R02:
        case R03:
        case R04:
        case R05:
        case R06:
        case R07:
            rf.regs[index] = data;
            break;
        case R08:
        case R09:
        case R10:
        case R11:
        case R12:
            (mode == MODE_FIQ) ? (rf.reg_fiq[index - R08] = data) : (rf.reg_usr[index - R08] = data);
            break;
        case SP:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    rf.sp[0] = data;
                    break;
                case MODE_SVC:
                    rf.sp[1] = data;
                    break;
                case MODE_ABORT:
                    rf.sp[2] = data;
                    break;
                case MODE_UNDEF:
                    rf.sp[3] = data;
                    break;
                case MODE_IRQ:
                    rf.sp[4] = data;
                    break;
                case MODE_FIQ:
                    rf.sp[5] = data;
                    break;
                default:
                    cout << "ERROR: register writing error: " << index << " in mode: " << mode << "!!" << endl;
                    exit(EXIT_SUCCESS);
                    break;
            }

            break;
        case LR:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    rf.lr[0] = data;
                    break;
                case MODE_SVC:
                    rf.lr[1] = data;
                    break;
                case MODE_ABORT:
                    rf.lr[2] = data;
                    break;
                case MODE_UNDEF:
                    rf.lr[3] = data;
                    break;
                case MODE_IRQ:
                    rf.lr[4] = data;
                    break;
                case MODE_FIQ:
                    rf.lr[5] = data;
                    break;
                default:
                    cout << "ERROR: register writing error: " << index <<  " in mode: " << mode << "!!" << endl;
                    exit(EXIT_SUCCESS);
                    break;
            }

            break;
        case PC:
            rf.pc = data;
            break;
        case CPSR:
            rf.cpsr = data;
            break;
        case SPSR:

            switch (mode) {
                case MODE_USR:
                case MODE_SYS:
                    rf.spsr[0] = data;
                    break;   // actually, spsr[0] doesn't exist
                case MODE_SVC:
                    rf.spsr[1] = data;
                    break;
                case MODE_ABORT:
                    rf.spsr[2] = data;
                    break;
                case MODE_UNDEF:
                    rf.spsr[3] = data;
                    break;
                case MODE_IRQ:
                    rf.spsr[4] = data;
                    break;
                case MODE_FIQ:
                    rf.spsr[5] = data;
                    break;
                default:
                    cout << "ERROR: register writing error: " << index <<  " in mode: " << mode << "!!" << endl;
                    exit(EXIT_SUCCESS);
                    break;
            }

            break;
        case FPS:
            break;
        default:
            cout << "ERROR: register writing error: " << index << " in mode: " << mode << "!!" << endl;
            exit(EXIT_SUCCESS);
            break;
    }
}
