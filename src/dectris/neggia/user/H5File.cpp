#include "H5File.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

struct UnMap {
    size_t size;
    void operator()(char* addr) {
        munmap(addr, size);
    }
};



std::shared_ptr<char> mapFile(const std::string & fileName)
{
    int fd = open(fileName.c_str(), O_RDONLY);
    if(fd < 0) {
        std::cerr << "ERRNO: " << errno << std::endl;
        throw std::out_of_range("Cannot map file");
    }
    off_t fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char * filePointer = (char*) mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
    if(filePointer == MAP_FAILED) {
        close(fd);
        std::cerr << "ERRNO: " << errno << std::endl;
        throw std::out_of_range("Cannot map file");
    }
    close(fd);
    UnMap deleter;
    deleter.size = fsize;
    return std::shared_ptr<char>(filePointer, deleter);
}

}



H5File::H5File(const std::string &path):
    _fileAddress(mapFile(path))
{
    for(ssize_t i=path.size()-1; i>0; i--) {
        if(path[i] == '/') {
            _fileDir = std::string(path, 0, i);
            break;
        }
    }
    if(_fileDir.empty()) _fileDir = ".";
}

H5File::~H5File()
{

}


const char *H5File::fileAddress() const
{
    return _fileAddress.get();
}

std::string H5File::fileDir() const
{
    return _fileDir;
}
