#include "timer.h"
#include <time.h>


void Timer::start( void ){
    clock_gettime( CLOCK_MONOTONIC, &t1);
    isStarted = true;
}

// stop the timer.
void Timer::stop( void ){
    isStarted = false;
}

// return true if this timer was starte more than 'sec' ago
// The timer will be started if not already running
bool Timer::isElapsed( int sec ){
    timespec t;
    if( isStarted ){
        clock_gettime( CLOCK_MONOTONIC, &t2);
        t = diff( t1, t2);
        if( t.tv_sec >= sec ){
            return true;
        }
    }
    else {
        // timer was not started, so we start it now
        start();
    }
    return false;
}

// calculate the difference between two times and return it
timespec Timer::diff(timespec start, timespec end) {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}
