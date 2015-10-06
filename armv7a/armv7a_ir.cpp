#include <armv7a_ir.h>
#include <debug_utils.h>

armv7a_ir::armv7a_ir()
{
    set_name("noname");
    set(0, 32);
    set_pc(0);

    vm_id(0);
    hyp_mode(true);
}

armv7a_ir& armv7a_ir::operator=(const armv7a_ir& right)
{
    this->val = right.val;
    this->n = right.n;
    return *this;
}

void armv7a_ir::check(const uint32_t msb, const uint32_t lsb, const uint32_t pattern)
{
#ifdef INST_ASSERT
    if(!this->match(msb, lsb, pattern))
    {
        printb(d_inst_assert, "%s assert error", this->get_name());
    }
#endif
}

void armv7a_ir::check(const uint32_t n, const uint32_t bit)
{
#ifdef INST_ASSERT
    if(!this->match(n, bit))
    {
        printb(d_inst_assert, "%s assert error", this->get_name());
    }
#endif
}

char* armv7a_ir::get_name()
{
    return name;
}

void armv7a_ir::set_name(const char* name)
{
    strcpy(this->name, name);
}

uint32_t armv7a_ir::get_pc()
{
    return this->pc;
}

void armv7a_ir::set_pc(uint32_t pc)
{
    this->pc = pc;
}

void armv7a_ir::print_inst(const char* name)
{
    set_name(name);

    if(this->hyp_mode())
    {
        printd(d_inst, "0x%08X][0x%08X][%s", val, this->get_pc(), name);
        printd(d_inst_clean, "0x%08X][0x%08X][%s", val,  this->get_pc(), name);
    }
    else
    {
        switch(this->vm_id())
        {
            case 0:
                printd(d_inst_vm0, "0x%08X][0x%08X][%s", val, this->get_pc(), name);
                printd(d_inst_clean_vm0, "0x%08X][0x%08X][%s", val,  this->get_pc(), name);
                break;
            case 1:
                printd(d_inst_vm1, "0x%08X][0x%08X][%s", val, this->get_pc(), name);
                printd(d_inst_clean_vm1, "0x%08X][0x%08X][%s", val,  this->get_pc(), name);
                break;
            case 2:
                printd(d_inst_vm2, "0x%08X][0x%08X][%s", val, this->get_pc(), name);
                printd(d_inst_clean_vm2, "0x%08X][0x%08X][%s", val,  this->get_pc(), name);
                break;
            case 3:
                printd(d_inst_vm3, "0x%08X][0x%08X][%s", val, this->get_pc(), name);
                printd(d_inst_clean_vm3, "0x%08X][0x%08X][%s", val,  this->get_pc(), name);
                break;
        }
    }
}

uint32_t armv7a_ir::cond()
{
    return (*this)(31, 28);
}

void armv7a_ir::vm_id(uint32_t _vm_id)
{
    //updated by armv7a::arm_eret
    this->_vm_id = _vm_id;
}

uint32_t armv7a_ir::vm_id()
{
    return this->_vm_id;
}

void armv7a_ir::hyp_mode(bool _hyp_mode)
{
    //updated by armv7a::enter_hyp_mode and armv7a::arm_eret
    this->_hyp_mode = _hyp_mode;
}

bool armv7a_ir::hyp_mode()
{
    return this->_hyp_mode;
}

