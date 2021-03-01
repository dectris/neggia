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

#include "H5File.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

namespace {

struct UnMap {
    size_t size;
    void operator()(char* addr) { munmap(addr, size); }
};

std::shared_ptr<char> mapFile(const std::string& fileName) {
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "NEGGIA ERROR: OPENING FILE RETURNED ERROR CODE: " << errno
                  << std::endl;
        throw std::out_of_range("Cannot open file");
    }
    off_t fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* filePointer = (char*)mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
    if (filePointer == MAP_FAILED) {
        close(fd);
        std::cerr << "NEGGIA ERROR: MAPPING FILE RETURNED ERROR CODE: " << errno
                  << std::endl;
        throw std::out_of_range("Cannot map file");
    }
    close(fd);
    UnMap deleter;
    deleter.size = fsize;
    return std::shared_ptr<char>(filePointer, deleter);
}

}  // namespace

H5File::H5File(const std::string& path) : _fileAddress(mapFile(path)) {
    for (ssize_t i = path.size() - 1; i > 0; i--) {
        if (path[i] == '/') {
            _fileDir = std::string(path, 0, i);
            break;
        }
    }
    if (_fileDir.empty())
        _fileDir = ".";
}

H5File::~H5File() {}

const char* H5File::fileAddress() const {
    return _fileAddress.get();
}

std::string H5File::fileDir() const {
    return _fileDir;
}
