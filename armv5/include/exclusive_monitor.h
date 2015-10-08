#ifndef _EXCLUSIVE_MONITOR_H_
#include <stdint.h>
#include <iostream>
#include <set>
using namespace std;

class exclusive_monitor
{
    public:
        exclusive_monitor();
        ~exclusive_monitor();

        void mark_exclusive(uint32_t addr, unsigned int core_id, unsigned int size);
        bool is_exclusive(uint32_t addr, unsigned int core_id, unsigned int size);
        void clear_by_addr(uint32_t addr, unsigned int size);
        void clear_exclusive(unsigned int core_id);

    private:
        set<uint64_t> marked_pairs;
        set<uint64_t>::iterator it;
};

#endif
