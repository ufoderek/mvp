#include <priority_queue.h>

void priority_queue::add_by_id_and_weight(unsigned int id, unsigned int weight)
{
    int i;

    for(i = 0; i < weight; i++)
    {
        queue.push_back(id);
    }
}

unsigned int priority_queue::get_id_by_order(unsigned int order)
{
    return queue[order];
}

bool priority_queue::pop_front_push_back()
{
    if(get_total_weight() > 0)
    {
        queue.push_back(queue.front());
        queue.pop_front();
        return true;
    }

    return false;
}

unsigned int priority_queue::get_total_weight()
{
    return queue.size();
}


