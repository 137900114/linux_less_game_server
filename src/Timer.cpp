#include "Timer.h"
#include <sys/time.h>

void Timer::Initialize(){
    accumulatedTime = 0.;
    gettimeofday(&lasttval,NULL);
}

void Timer::Update(){
    timeval currt;
    gettimeofday(&currt,NULL);
    deltaTime = currt.tv_sec - lasttval.tv_sec +
        (float)(currt.tv_usec - lasttval.tv_usec) * 1e-6;
    accumulatedTime += deltaTime;
    lasttval = currt;
}