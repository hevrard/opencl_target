//-----------------------------------------------------------------------------
// Copyright (c) 2017 Hugues Evrard, MIT License

#ifndef __OPENCL_TARGET__
#define __OPENCL_TARGET__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h> // getenv, abort
#include <string.h> // strstr
#include <stdio.h>  // printf

#define OPENCL_TARGET_CL_CALL(func, ...) do {                           \
        cl_int __cl_err = 0;                                            \
        __cl_err = func(__VA_ARGS__);                                   \
        if (__cl_err != CL_SUCCESS) {                                   \
            fprintf(stderr, "%s:%d OPENCL_TARGET PANIC: OpenCL primitive failed: %s\n", __FILE__, __LINE__, #func ); \
            abort();                                                    \
        }                                                               \
    } while (0)

void opencl_target_str(cl_platform_id *platform_id, cl_device_id *device_id, char *target);

inline void opencl_target_str(cl_platform_id *platform_id, cl_device_id *device_id, char *target)
{
    cl_platform_id *platforms = NULL;
    cl_device_id *devices = NULL;
    char *device_version = NULL;
    *device_id = NULL;

    cl_uint num_platforms = 0;
    OPENCL_TARGET_CL_CALL (clGetPlatformIDs, 0, NULL, &num_platforms);
    if (num_platforms <= 0) {
        goto opencl_target_exit;
    }

    platforms = (cl_platform_id *) malloc(num_platforms * sizeof(cl_platform_id));
    OPENCL_TARGET_CL_CALL (clGetPlatformIDs, num_platforms, platforms, NULL);

    for (cl_uint i = 0; i < num_platforms; i++) {
        *platform_id = platforms[i];

        cl_uint num_devices = 0;
        OPENCL_TARGET_CL_CALL (clGetDeviceIDs, *platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        if (num_devices <= 0) {
            continue;
        }

        devices = (cl_device_id *) realloc(devices, num_devices * sizeof(cl_device_id));
        OPENCL_TARGET_CL_CALL (clGetDeviceIDs, *platform_id, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

        for (cl_uint j = 0; j < num_devices; j++) {
            *device_id = devices[j];
            size_t device_version_size = 0;
            OPENCL_TARGET_CL_CALL (clGetDeviceInfo, *device_id, CL_DEVICE_VERSION, 0, NULL, &device_version_size);
            if (device_version_size <= 0) {
                continue;
            }
            device_version = (char *) realloc(device_version, device_version_size);
            if (device_version == NULL) {
                goto opencl_target_exit;
            }
            OPENCL_TARGET_CL_CALL (clGetDeviceInfo, *device_id, CL_DEVICE_VERSION, device_version_size, device_version, NULL);
            int name_match = (strstr(device_version, target) != NULL);
            free(device_version);
            device_version = NULL;
            if (name_match) {
                goto opencl_target_exit;
            }
        }
    }

    // reaching here means no device was found;
    *device_id = NULL;
    *platform_id = NULL;

opencl_target_exit:

    if (platforms      != NULL) { free(platforms); }
    if (devices        != NULL) { free(devices); }
    if (device_version != NULL) { free(device_version); }
}

void opencl_target_env(cl_platform_id *platform_id, cl_device_id *device_id);

inline void opencl_target_env(cl_platform_id *platform_id, cl_device_id *device_id)
{
    char *target;
    target = getenv("OPENCL_TARGET");
    if (target == NULL) {
        target = (char *)"";   // cast to please c++ compiler
    }
    opencl_target_str(platform_id, device_id, target);
}

#undef OPENCL_TARGET_CL_CALL

#endif // __OPENCL_TARGET__

#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
