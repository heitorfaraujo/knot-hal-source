#include <limits.h>
#include <errno.h>
#include "util.h"

#ifndef ARDUINO

int tline(unsigned long *ptime, unsigned long time_base /*=1*/)
{
	struct timeval tv;
	unsigned long time;
	/* Obtain the current time, expressed as seconds and microseconds */
	if (gettimeofday(&tv, NULL) != 0)
		return -errno;

	/* Computes to microseconds */
	time = tv.tv_sec * 1000000UL;
	time += tv.tv_usec;
	/* Converts microseconds to time base */
	time /= time_base;
	*ptime = time;

	return 0;
}

unsigned long tline_ms(void)
{

	unsigned long time;

	while (tline(&time, 1000UL) < 0)
		usleep(1);

	return time;
}

unsigned long tline_us(void)
{
	unsigned long time;

	while (tline(&time, 1UL) < 0)
		usleep(1);

	return time;
}

#endif

int tline_timeout(unsigned long time,  unsigned long last,  unsigned long timeout)
{
	/* Time overflow */
	if (time < last)
		/* Fit time overflow and compute time elapsed */
		time += (ULONG_MAX - last);
	else
		/* Compute time elapsed */
		time -= last;

	/* Timeout is flagged */
	return (time >= timeout);
}


int get_random_value(int interval, int ntime, int min)
{
	int value;

	value = (9973 * ~tline_us()) + ((value) % 701);
	srand((unsigned int)value);

	value = (rand() % interval) * ntime;
	if (value < 0) {
		value *= -1;
		value = (value % interval) * ntime;
	}
	if (value < min)
		value += min;

	return value;
}
