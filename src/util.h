#pragma once

#include <functional>
#include <stdio.h>

long get_usec();

template<typename T>
T time(const char *op_name, std::function<T ()> op)
{
	long old_usec = get_usec();
	auto ret = op();
	long new_usec = get_usec();
	printf("%s elapsed time: %ld", op_name, new_usec - old_usec);
	return ret;
}

void time(const char *op_name, std::function<void ()> op);
