#include <stdint.h>
#include <debug_utils.h>

class decoder
{
    public:
        decoder();
        ~decoder();

        bool add_mapping(unsigned int slave_id, uint32_t base_address, uint32_t mapping_size);
        bool decode(unsigned int& slave_id, uint32_t address);

    private:
        static const uint32_t shift = 10;
        static const uint32_t map_size = 0xFFFFFFFF >> shift;
        char* map;
};

