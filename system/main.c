#include <xinu.h>
#include <stdio.h>



//TIME_SLICE            2
//TIME_ALLOTMENT        400
//PRIORITY_BOOST_PERIOD 2200

int main() {
    pid32 prA, prB, prC, prD;
    int i;

    prA = create_user_proc(burst_execution, 1024, 40, "burst_execution", 3, 1, 40, 0);
    prB = create_user_proc(burst_execution, 1024, 42, "burst_execution", 3, 3, 14, 6);
    resume(prA);
    resume(prB);
    sleepms(70);
    for(i=0;i<1000000;i++);
    prC = create_user_proc(burst_execution, 1024, 42, "burst_execution", 3, 1, 42, 0);
    prD = create_user_proc(burst_execution, 1024, 32, "burst_execution", 3, 4, 8, 8);
    resume(prC);
    resume(prD);
    sleepms(200);
    for(i=0;i<1000000;i++);

    return OK;
}


