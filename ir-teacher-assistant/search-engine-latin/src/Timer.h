/**
 * Timer.h
 * Created on: Mar 18, 2013
 *      
 * Timer class - simple timer with
 * millisecond precision
 *
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

typedef unsigned long long int ulli;

class Timer {
	ulli startTime;

	public:
	void start(void){
		struct timeval t;
		gettimeofday(&t, 0);
		startTime = t.tv_sec*(ulli)1000000 + (ulli)t.tv_usec;
	}
	/**
	 * Returns the time in microseconds
	 */
	ulli stop(void){
		ulli endTime;
		struct timeval t;
		gettimeofday(&t, 0);
		endTime = t.tv_sec*(ulli)1000000 + (ulli)t.tv_usec;
		return (endTime-startTime);
	}
	/**
	 * Returns the time in seconds with two decimals of precision
	 */
	float stopAndGetSeconds(void){
		return  ((stop()*100)/1000000)/100.0f;
	}
};

#endif

