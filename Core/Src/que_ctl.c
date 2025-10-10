#include "que_ctl.h"
#include "app_filex.h"
#include "usart.h"
#include "sai.h"

que_create(sd_que, sd_que_stack, sizeof(sd_req_t), 4, &sd_que_sem);
que_create(sai1_tx_que, sai1_tx_que_stack, sizeof(sai_tx_req_t), 4, &sai1_tx_que_sem);
que_create(i2s1_tx_que, i2s1_tx_que_stack, SAI_PRE_BUF_SIZE, 8, NULL);

int __que_init(que_ctl_t *q)
{
    if (tx_semaphore_create(q->sem, "que_sem", 1) != TX_SUCCESS)
    {
        printfail("init sd sem fail");
        return -1;
    }

    return 1;
}

void que_init()
{
    __que_init(&sd_que);
    __que_init(&sai1_tx_que);
    printok("Que init end");
}

int que_size(que_ctl_t *q)
{
    int (*que_sem_wait)(TX_SEMAPHORE *) = NULL;
    int ret = 0;

    if (__get_IPSR() != 0)
    {
        que_sem_wait = sem_wait_isr; 
    }
    else
    {
        que_sem_wait = sem_wait;
    }

    if (que_sem_wait(q->sem) != TX_SUCCESS)
    {
        return -1;
    }
    ret = (q->front - q->rear) & (q->que_size - 1);
    sem_post(q->sem);
    return ret;
}

bool que_full(que_ctl_t *q)
{
    int (*que_sem_wait)(TX_SEMAPHORE *) = NULL;
    bool ret = false;

    if (__get_IPSR() != 0)
    {
        que_sem_wait = sem_wait_isr; 
    }
    else
    {
        que_sem_wait = sem_wait;
    }

    if (que_sem_wait(q->sem) != TX_SUCCESS)
    {
        return -1;
    }

    if (((q->rear + 1) & (q->que_size - 1)) == q->front)
    {
        ret = true;
    }
    sem_post(q->sem);
    return ret;
}

int que_flush(que_ctl_t *q)
{
    int (*que_sem_wait)(TX_SEMAPHORE *) = NULL;
    int ret = 0;

    if (__get_IPSR() != 0)
    {
        que_sem_wait = sem_wait_isr; 
    }
    else
    {
        que_sem_wait = sem_wait;
    }

    if (que_sem_wait(q->sem) != TX_SUCCESS)
    {
        return -1;
    }

    q->front = 0;
    q->rear = 0;

    sem_post(q->sem);
    return ret;
}

int que_push(que_ctl_t *q, void *buf, size_t buf_size)
{
    int (*que_sem_wait)(TX_SEMAPHORE *) = NULL;
    int ret = 0;

    if (__get_IPSR() != 0)
    {
        que_sem_wait = sem_wait_isr; 
    }
    else
    {
        que_sem_wait = sem_wait;
    }

    if (que_sem_wait(q->sem) != TX_SUCCESS)
    {
        return -1;
    }

    if (q->buf_size != buf_size)
    {
        printr("que size, setting : %d, your : %d", q->buf_size, buf_size);
        ret = -1;
        goto sem_out;
    }

    if (((q->rear + 1) & (q->que_size - 1)) == q->front)
    {
        printr("que is full");
        ret = -1;
        goto sem_out;
    }

    memcpy(&q->buf[q->rear * q->buf_size], buf, buf_size);
    q->rear = (q->rear + 1) & (q->que_size - 1);
    ret = buf_size;

sem_out:
    sem_post(q->sem);
    return ret;
}

int que_pop(que_ctl_t *q, void *buf, size_t buf_size)
{
    int (*que_sem_wait)(TX_SEMAPHORE *) = NULL;
    int ret = 0;

    if (__get_IPSR() != 0)
    {
        que_sem_wait = sem_wait_isr; 
    }
    else
    {
        que_sem_wait = sem_wait;
    }

    if (que_sem_wait(q->sem) != TX_SUCCESS)
    {
        return -1;
    }

    if (q->buf_size != buf_size)
    {
        printr("que size, setting : %d, your : %d", q->buf_size, buf_size);
        ret = -1;
        goto sem_out;
    }

    if (q->front == q->rear)
    {
        ret = -1;
        goto sem_out;
    }

    memcpy(buf, &q->buf[q->front * q->buf_size], buf_size);
    q->front = (q->front + 1) & (q->que_size - 1);
    ret = buf_size;
    
sem_out:
    sem_post(q->sem);
    return ret;
}