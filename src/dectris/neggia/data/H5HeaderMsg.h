// SPDX-License-Identifier: MIT

#ifndef H5HEADERMESSAGE_H
#define H5HEADERMESSAGE_H
#include "H5Object.h"

/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderMessages
/// Contains type, size and flags of Header Message part of H5ObjectHeader:
/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#V1ObjectHeaderPrefix

struct H5HeaderMessage {
    H5Object object;
    uint16_t type;
};

#endif  // H5HEADERMESSAGE_H
