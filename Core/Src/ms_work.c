#include "ms_work.h"
#include "tim.h"

void ms1_work()
{
    tim_pwm_run_led();
}

void ms_proc()
{
    ms1_work();
}

