// SPDX-License-Identifier: MIT

#ifndef H5OBJECT_H
#define H5OBJECT_H
#include <cstdint>
#include <string>

class H5Object {
public:
    H5Object();
    H5Object(const char* fileAddress, size_t offset);
    const char* fileAddress() const { return _fileAddress; }
    size_t offset() const { return _offset; }
    H5Object at(size_t relativeOffset) const {
        return H5Object(_fileAddress, _offset + relativeOffset);
    }
    H5Object operator+(size_t relativeOffset) const {
        return H5Object(_fileAddress, _offset + relativeOffset);
    }
    const char* address() const { return _fileAddress + _offset; }
    const char* address(size_t offset) const { return address() + offset; }
    uint8_t read_u8(size_t offset) const { return *(uint8_t*)address(offset); }
    uint16_t read_u16(size_t offset) const {
        return *(uint16_t*)address(offset);
    }
    uint32_t read_u32(size_t offset) const {
        return *(uint32_t*)address(offset);
    }
    uint64_t read_u64(size_t offset) const {
        return *(uint64_t*)address(offset);
    }
    int8_t read_i8(size_t offset) const { return *(int8_t*)address(offset); }
    int16_t read_i16(size_t offset) const { return *(int16_t*)address(offset); }
    int32_t read_i32(size_t offset) const { return *(int32_t*)address(offset); }
    int64_t read_i64(size_t offset) const { return *(int64_t*)address(offset); }

    size_t readIntegerAt(size_t offset, size_t length) const;

    void debugPrint(int nBytes) const;

private:
    const char* _fileAddress;
    size_t _offset;
};

#endif  // H5OBJECT_H
