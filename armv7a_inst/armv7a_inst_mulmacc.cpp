#include <armv7a.h>

void armv7a::decode_mulmacc(armv7a_ir& inst)
{
    uint32_t op = inst(23, 20);

    switch(op)
    {
        case B(0000):
        case B(0001):
            arm_mul(inst);
            return;
        case B(0010):
        case B(0011):
            arm_mla(inst);
            return;
        case B(0100):
            arm_umaal(inst);
            return;
        case B(0110):
            arm_mls(inst);
            return;
        case B(1000):
        case B(1001):
            arm_umull(inst);
            return;
        case B(1010):
        case B(1011):
            arm_umlal(inst);
            return;
        case B(1100):
        case B(1101):
            arm_smull(inst);
            return;
        case B(1110):
        case B(1111):
            arm_smlal(inst);
            return;
        case B(0101):
        case B(0111):
            printb(d_armv7a_decode_mulmacc, "undef inst: %X", inst.val);
            return;
        default:
            printb(core_id, d_armv7a_decode_mulmacc, "decode error: %X", inst.val);
    }
}

void armv7a::arm_mul(armv7a_ir& inst)
{
    inst.print_inst("arm_mul");
    inst.check(27, 21, 0);
    inst.check(15, 12, 0);
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(19, 16);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((d == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_mul error");
        }

        if((arch_version() < 6) && (d == n))
        {
            printb(d_inst, "arm_mul error 2");
        }

        //ESO
        uint64_t operand1 = rf.r_read(n);
        uint64_t operand2 = rf.r_read(m);
        uint64_t result = operand1 * operand2;
        rf.r_write(d, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N(get_bit64(result, 31));
            rf.cpsr_Z(get_field64(result, 31, 0) == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_mul error 3");
            }
        }
    }
}

void armv7a::arm_mla(armv7a_ir& inst)
{
    inst.print_inst("arm_mla");
    inst.check(27, 21, 1);
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rd = inst(19, 16);
        uint32_t ra = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t a = ra;
        bool setflags = s == 1;

        if((d == 15) || (n == 15) || (m == 15) || (a == 15))
        {
            printb(d_inst, "arm_mla error");
        }

        if((arch_version() < 6 && (d == n)))
        {
            printb(d_inst, "arm_mla error 2");
        }

        //ESO
        uint64_t operand1 = rf.r_read(n);
        uint64_t operand2 = rf.r_read(m);
        uint64_t addend = rf.r_read(a);
        uint64_t result = operand1 * operand2 + addend;
        rf.r_write(d, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N(get_bit64(result, 31));
            rf.cpsr_Z(get_field64(result, 31, 0) == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_mla error 3");
            }
        }
    }
}

void armv7a::arm_umaal(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_umaal not implemented yet.");
}

void armv7a::arm_mls(armv7a_ir& inst)
{
    inst.print_inst("arm_mls");
    inst.check(27, 20, B(0000 0110));
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rd = inst(19, 16);
        uint32_t ra = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t d = rd;
        uint32_t n = rn;
        uint32_t m = rm;
        uint32_t a = ra;

        if((d == 15) || (n == 15) || (m == 15) || (a == 15))
        {
            printb(d_inst, "arm_mls error");
        }

        //ESO
        uint64_t operand1 = rf.r_read(n);
        uint64_t operand2 = rf.r_read(m);
        uint64_t addend = rf.r_read(a);
        uint64_t result = addend - operand1 * operand2;
        rf.r_write(d, get_field64(result, 31, 0));
    }
}

void armv7a::arm_umull(armv7a_ir& inst)
{
    inst.print_inst("arm_umull");
    inst.check(27, 21, B(000 0100));
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rdhi = inst(19, 16);
        uint32_t rdlo = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t dlo = rdlo;
        uint32_t dhi = rdhi;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((dlo == 15) || (dhi == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_umull error");
        }

        if(dhi == dlo)
        {
            printb(d_inst, "arm_umull error 2");
        }

        if((arch_version() < 6) && ((dhi == n) || (dlo == n)))
        {
            printb(d_inst, "arm_umull error 3");
        }

        //ESO
        uint64_t result = (uint64_t)rf.r_read(n) * (uint64_t)rf.r_read(m);
        rf.r_write(dhi, get_field64(result, 63, 32));
        rf.r_write(dlo, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N(get_bit64(result, 63));
            rf.cpsr_Z(result == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_umull error 3");
            }
        }
    }
}

void armv7a::arm_umlal(armv7a_ir& inst)
{
    inst.print_inst("arm_umlal");
    inst.check(27, 21, B(000 0101));
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rdhi = inst(19, 16);
        uint32_t rdlo = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t dlo = rdlo;
        uint32_t dhi = rdhi;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((dlo == 15) || (dhi == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_umlal error");
        }

        if(dhi == dlo)
        {
            printb(d_inst, "arm_umlal error 2");
        }

        if((arch_version() < 6) && ((dhi == n) || (dlo == n)))
        {
            printb(d_inst, "arm_umlal error 3");
        }

        //ESO
        uint64_t result = (uint64_t)rf.r_read(n) * (uint64_t)rf.r_read(m) + ((uint64_t)rf.r_read(dhi) << 32) + (uint64_t)(rf.r_read(dlo));
        rf.r_write(dhi, get_field64(result, 63, 32));
        rf.r_write(dlo, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N(get_bit64(result, 63));
            rf.cpsr_Z(result == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_umlal error 3");
            }
        }
    }
}

void armv7a::arm_smull(armv7a_ir& inst)
{
    inst.print_inst("arm_smull");
    inst.check(27, 21, B(000 0110));
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rdhi = inst(19, 16);
        uint32_t rdlo = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t dlo = rdlo;
        uint32_t dhi = rdhi;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((dlo == 15) || (dhi == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_smull error");
        }

        if(dhi == dlo)
        {
            printb(d_inst, "arm_smull error 2");
        }

        if((arch_version() < 6) && ((dhi == n) || (dlo == n)))
        {
            printb(d_inst, "arm_smull error 3");
        }

        //ESO
        bits _rn(rf.r_read(n), 32);
        bits _rm(rf.r_read(m), 32);
        uint64_t result = sign_extend64(_rn) * sign_extend64(_rm);
        rf.r_write(dhi, get_field64(result, 63, 32));
        rf.r_write(dlo, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N((get_bit64(result, 63)));
            rf.cpsr_Z(result == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_smull error 3");
            }
        }
    }
}

void armv7a::arm_smlal(armv7a_ir& inst)
{
    inst.print_inst("arm_smlal");
    inst.check(27, 21, B(111));
    inst.check(7, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t s = inst(20);
        uint32_t rdhi = inst(19, 16);
        uint32_t rdlo = inst(15, 12);
        uint32_t rm = inst(11, 8);
        uint32_t rn = inst(3, 0);
        uint32_t dlo = rdlo;
        uint32_t dhi = rdhi;
        uint32_t n = rn;
        uint32_t m = rm;
        bool setflags = s == 1;

        if((dlo == 15) || (dhi == 15) || (n == 15) || (m == 15))
        {
            printb(d_inst, "arm_smlal error");
        }

        if(dlo == dhi)
        {
            printb(d_inst, "arm_smlal error 2");
        }

        if((arch_version() < 6) && ((dhi == n) || (dlo == n)))
        {
            printb(d_inst, "arm_smlal error 3");
        }

        //ESO
        bits _rn(rf.r_read(n), 32);
        bits _rm(rf.r_read(m), 32);
        uint64_t result = SInt(_rn) * SInt(_rm) + ((uint64_t)rf.r_read(dhi) << 32) + (uint64_t)rf.r_read(dlo);
        rf.r_write(dhi, get_field64(result, 63, 32));
        rf.r_write(dlo, get_field64(result, 31, 0));

        if(setflags)
        {
            rf.cpsr_N(get_bit(result, 63));
            rf.cpsr_Z(result == 0);

            if(arch_version() == 4)
            {
                printb(d_inst, "arm_smlal error 4");
            }
        }
    }
}

