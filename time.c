
// linux/bsd - _GNU_SOURCE
#include <time.h>
int clock_gettime (clockid_t __clock_id, struct timespec *__tp);
int clock_nanosleep (clockid_t __clock_id, int __flags, const struct timespec *__req, struct timespec *__rem);

unsigned long long int get_time_us(void) {
	unsigned long long int time;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	time = (unsigned long long int)ts.tv_nsec / 1000;
	time += (unsigned long long int)ts.tv_sec * 1000000;
	
	return time;
}

static inline void sleep_ns(unsigned long long int ns) {
	struct timespec req;
	
	/* get seconds */
	req.tv_sec = 0;
	while(ns > 1000000000) {
		req.tv_sec++;
		ns -= 1000000000;
	}
	/* get nanoseconds */
	req.tv_nsec = ns;

	clock_nanosleep(CLOCK_MONOTONIC, 0, &req, NULL);
}

static inline void sleep_us(unsigned long long int us) {
	sleep_ns(us * 1000);
}
