/*****************************************************************************
 * Copyright (c) 2013-2016 Intel Corporation
 * All rights reserved.
 *
 * WARRANTY DISCLAIMER
 *
 * THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
 * MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Intel Corporation is the author of the Materials, and requests that all
 * problem reports or change requests be submitted to it directly
 *****************************************************************************/
#if defined(__USE_OPENCL__) || defined(__USE_CUDA__)

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include "utils.h"
#include <assert.h>

//we want to use POSIX functions
#pragma warning( push )
#pragma warning( disable : 4996 )


void LogInfo(const char* str, ...)
{
    if (str)
    {
        va_list args;
        va_start(args, str);

        vfprintf(stdout, str, args);

        va_end(args);
    }
}

void LogError(const char* str, ...)
{
    if (str)
    {
        va_list args;
        va_start(args, str);

        vfprintf(stderr, str, args);

        va_end(args);
    }
}

// Upload the OpenCL C source code to output argument source
// The memory resource is implicitly allocated in the function
// and should be deallocated by the caller
int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize)
{
    int errorCode = RSEC_SUCCESS;
    
#ifdef __linux__
    // Linux implementation
    char executablePath[4096] = {0}; // Use a reasonable size for Linux
    ssize_t count = readlink("/proc/self/exe", executablePath, 4096);
    if (count <= 0) {
        LogError("Error: Couldn't determine executable path.\n");
        return RSEC_FILE_NOT_EXIST;
    }
    executablePath[count] = '\0'; // readlink doesn't null-terminate
    
    // Remove executable name to get directory
    char* lastSlash = strrchr(executablePath, '/');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }
#elif defined(_WIN32)
    // Windows implementation
    char executablePath[MAX_PATH] = {0}; // MAX_PATH is defined in Windows headers
    DWORD count = GetModuleFileNameA(NULL, executablePath, MAX_PATH);
    if (count == 0) {
        LogError("Error: Couldn't determine executable path.\n");
        return RSEC_FILE_NOT_EXIST;
    }
    
    // Remove executable name to get directory
    char* lastSlash = strrchr(executablePath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }
#else
    // Fallback for other platforms
    LogError("Error: Executable path detection not implemented for this platform.\n");
    return RSEC_FILE_NOT_EXIST;
#endif

    // Calculate required buffer size and allocate
    size_t dirLength = strlen(executablePath);
    size_t fileNameLength = strlen(fileName);
    size_t fullPathSize = dirLength + fileNameLength + 1;
    
    char* fullPath = new char[fullPathSize];
    if (fullPath == NULL) {
        LogError("Error: Couldn't allocate memory for path.\n");
        return RSEC_OUT_OF_HOST_MEMORY;
    }
    
    // Construct full path by combining executable directory and filename
    snprintf(fullPath, fullPathSize, "%s%s", executablePath, fileName);

    FILE* fp = NULL;
#ifdef __linux__
    fp = fopen(fullPath, "rb");
#else
    fopen_s(&fp, fullPath, "rb");
#endif
    if (fp == NULL)
    {
        LogError("Error: Couldn't find program source file '%s'.\n", fullPath);
        delete[] fullPath;
        errorCode = RSEC_FILE_NOT_EXIST;
    }
    else {
        fseek(fp, 0, SEEK_END);
        *sourceSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *source = new char[*sourceSize + 1];
        if (*source == NULL)
        {
            LogError("Error: Couldn't allocate %d bytes for program source from file '%s'.\n", 
                     *sourceSize, fullPath);
            delete[] fullPath;
            fclose(fp);
            errorCode = RSEC_OUT_OF_HOST_MEMORY;
        }
        else {
            fread(*source, 1, *sourceSize, fp);
            (*source)[*sourceSize] = '\0';
            fclose(fp);
        }
        delete[] fullPath;
    }
    return errorCode;
}
#pragma warning( pop )

#endif
