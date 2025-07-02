#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdint.h>

#include <string>

enum HKD_FileStatus {
    HKD_FILE_SUCCESS,
    HKD_ERROR_READ_FILE,
    HKD_ERROR_WRITE_FILE,
    HKD_ERROR_NO_FILE
};

struct HKD_File {
    uint8_t* data;
    uint32_t size;
};

std::string    hkd_GetExePath(void);
HKD_FileStatus hkd_read_file(char const* filename, HKD_File* out_File);
HKD_FileStatus hkd_write_file(char const* filename, void* data, size_t sizeElment, size_t numElements);
HKD_FileStatus hkd_destroy_file(HKD_File* file);

#endif
