#ifndef KENNY_INCLUDE
#define KENNY_INCLUDE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>


long double get_time();
long double get_second_time();
long double get_microsecond_time();

template<typename T> int sgn(T val) {
    // https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
    // If the first is true, then we return 1.
    // If the second is true, then we return -1.
    // If neither is true, then we return 0.
    return (T(0) < val) - (val < T(0));
}

long double get_second_time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec;
}

long double get_microsecond_time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_usec;
}

long double get_time(){
    long double retval;
    retval = get_second_time();
    retval = retval * 1000000;
    retval += get_microsecond_time();
    return retval;
}





#endif /*KENNY_INCLUDE */
