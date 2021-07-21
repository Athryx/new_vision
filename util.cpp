#include "util.h"
#include <sys/time.h>

// gets microseconds since unix epoch
long get_usec()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv.tv_sec + 1000000 * tv.tv_usec;
}

void time(const char *op_name, std::function<void ()> op)
{
	long old_usec = get_usec();
	op();
	long new_usec = get_usec();
	printf("%s elapsed time: %ld", op_name, new_usec - old_usec);
}
