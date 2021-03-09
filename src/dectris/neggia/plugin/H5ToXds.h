// SPDX-License-Identifier: MIT

#ifndef DECTRISH5TOXDS_H
#define DECTRISH5TOXDS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECTRIS_H5TOXDS_CUSTOMER_ID 1         // Customer ID [1:Dectris]
#define DECTRIS_H5TOXDS_VERSION_MAJOR 0       // Version  [Major]
#define DECTRIS_H5TOXDS_VERSION_MINOR 5       // Version  [Minor]
#define DECTRIS_H5TOXDS_VERSION_PATCH 3       // Version  [Patch]
#define DECTRIS_H5TOXDS_VERSION_TIMESTAMP -1  // Version  [timestamp]

void plugin_open(const char*, int info_array[1024], int* error_flag);

void plugin_get_header(int* nx,
                       int* ny,
                       int* nbytes,
                       float* qx,
                       float* qy,
                       int* number_of_frames,
                       int info[1024],
                       int* error_flag);

void plugin_get_data(int* frame_number,
                     int* nx,
                     int* ny,
                     int data_array[],
                     int info_array[1024],
                     int* error_flag);

void plugin_close(int* error_flag);

#ifdef __cplusplus
}
#endif

#endif  // DECTRISH5TOXDS_H
