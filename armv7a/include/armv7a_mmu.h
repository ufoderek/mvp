#ifndef _ARMV7A_MMU_H_
#define _ARMV7A_MMU_H_


enum mem_arch
{
    MemArch_VMSA,
    MemArch_PMSA
};

enum mem_type
{
    MemType_Normal,
    MemType_Device,
    MemType_SO
};

class memory_attributes
{
    public:
        mem_type type;
        uint32_t inner_attrs;
        uint32_t outer_attrs;
        uint32_t inner_hints;
        uint32_t outer_hints;
        bool shareable;
        bool outer_shareable;
};

class full_address
{
    public:
        uint64_t physical_address;
        uint8_t physical_address_ext;
        bool ns;
        //uint32_t ns;
};

class address_descriptor
{
    public:
        memory_attributes mem_attrs;
        full_address p_address;
};

class permissions
{
    public:
        uint32_t ap;
        bool xn;
        bool pxn;
};

enum tlb_record_type
{
    TLBRecordType_SmallPage,
    TLBRecordType_LargePage,
    TLBRecordType_Section,
    TLBRecordType_Supersection,
    TLBRecordType_MMUDisabled
};

class tlb_record
{
    public:
        permissions perms;
        bool ng;
        uint32_t domain;
        bool contiguous_hint;
        uint32_t level;
        uint32_t block_size;
        //tlb_record_type type;
        address_descriptor addr_desc;
};

#endif
