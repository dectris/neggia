#include "H5LinkInfoMessage.h"

H5LinkInfoMsg::H5LinkInfoMsg(const char *fileAddress, size_t offset):
    H5Object(fileAddress, offset)
{

}

H5LinkInfoMsg::H5LinkInfoMsg(const H5Object &other):
    H5Object(other)
{

}

uint8_t H5LinkInfoMsg::getFlags() const
{
    return uint8(1);
}

bool H5LinkInfoMsg::existsMaximumCreationIndex() const
{
    return (bool)(getFlags() & 1);
}

uint64_t H5LinkInfoMsg::getFractalHeapAddress() const
{
    if(existsMaximumCreationIndex()) return uint64(10);
    else return uint64(2);
}

uint64_t H5LinkInfoMsg::getBTreeAddress() const
{
    if(existsMaximumCreationIndex()) return uint64(18);
    else return uint64(10);
}
