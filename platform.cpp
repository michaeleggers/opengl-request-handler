#include "platform.h"

#include <stdint.h>
#include <string>

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

std::string hkd_GetExePath(void)
{
    char  out_buffer[ 256 ];
    int   buffer_size = 256;
    DWORD len         = GetModuleFileNameA(NULL, out_buffer, buffer_size);
    if ( !len )
    {
        DWORD error = GetLastError();
        char  errorMsgBuf[ 256 ];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      error,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      errorMsgBuf,
                      (sizeof(errorMsgBuf) / sizeof(char)),
                      NULL);

        printf("%s\n", errorMsgBuf);
    }

    // strip actual name of the .exe
    char* last = out_buffer + len;
    while ( *last != '\\' )
    {
        *last-- = '\0';
    }

    return std::string(out_buffer);
}

#elif __APPLE__

#include <mach-o/dyld.h>

std::string hkd_GetExePath(void)
{
    char     out_buffer[ 256 ];
    uint32_t buffer_size = 256;

    int error = _NSGetExecutablePath(out_buffer, &buffer_size);
    if ( error )
    {
        // TOOO: handle error
    }
    int   len   = strlen(out_buffer);
    char* slash = out_buffer + len + 1;
    while ( len >= 0 && *slash != '/' )
    {
        slash--;
        len--;
    }
    out_buffer[ len + 1 ] = '\0';

    return std::string(out_buffer);
}

#elif __linux__

#include <SDL3/SDL.h>
#include <string.h>

std::string hkd_GetExePath(void)
{
    const char* basePath = SDL_GetBasePath();

    return std::string(basePath);
}
#endif

HKD_FileStatus hkd_read_file(char const* filename, HKD_File* out_File)
{
    FILE* file = 0;
    file       = fopen(filename, "rb");
    if ( file == 0 )
    {
        printf("Failed to open file: %s\n", filename);
        return HKD_ERROR_READ_FILE;
    }
    fseek(file, 0L, SEEK_END);
    out_File->size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    out_File->data = (uint8_t*)malloc(out_File->size + 1);
    fread(out_File->data, sizeof(uint8_t), out_File->size, file);
    fclose(file);

    out_File->data[ out_File->size ] = '\0';

    return HKD_FILE_SUCCESS;
}

HKD_FileStatus hkd_write_file(char const* filename, void* data, size_t sizeElement, size_t numElements)
{
    FILE* pFile = 0;
    pFile       = fopen(filename, "wb");
    if ( pFile == 0 )
    { // early out here.
        printf("Failed to open file for writing: %s\n", filename);
        return HKD_ERROR_WRITE_FILE;
    }

    HKD_FileStatus result;
    size_t         retFwriteOp = fwrite(data, sizeElement, numElements, pFile);
    if ( retFwriteOp != numElements )
    {
        fprintf(stderr, "fread failed: %zu\n", retFwriteOp);
        result = HKD_ERROR_WRITE_FILE;
    }
    else
    {
        result = HKD_FILE_SUCCESS;
    }

    fclose(pFile);

    return result;
}

HKD_FileStatus hkd_destroy_file(HKD_File* file)
{
    if ( file->data != NULL )
    {
        free(file->data);
        file->size = 0;

        return HKD_FILE_SUCCESS;
    }

    return HKD_ERROR_NO_FILE;
}
