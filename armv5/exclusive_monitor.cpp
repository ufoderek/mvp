#include <map>
#include <exclusive_monitor.h>
#include <debug_utils.h>

exclusive_monitor::exclusive_monitor()
{
}

exclusive_monitor::~exclusive_monitor()
{
}

void exclusive_monitor::mark_exclusive(uint32_t addr, unsigned int core_id, unsigned int size)
{
    uint64_t pair = (uint64_t)addr | ((uint64_t)(core_id) << 32);

    for (int i = 0; i < size; i++) {
        marked_pairs.insert(pair + i);
    }
}

bool exclusive_monitor::is_exclusive(uint32_t addr, unsigned int core_id, unsigned int size)
{
    uint64_t pair = (uint64_t)addr | ((uint64_t)(core_id) << 32);

    for (int i = 0; i < size; i++) {
        if (marked_pairs.find(pair + i) != marked_pairs.end()) {
            return true;
        }
    }

    return false;
}

void exclusive_monitor::clear_by_addr(uint32_t addr, unsigned int size)
{
    for (int i = 0; i < size; i++) {
        it = marked_pairs.begin();

        while (it != marked_pairs.end()) {
            if (((*it) & 0xFFFFFFFF) == (addr + i)) {
                marked_pairs.erase(it);
            }

            else {
                ++it;
            }
        }
    }
}

void exclusive_monitor::clear_exclusive(unsigned int core_id)
{
    it = marked_pairs.begin();

    while (it != marked_pairs.end()) {
        if (((*it) >> 32) == core_id) {
            marked_pairs.erase(it);
        }

        else {
            ++it;
        }
    }
}

