#include "status.h"
#include "usart.h"
#if (status_integer_use)
int_status_t int_status[STATUS_INTEGER_MAX] = {
    [STATUS_INTEGER_DMESG].status = STATUS_DMESG_ON,
};
#endif
#if (status_string_use)
string_status_t string_status = {0, };
#endif

void status_init()
{
    printok("STATUS : init stm32 status");
}

#if (status_integer_use)
int status_get_int(STATUS_INTEGER val)
{
    int ret = 0;

    sem_wait(int_status[val].sem);
    ret = int_status[val].status;
    sem_post(int_status[val].sem);
    return ret;
}

void status_set_int(STATUS_INTEGER val, int set)
{
    sem_wait(int_status[val].sem);
    int_status[val].status = set;
    sem_post(int_status[val].sem);
}
#endif