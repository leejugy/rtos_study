#ifndef __QUE_CTL_H__
#define __QUE_CTL_H__

#include "app_threadx.h"
#include "main.h"

typedef struct
{
    uint8_t *buf;
    size_t buf_size;
    size_t que_size;
    TX_SEMAPHORE *sem;
    int front;
    int rear;
}que_ctl_t;

#define que_create(que_name, stack_name, buffer_size, queue_size, semaphore) \
uint8_t stack_name[buffer_size * queue_size] = {0, }; \
que_ctl_t que_name = {\
    .buf = stack_name, \
    .buf_size = buffer_size, \
    .que_size = queue_size, \
    .sem = semaphore \
}

extern que_ctl_t sd_que;
extern que_ctl_t sai1_tx_que;
extern que_ctl_t i2s1_tx_que;

void que_init();
int que_flush(que_ctl_t *q);
int que_size(que_ctl_t *q);
bool que_full(que_ctl_t *q);
int que_push(que_ctl_t *q, void *buf, size_t buf_size);
int que_pop(que_ctl_t *q, void *buf, size_t buf_size);
#endif