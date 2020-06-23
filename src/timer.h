#include <time.h>

class Timer {
    public:
    Timer( void ){};

    // start timer by saving the current time.
    void start( void );

    // stop the timer.
    void stop( void );

    // return true if this timer was starte more than 'sec' ago
    // The timer will be started if not already running
    bool isElapsed( int sec );

    private:
    timespec t1, t2;
    bool isStarted = false;

    // calculate the difference between two times and return it
    timespec diff(timespec start, timespec end);
};
