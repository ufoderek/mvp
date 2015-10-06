#include <armv7a_rf.h>
#include <bit_opt_v2.h>
#include <debug_utils.h>
#include <armv7a.h>

armv7a_rf::armv7a_rf()
{
    for(int i = 0; i < RName_count; i++)
    {
        _r[i] = 0;
    }

    _cpsr = 0x400001d3;
    _spsr[usr] = 0x400001d0; //USR & SYS
    _spsr[svc] = 0x400001d3; //SVC
    _spsr[abt] = 0x400001d7; //ABORT
    _spsr[und] = 0x400001db; //UNDEF
    _spsr[irq] = 0x400001d2; //IRQ
    _spsr[fiq] = 0x400001d1; //FIQ
    _spsr[mon] = 0x400001d6; //MON
}

armv7a_rf::~armv7a_rf()
{
}

bool armv7a_rf::bad_mode(uint32_t mode)
{
#ifdef CPU_ASSERT
    switch(mode)
    {
        case B(10000):
        case B(10001):
        case B(10010):
        case B(10011):
            return false;
        case B(10110):
            printb(d_armv7a_rf, "bad mode: Monitor Mode");
        case B(10111):
            return false;
        case B(11010):
            printd(d_armv7a_vm, "enter HYP mode");
        case B(11011):
        case B(11111):
            return false;
        default:
            printb(d_armv7a_rf, "bad mode");
    }
#else
    return false;
#endif
}

bool armv7a_rf::current_mode_is_hyp()
{
#ifdef CPU_ASSERT
    if(bad_mode(cpsr_M()))
    {
        printb(d_armv7a_rf, "current_mode_is_not_hyp unpredictable");
    }
#endif

    if(cpsr_M() == B(11010))
    {
        return true;
    }

    return false;
}

bool armv7a_rf::current_mode_is_not_user()
{
#ifdef CPU_ASSERT
    if(bad_mode(cpsr_M()))
    {
        printb(d_armv7a_rf, "current_mode_is_not_user unpredictable");
    }
#endif

    if(cpsr_M() == B(10000))
    {
        return false;
    }

    return true;
}

bool armv7a_rf::current_mode_is_user_or_system()
{
#ifdef CPU_ASSERT
    if(bad_mode(cpsr_M()))
    {
        printb(d_armv7a_rf, "current_mode_is_user_or_system error");
    }
#endif

    if(cpsr_M() == B(10000))
    {
        return true;
    }
    else if(cpsr_M() == B(11111))
    {
        return true;
    }

    return false;
}

r_bank armv7a_rf::r_bank_select(uint32_t mode)
{
#ifdef CPU_ASSERT
    if(bad_mode(mode))
    {
        printb(d_armv7a_rf, "r_bank_select bad mode");
    }
#endif
    switch(mode)
    {
        case B(10000):
            return usr;
        case B(10001):
            return fiq;
        case B(10010):
            return irq;
        case B(10011):
            return svc;
        case B(10110):
            return mon;
        case B(10111):
            return abt;
        case B(11010):
            return hyp;
        case B(11011):
            return und;
        case B(11111):
            return usr;      //System Mode use User Mode's registers
        default:
            printb(d_armv7a_rf, "r_bank_select error");
    }
}

r_name armv7a_rf::look_up_r_name(uint32_t n, uint32_t mode)
{
#ifdef CPU_ASSERT
    if(!((n >= 0) && (n <= 14)))
    {
        printb(d_armv7a_rf, "look_up_name assert error");
    }
#endif

    switch(n)
    {
        case 0:
            return RName_0usr;
            break;
        case 1:
            return RName_1usr;
            break;
        case 2:
            return RName_2usr;
            break;
        case 3:
            return RName_3usr;
            break;
        case 4:
            return RName_4usr;
            break;
        case 5:
            return RName_5usr;
            break;
        case 6:
            return RName_6usr;
            break;
        case 7:
            return RName_7usr;
            break;
        case 8:
            if(r_bank_select(mode) == fiq)
            {
                return RName_8fiq;
            }
            else
            {
                return RName_8usr;
            }
            break;
        case 9:
            if(r_bank_select(mode) == fiq)
            {
                return RName_9fiq;
            }
            else
            {
                return RName_9usr;
            }
            break;
        case 10:
            if(r_bank_select(mode) == fiq)
            {
                return RName_10fiq;
            }
            else
            {
                return RName_10usr;
            }
            break;
        case 11:
            if(r_bank_select(mode) == fiq)
            {
                return RName_11fiq;
            }
            else
            {
                return RName_11usr;
            }
            break;
        case 12:
            if(r_bank_select(mode) == fiq)
            {
                return RName_12fiq;
            }
            else
            {
                return RName_12usr;
            }
            break;
        case 13:
            switch(r_bank_select(mode))
            {
                case usr:
                    return RName_SPusr;
                case svc:
                    return RName_SPsvc;
                case mon:
                    return RName_SPmon;
                case abt:
                    return RName_SPabt;
                case hyp:
                    return RName_SPhyp;
                case und:
                    return RName_SPund;
                case irq:
                    return RName_SPirq;
                case fiq:
                    return RName_SPfiq;
                default:
                    printb(d_armv7a_rf, "look_up_r_name error");
            }
            break;
        case 14:
            switch(r_bank_select(mode))
            {
                case usr:
                    return RName_LRusr;
                case svc:
                    return RName_LRsvc;
                case mon:
                    return RName_LRmon;
                case abt:
                    return RName_LRabt;
                case hyp:
                    return RName_LRusr;
                case und:
                    return RName_LRund;
                case irq:
                    return RName_LRirq;
                case fiq:
                    return RName_LRfiq;
                default:
                    printb(d_armv7a_rf, "look_up_r_name error 2");
            }
            break;
        default:
            printb(d_armv7a_rf, "look_up_name error 3");
            break;
    }
}

uint32_t armv7a_rf::r_mode_read(uint32_t n, uint32_t mode)
{
    //igore secure check
#ifdef CPU_ASSERT
    if(!((n >= 0) && (n <= 14)))
    {
        printb(d_armv7a_rf, "r_mode_read error");
    }

    if(!armv7a::is_secure() && (mode == B(10110)))
    {
        printb(d_armv7a_rf, "r_mode_read error 2");
    }

    /*
    if(!armv7a::is_secure() && (mode==B(10001)) && (cp15.nsacr_rfr())){
        printb(d_armv7a_rf, "r_mode_read error 3");
    }
    */
#endif

    return _r[look_up_r_name(n, mode)];
}

void armv7a_rf::r_mode_write(uint32_t n, uint32_t mode, uint32_t value)
{
#ifdef CPU_ASSERT
    if(!((n >= 0) && (n <= 14)))
    {
        printb(d_armv7a_rf, "r_mode_write error");
    }

    if(!armv7a::is_secure() && (mode == B(10110)))
    {
        printb(d_armv7a_rf, "r_mode_write error 2");
    }

    /*
    if(!armv7a::is_secure() && (mode==B(10001)) && (cp15.nsacr_rfr())){
        printb(d_armv7a_rf, "r_mode_write error 3");
    }
    */

    if((n == 13) && (get_field(value, 1, 0) != B(00)) && (current_inst_set() != InstSet_ARM))
    {
        printb(d_armv7a_rf, "r_mode_write error 4");
    }
#endif

    _r[look_up_r_name(n, mode)] = value;
}

uint32_t armv7a_rf::r_read(uint32_t n)
{
#ifdef CPU_ASSERT
    if(!((n >= 0) && (n <= 15)))
    {
        printb(d_armv7a_rf, "r_read error");
    }
#endif
    if(n == 15)
    {
        uint32_t offset;
        offset = (current_inst_set() == InstSet_ARM) ? 8 : 4;
        //ARM Ref Manual said that in programmer's view, this PC value is "current instruction's PC + 8"
        //but since we have incremented PC (+4) after instruction fecth, we let offset = 4 here to accomplish the ARM spec.
        offset = 4;
        return _r[RName_PC] + offset;
    }
    else
    {
        uint32_t value;
        value = r_mode_read(n, cpsr_M());
        return value;
    }
}

void armv7a_rf::r_write(uint32_t n, uint32_t value)
{
#ifdef CPU_ASSERT
    if(!((n >= 0) && (n <= 15)))
    {
        printb(d_armv7a_rf, "r_write error");
    }
#endif

    r_mode_write(n, cpsr_M(), value);
}

void armv7a_rf::reset_pc(uint32_t addr)
{
    _r[RName_PC] = addr;
}

void armv7a_rf::incre_pc()
{
    _r[RName_PC] += 4;
}

uint32_t armv7a_rf::pc()
{
    return r_read(15);
}

uint32_t armv7a_rf::current_pc()
{
    return _r[RName_PC];
}

void armv7a_rf::alu_write_pc(uint32_t addr)
{
#ifdef CPU_ASSERT
    if((armv7a::arch_version() >= 7) && (current_inst_set() == InstSet_ARM))
    {
        bx_write_pc(addr);
    }
    else
    {
        printb(d_armv7a_rf, "alu_write_pc error");
    }
#else
    bx_write_pc(addr);
#endif
}

void armv7a_rf::branch_to(uint32_t addr)
{
    _r[RName_PC] = addr;
}

void armv7a_rf::branch_write_pc(uint32_t address)
{
#ifdef CPU_ASSERT
    if(current_inst_set() == InstSet_ARM)
    {
        if((armv7a::arch_version() < 6) && (get_field(address, 1, 0) != B(00)))
        {
            printb(d_armv7a_rf, "branch_write_pc unpredictable");
        }
        branch_to(address & mask(31, 2));
    }
    else
    {
        printb(d_armv7a_rf, "branch_write_pc error");
    }
#else
    branch_to(address & mask(31, 2));
#endif
}

void armv7a_rf::bx_write_pc(uint32_t address)
{
#ifdef CPU_ASSERT
    if(current_inst_set() == InstSet_ThumbEE)
    {
        printb(d_armv7a_rf, "bx_write_pc thumbee");
    }
    else
    {
        if(get_bit(address, 0) == 1)
        {
            printb(d_armv7a_rf, "bx_write_pc thumb");
        }
        else if(get_bit(address, 1) == 0)
        {
            branch_to(address);
        }
        else
        {
            printb(d_armv7a_rf, "bx_write_pc unpredictable");
        }
    }
#else
    branch_to(address & mask(31, 2));
#endif
}

bool armv7a_rf::condition_passed(uint32_t cond)
{
    bool result;

    switch(get_field(cond, 3, 1))
    {
        case B(000):
            result = (cpsr_Z() == 1);
            break;
        case B(001):
            result = (cpsr_C() == 1);
            break;
        case B(010):
            result = (cpsr_N() == 1);
            break;
        case B(011):
            result = (cpsr_V() == 1);
            break;
        case B(100):
            result = (cpsr_C() == 1) && (cpsr_Z() == 0);
            break;
        case B(101):
            result = (cpsr_N() == cpsr_V());
            break;
        case B(110):
            result = (cpsr_N() == cpsr_V()) && (cpsr_Z() == 0);
            break;
        case B(111):
            result = true;
            break;
    }

    if((get_bit(cond, 0) == 1) && (cond != B(1111)))
    {
        result = !result;
    }

    return result;
}

inst_set armv7a_rf::current_inst_set()
{
    uint32_t iset_state = (cpsr_J() << 1) | cpsr_T();

    switch(iset_state)
    {
        case B(00):
            return InstSet_ARM;
            break;
            /*
               case B(01) :return InstSet_Thumb;
               case B(10) :return InstSet_Jazelle;
               case B(11): return InstSet_ThumbEE;
             */
        default:
            printb(d_armv7a_rf, "current_inst_set error");
            break;
    }
}

void armv7a_rf::load_write_pc(uint32_t address)
{
#ifdef CPU_ASSERT
    if(armv7a::arch_version() >= 5)
    {
        bx_write_pc(address);
    }
    else
    {
        branch_write_pc(address);
    }
#else
    bx_write_pc(address);
#endif
}

uint32_t armv7a_rf::pc_store_value()
{
    return pc();
}

uint32_t armv7a_rf::apsr()
{
    return _cpsr;
}

uint32_t armv7a_rf::cpsr()
{
    return _cpsr;
}

uint32_t armv7a_rf::cpsr_M()
{
    return get_field(_cpsr, 4, 0);
}

uint32_t armv7a_rf::cpsr_T()
{
    return get_bit(_cpsr, 5);
}

uint32_t armv7a_rf::cpsr_F()
{
    return get_bit(_cpsr, 6);
}

uint32_t armv7a_rf::cpsr_I()
{
    return get_bit(_cpsr, 7);
}

uint32_t armv7a_rf::cpsr_A()
{
    return get_bit(_cpsr, 8);
}

uint32_t armv7a_rf::cpsr_E()
{
    return get_bit(_cpsr, 9);
}

uint32_t armv7a_rf::cpsr_IT()
{
    return (get_field(_cpsr, 26, 25) << 6) | get_field(_cpsr, 15, 10);
}

uint32_t armv7a_rf::cpsr_GE()
{
    return get_field(_cpsr, 19, 16);
}

uint32_t armv7a_rf::cpsr_J()
{
    return get_bit(_cpsr, 24);
}

uint32_t armv7a_rf::cpsr_Q()
{
    return get_bit(_cpsr, 27);
}

uint32_t armv7a_rf::cpsr_V()
{
    return get_bit(_cpsr, 28);
}

uint32_t armv7a_rf::cpsr_C()
{
    return get_bit(_cpsr, 29);
}

uint32_t armv7a_rf::cpsr_Z()
{
    return get_bit(_cpsr, 30);
}

uint32_t armv7a_rf::cpsr_N()
{
    return get_bit(_cpsr, 31);
}

void armv7a_rf::cpsr(uint32_t value)
{
#ifndef RELEASE
    uint32_t old_mode = get_field(_cpsr, 4, 0);
    uint32_t new_mode = get_field(value, 4, 0);

    if(old_mode != new_mode)
    {
        printd(d_armv7a_rf, "change to mode: 0x%X, pc=0x%X", new_mode, current_pc());
    }
#endif

    _cpsr = value;
}

void armv7a_rf::cpsr_M(uint32_t mode)
{
    printd(d_armv7a_rf, "change to mode: 0x%X, pc=0x%X", mode, current_pc());
    set_field(&_cpsr, 4, 0, mode);
}

void armv7a_rf::cpsr_T(bool b)
{
    set_bit(&_cpsr, 5, b);
}

void armv7a_rf::cpsr_F(bool b)
{
    set_bit(&_cpsr, 6, b);
}

void armv7a_rf::cpsr_I(bool b)
{
    if(cpsr_I() != b)
    {
        printd(d_armv7a_rf, "cpsr_I %s", b ? "on" : "off");
    }

    set_bit(&_cpsr, 7, b);
}

void armv7a_rf::cpsr_A(bool b)
{
    set_bit(&_cpsr, 8, b);
}

void armv7a_rf::cpsr_E(bool b)
{
    set_bit(&_cpsr, 9, b);
}

void armv7a_rf::cpsr_IT(uint32_t it)
{
    set_field(&_cpsr, 15, 10, get_field(it, 7, 2));
    set_field(&_cpsr, 26, 25, get_field(it, 1, 0));
}

void armv7a_rf::cpsr_GE(uint32_t ge)
{
    set_field(&_cpsr, 19, 16, ge);
}

void armv7a_rf::cpsr_J(bool b)
{
    set_bit(&_cpsr, 24, b);
}

void armv7a_rf::cpsr_Q(bool b)
{
    set_bit(&_cpsr, 27, b);
}

void armv7a_rf::cpsr_V(bool b)
{
    set_bit(&_cpsr, 28, b);
}

void armv7a_rf::cpsr_C(bool b)
{
    set_bit(&_cpsr, 29, b);
}

void armv7a_rf::cpsr_Z(bool b)
{
    set_bit(&_cpsr, 30, b);
}

void armv7a_rf::cpsr_N(bool b)
{
    set_bit(&_cpsr, 31, b);
}

uint32_t armv7a_rf::spsr()
{
#ifdef CPU_ASSERT
    if(bad_mode(cpsr_M()))
    {
        printb(d_armv7a_rf, "spsr non-assign unpredictable");
    }
#endif
    switch(cpsr_M())
    {
        case B(10001):
            return _spsr[fiq];
            break;
        case B(10010):
            return _spsr[irq];
            break;
        case B(10011):
            return _spsr[svc];
            break;
        case B(10110):
            return _spsr[mon];
            break;
        case B(10111):
            return _spsr[abt];
            break;
        case B(11010):
            return _spsr[hyp];
            break;
        case B(11011):
            return _spsr[und];
            break;
        default:
            printb(d_armv7a_rf, "spsr non-assign fault");
            break;
    }
}

void armv7a_rf::spsr(uint32_t value)
{
#ifdef CPU_ASSERT
    if(bad_mode(cpsr_M()))
    {
        printb(d_armv7a_rf, "spsr assign unpredictable");
    }
#endif
    switch(cpsr_M())
    {
        case B(10001):
            _spsr[fiq] = value;
            break;
        case B(10010):
            _spsr[irq] = value;
            break;
        case B(10011):
            _spsr[svc] = value;
            break;
        case B(10110):
            _spsr[mon] = value;
            break;
        case B(10111):
            _spsr[abt] = value;
            break;
        case B(11010):
            _spsr[hyp] = value;
            break;
        case B(11011):
            _spsr[und] = value;
            break;
        default:
            printb(d_armv7a_rf, "spsr assign fault");
            break;
    }
}

uint32_t armv7a_rf::spsr_bank(r_bank b)
{
    return _spsr[b];
}

void armv7a_rf::spsr_bank(r_bank b, uint32_t value)
{
    _spsr[b] = value;
}

uint32_t armv7a_rf::elr_hyp()
{
    return _r[RName_ELRhyp];
}

void armv7a_rf::elr_hyp(uint32_t value)
{
    _r[RName_ELRhyp] = value;
}

