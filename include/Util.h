#pragma once
#include <string>

class Util {
public:
	static constexpr int MAX_KEY_LEN = 1024;
	static constexpr int MAX_VALUE_LEN = 1024 * 1024;
	static unsigned long long Decode64Long(const char* pData);
	static const char* Encode64Long(unsigned long long number, char* pData);
	static bool DoesFileExist(const std::string& filefullname);
	static bool DoesDirExist(const std::string& path);
	static bool MakePathIfNotExist(const std::string& path);
};

