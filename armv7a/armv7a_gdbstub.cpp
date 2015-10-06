#include <armv7a.h>

/* read register by GDB */
void armv7a::regRead(uint32_t* data, uint16_t index)
{
    if(index == 16)
    {
        *data = rf.cpsr();
    }

    else if(index == 15)
    {
        *data = rf.current_pc();
    }

    else if(index < 15)
    {
        *data = rf.r_read(index);
        printd(d_armv7a_gdbstub, "regRead R%d:%X", index, *data);
    }

    else
    {
        *data = 0;
        //printm(core_id, d_armv7a_gdbstub, "no such register R%d", index);
    }
}

/* write register by external modules */
void armv7a::regWrite(uint32_t data, uint16_t index)
{
    rf.r_write(index, data);
}

/* read memory by external modules */
void armv7a::memRead(uint32_t* data, uint32_t vir, int size)
{
    uint32_t phy = 0;
    bool cache = true;

    if(cp15.read(SCTLR_M))
    {
        bits _data(0, 32);
        bits _tmp(0, 8);
        address_descriptor mem_addr_desc;
        translate_address(&mem_addr_desc, vir, true, false, 4, false, true);
        _mem_read(&_data, mem_addr_desc, size);
        *data = _data.val;
        return;
    }
    else
    {
        phy = vir;
        bool success;
        success = bus_read(data, phy, size);

        if(!success)
        {
            *data = 0;
        }
    }

    printd(core_id, d_armv7a_gdbstub, "memRead %X, data = %X", phy, *data);
}

/* write memory by external modules */
void armv7a::memWrite(uint32_t data, uint32_t vir, int size)
{
    /* NOTE: don't use data_write() function directly, or it probably brings some bizarre side effects */
    uint32_t phy = 0;
    bool cache = true;

    if(cp15.read(SCTLR_M))
    {
        printb(d_armv7a_gdbstub, "memWrite mmu on");
        //vir2phy(vir, &phy, CPSR_MODE(rf.cpsr), true, &cache);
    }

    else
    {
        phy = vir;
    }

    bus_write(data, phy, size);
}

/* insert break point */
void armv7a::bkptInsert(uint32_t addr)
{
    printd(d_armv7a_gdbstub, "bkptInsert %X", addr);

    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if(break_point[i].exist == false)
        {
            break_point[i].exist = true;
            break_point[i].addr = addr;
            break;
        }
    }
}

/* remove break point */
void armv7a::bkptRemove(uint32_t addr)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((break_point[i].exist == true) && (break_point[i].addr == addr))
        {
            break_point[i].exist = false;
        }
    }
}

/* remove all break points */
void armv7a::bkptRemove(void)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        break_point[i].exist = false;
    }
}

/* insert both rWatch and wWatch point */
void armv7a::awatchInsert(uint32_t addr)
{
    rwatchInsert(addr);
    wwatchInsert(addr);
}

/* insert rWatch point */
void armv7a::rwatchInsert(uint32_t addr)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if(rwatch_point[i].exist == false)
        {
            rwatch_point[i].exist = true;
            rwatch_point[i].addr = addr;
            break;
        }
    }
}

/* insert wWatch point */
void armv7a::wwatchInsert(uint32_t addr)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if(wwatch_point[i].exist == false)
        {
            wwatch_point[i].exist = true;
            wwatch_point[i].addr = addr;
            break;
        }
    }
}

/* remove both rWatch and wWatch point */
void armv7a::awatchRemove(uint32_t addr)
{
    rwatchRemove(addr);
    wwatchRemove(addr);
}

/* remove wWatch point */
void armv7a::rwatchRemove(uint32_t addr)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((rwatch_point[i].exist == true) && (rwatch_point[i].addr == addr))
        {
            rwatch_point[i].exist = false;
        }
    }
}

/* remove wWatch point */
void armv7a::wwatchRemove(uint32_t addr)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((wwatch_point[i].exist == true) && (wwatch_point[i].addr == addr))
        {
            wwatch_point[i].exist = false;
        }
    }
}

/* remove all watch points */
void armv7a::watchRemove(void)
{
    for(int i = 0; i < SIZE_BKPT; i++)
    {
        rwatch_point[i].exist = false;
        wwatch_point[i].exist = false;
    }
}


/* check the break point reaching */
bool armv7a::bkptCheck(uint32_t addr)
{
    bool result = false;

    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((break_point[i].exist) && (break_point[i].addr == addr))
        {
            result = true;
            break;
        }
    }

    return result;
}

/* check the read watch point reaching */
bool armv7a::rwatchCheck(uint32_t addr)
{
    bool result = false;

    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((rwatch_point[i].exist) && (rwatch_point[i].addr == addr))
        {
            result = true;
            break;
        }
    }

    return result;
}

/* check the write watch point reaching */
bool armv7a::wwatchCheck(uint32_t addr)
{
    bool result = false;

    for(int i = 0; i < SIZE_BKPT; i++)
    {
        if((wwatch_point[i].exist) && (wwatch_point[i].addr == addr))
        {
            result = true;
            break;
        }
    }

    return result;
}

