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
};

#endif  // RESOLVED_PATH_H
