#include <armv7a.h>

void armv7a::decode_sync(armv7a_ir& inst)
{
    uint32_t op = inst(23, 20);

    switch(op)
    {
        case B(0000) :
        case B(0100) :
            arm_swp(inst);
            return;
        case B(1000) :
            arm_strex(inst);
            return;
        case B(1001) :
            arm_ldrex(inst);
            return;
        case B(1010) :
            arm_strexd(inst);
            return;
        case B(1011) :
            arm_ldrexd(inst);
            return;
        case B(1100) :
            arm_strexb(inst);
            return;
        case B(1101) :
            arm_ldrexb(inst);
            return;
        case B(1110) :
            arm_strexh(inst);
            return;
        case B(1111) :
            arm_ldrexh(inst);
            return;
        default:
            printb(core_id, d_armv7a_decode_sync, "decode error: %X", inst.val);
            return;
    }
}

void armv7a::arm_swp(armv7a_ir& inst)
{
    inst.print_inst("arm_swp");
    inst.check(27, 23, B(10));
    inst.check(21, 20, 0);
    inst.check(11, 4, B(1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t b = inst(22);
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t rt2 = inst(3, 0);

        uint32_t t = rt;
        uint32_t t2 = rt2;
        uint32_t n = rn;
        uint32_t size = (b == 1) ? 1 : 4;

        if((t == 15) || (t2 == 15) || (n == t) || (n == t2))
        {
            printb(d_inst, "arm_swp error");
        }

        //ESO

        bits data;
        mem_a_read(&data, rf.r_read(n), size);

        bits _t2(rf.r_read(t2), 8 * size);
        mem_a_write(rf.r_read(n), size, _t2);

        if(size == 1)
        {
            rf.r_write(t, data.val);
        }

        else
        {
            rf.r_write(t, data.val);
        }
    }
}

void armv7a::arm_strex(armv7a_ir& inst)
{
    inst.print_inst("arm_strex");
    inst.check(27, 20, B(0001 1000));
    inst.check(11, 4, B(1111 1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rt = inst(3, 0);
        uint32_t d = rd;
        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = 0;

        if((d == 15) || (t == 15) || (n == 15))
        {
            printb(d_inst, "arm_strex error");
        }

        if((d == n) || (d == t))
        {
            printb(d_inst, "arm_strex error 2");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n) + imm32;

        if(exclusive_monitor_pass(address, 4))
        {
            bits _rt(rf.r_read(t), 32);

            if(! mem_a_write(address, 4, _rt))
            {
                return;
            }

            rf.r_write(d, 0);
        }

        else
        {
            rf.r_write(d, 1);
        }
    }
}

void armv7a::arm_ldrex(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrex");
    inst.check(27, 20, B(0001 1001));
    inst.check(11, 0, B(1111 1001 1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t t = rt;
        uint32_t n = rn;
        uint32_t imm32 = 0;

        if((t == 15) || (n == 15))
        {
            printb(d_inst, "arm_ldrex error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n) + imm32;
        set_exclusive_monitor(address, 4);
        bits data;

        if(!mem_a_read(&data, address, 4))
        {
            return;
        }

        rf.r_write(t, data.val);
    }
}

void armv7a::arm_strexd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_strexd not implemented yet.");
}

void armv7a::arm_ldrexd(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrexd not implemented yet.");
}

void armv7a::arm_strexb(armv7a_ir& inst)
{
    inst.print_inst("arm_strexb");
    inst.check(27, 20, B(0001 1100));
    inst.check(11, 4, B(1111 1001));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rd = inst(15, 12);
        uint32_t rt = inst(3, 0);
        uint32_t d = rd;
        uint32_t t = rt;
        uint32_t n = rn;

        if((d == 15) || (t == 15) || (n == 15))
        {
            printb(d_inst, "arm_strexb error");
        }

        if((d == n) || (d == t))
        {
            printb(d_inst, "arm_strexb error 2");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n);

        if(exclusive_monitor_pass(address, 1))
        {
            bits _rt(rf.r_read(t), 8);

            if(!mem_a_write(address, 1, _rt))
            {
                return;
            }

            rf.r_write(d, 0);
        }

        else
        {
            rf.r_write(d, 1);
        }
    }
}

void armv7a::arm_ldrexb(armv7a_ir& inst)
{
    inst.print_inst("arm_ldrexb");
    inst.check(27, 20, B(0001 1101));
    inst.check(11, 0, B(1111 1001 1111));

    if(rf.condition_passed(inst.cond()))
    {
        uint32_t rn = inst(19, 16);
        uint32_t rt = inst(15, 12);
        uint32_t t = rt;
        uint32_t n = rn;

        if((t == 15) || (n == 15))
        {
            printb(d_inst, "arm_ldrexb error");
        }

        //ESO
        null_check_if_thumbee(n);
        uint32_t address = rf.r_read(n);
        set_exclusive_monitor(address, 1);
        bits data;

        if(!mem_a_read(&data, address, 1))
        {
            return;
        }

        rf.r_write(t, data.val);
    }
}

void armv7a::arm_strexh(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_strexh not implemented yet.");
}

void armv7a::arm_ldrexh(armv7a_ir& inst)
{
    printb(core_id, d_inst, "arm_ldrexh not implemented yet.");
}

