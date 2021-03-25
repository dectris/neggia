// SPDX-License-Identifier: MIT

#ifndef RESOLVED_PATH_H
#define RESOLVED_PATH_H
#include <memory>
#include <string>
#include "H5ObjectHeader.h"
#include "H5Path.h"

struct ResolvedPath {
    // When resolving a path in hdf5 the result can either be a link to an
    // external file or a H5ObjectHeader.
    // gcc4.8 does not support std::optional yet so we use an unique pointer
    // to externalfile struct. If set, objectHeader will be invalid.
    struct ExternalFile {
        std::string filename;
        H5Path h5Path;
    };
    H5ObjectHeader objectHeader;
    std::unique_ptr<ExternalFile> externalFile;
    std::unique_ptr<H5Path> softLink;
};

#endif  // RESOLVED_PATH_H
