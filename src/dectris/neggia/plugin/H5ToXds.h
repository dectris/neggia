/**
MIT License

Copyright (c) 2017 DECTRIS Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
