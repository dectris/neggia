// SPDX-License-Identifier: MIT

#include "Decode.h"
#include <dectris/neggia/compression_algorithms/bitshuffle.h>
#include <dectris/neggia/compression_algorithms/lz4.h>
#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffffL  /// 2GB
#endif

/// conversion macros: BE -> host, and host -> BE
#include <arpa/inet.h>

#define htonll(x)                                             \
    (((uint64_t)(htonl((uint32_t)((x << 32) >> 32))) << 32) | \
     htonl(((uint32_t)(x >> 32))))
#define ntohll(x) htonll(x)
#define htobe16t(x) htons(x)
#define htobe32t(x) htonl(x)
#define htobe64t(x) htonll(x)
#define be16toht(x) ntohs(x)
#define be32toht(x) ntohl(x)
#define be64toht(x) ntohll(x)

namespace {
// set inBuffer to inBuffer + 12 and outBufferSize to decompressed Size. Read
// blockSize from data
void readLz4Header(const char*& inBuffer, size_t& outSize, size_t& blockSize) {
    const uint64_t* const i64Buf = (uint64_t*)inBuffer;
    const uint64_t origSize =
            (uint64_t)(be64toht(*i64Buf));  // is saved in be format
    if (origSize > outSize)
        throw std::runtime_error("DCompression: outBufferSize too small");
    outSize = origSize;
    inBuffer += 8;  // advance the pointer
    const uint32_t* i32Buf = (const uint32_t*)inBuffer;
    blockSize = (uint32_t)(be32toht(*i32Buf));
    inBuffer += 4;
}
}  // namespace

void lz4Decode(const char* inBuffer, char* outBuffer, size_t& outBufferSize) {
    size_t blockSize = 0;
    // set inBuffer to inBuffer + 12 and outBufferSize to decompressed Size.
    // Read blockSize from data
    readLz4Header(inBuffer, outBufferSize, blockSize);
    if (blockSize > outBufferSize)
        blockSize = outBufferSize;

    uint64_t decompSize = 0;
    // start with the first block
    while (decompSize < outBufferSize) {
        if (outBufferSize - decompSize <
            blockSize)  // the last block can be smaller than blockSize.
            blockSize = outBufferSize - decompSize;
        const uint32_t* i32Buf = (const uint32_t*)inBuffer;
        uint32_t compressedBlockSize =
                be32toht(*i32Buf);  // is saved in be format
        inBuffer += 4;
        if (compressedBlockSize == blockSize)  // there was no compression
        {
            memcpy(outBuffer, inBuffer, blockSize);
        } else  // do the decompression
        {
            int compressedBytes =
                    LZ4_decompress_fast(inBuffer, outBuffer, blockSize);
            if (compressedBytes != (int)compressedBlockSize) {
                std::ostringstream failureStr;
                failureStr << "DCompression: Decompressed size of "
                           << compressedBlockSize << " bytes expected. Got "
                           << compressedBytes << " bytes." << std::endl;
                throw std::runtime_error(failureStr.str());
            }
        }
        inBuffer += compressedBlockSize; /* advance the read pointer to the next
                                            block */
        outBuffer += blockSize;          /* advance the write pointer */
        decompSize += blockSize;
    }
}

void bshufUncompressLz4(const char* inBuffer,
                        char* outBuffer,
                        size_t& outBufferSize,
                        size_t elementSize) {
    size_t blockSize;
    readLz4Header(inBuffer, outBufferSize, blockSize);
    if (outBufferSize % elementSize)
        throw std::runtime_error("Non integer number of elements");
    int64_t err = bshuf_decompress_lz4(inBuffer, outBuffer,
                                       outBufferSize / elementSize, elementSize,
                                       blockSize / elementSize);
    if (err < 0) {
        std::stringstream errStream;
        errStream << "bitshuffle returned with error code: " << err;
        throw std::runtime_error(errStream.str());
    }
}
