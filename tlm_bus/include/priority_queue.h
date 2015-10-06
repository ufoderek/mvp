#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include <deque>
#include <debug_utils.h>

using namespace std;

class priority_queue
{
    public:
        void add_by_id_and_weight(unsigned int id, unsigned int weight);
        unsigned int get_id_by_order(unsigned int order);
        bool pop_front_push_back();
        unsigned int get_total_weight();

    private:
        deque<unsigned int>queue;
};

#endif
