#include "Util.h"
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <cstdio>
#include <errno.h>

unsigned long long Util::Decode64Long(const char* pData) {
	return *((unsigned long long *) pData);
}
const char* Util::Encode64Long(unsigned long long number, char* pData) {
	*(unsigned long long *)pData = number;
	return pData;
}

bool Util::DoesFileExist(const std::string& filefullname) {
    struct stat info;
    if (stat(filefullname.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFREG) != 0;
}

bool Util::DoesDirExist(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

bool Util::MakePathIfNotExist(const std::string& path) {
	if (DoesDirExist(path)) {
		return true;
	}

	int pos = 0;
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
    if (ret == 0)
        return true;

    switch (errno) {
		case ENOENT:
			// parent didn't exist, try to create it
			pos = path.find_last_of('/');
			if (pos == std::string::npos) {
				return false;
			}
			if (!MakePathIfNotExist(path.substr(0, pos))) {
				return false;
			}
			
			// now, try to create again
			return 0 == mkdir(path.c_str(), mode);

		case EEXIST:
			// done!
			return DoesDirExist(path);

		default:
			return false;
    }
}
