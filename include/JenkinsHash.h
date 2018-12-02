#pragma once
#include "Hash.h"

class JenkinsHash : Hash {
public:
	static long long hash(const char* key, int nbytes, int initval);
	static int rotateLeft(int i, int distance);
	static long long rot(long val, int pos);
};
