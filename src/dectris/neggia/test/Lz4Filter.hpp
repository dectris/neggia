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

#ifndef LZ4FILTER_HPP
#define LZ4FILTER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dectris/neggia/compression_algorithms//lz4.h>
#include <H5PLextern.h>
#include <dectris/neggia/user/H5File.h>


#ifndef INT32_MAX
#define INT32_MAX   0x7fffffffL  /// 2GB
#endif

#define LZ4_FILTER 32004

/// conversion macros: BE -> host, and host -> BE
#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__) || defined(__MACH__) || defined(__unix)
#include <arpa/inet.h>
#elif defined(_WIN32)
#include <Winsock2.h>
#endif

#define htonll(x) ( ( (uint64_t)(htonl( (uint32_t)((x << 32) >> 32)))<< 32) | htonl( ((uint32_t)(x >> 32)) ))
#define ntohll(x) htonll(x)

#define htobe16t(x) htons(x)
#define htobe32t(x) htonl(x)
#define htobe64t(x) htonll(x)
#define be16toht(x) ntohs(x)
#define be32toht(x) ntohl(x)
#define be64toht(x) ntohll(x)


#define DEFAULT_BLOCK_SIZE 1<<30; /* 1GB. LZ4 needs blocks < 1.9GB. */

static size_t lz4_filter(unsigned int flags, size_t cd_nelmts,
                         const unsigned int cd_values[], size_t nbytes,
                         size_t *buf_size, void **buf)
{
   void * outBuf = NULL;
   size_t ret_value;

   if (flags & H5Z_FLAG_REVERSE)
   {
      const char* rpos = (char*)*buf; /* pointer to current read position */


      const uint64_t * const i64Buf = (uint64_t *) rpos;
      const uint64_t origSize = (uint64_t)(be64toht(*i64Buf));/* is saved in be format */
      rpos += 8; /* advance the pointer */

      uint32_t *i32Buf = (uint32_t*)rpos;
      uint32_t blockSize = (uint32_t)(be32toht(*i32Buf));
      rpos += 4;
      if(blockSize>origSize)
         blockSize = origSize;

      if (NULL==(outBuf = malloc(origSize)))
      {
         printf("cannot malloc\n");
         goto error;
      }
      char *roBuf = (char*)outBuf;   /* pointer to current write position */
      uint64_t decompSize     = 0;
      /// start with the first block ///
      while(decompSize < origSize)
      {

         if(origSize-decompSize < blockSize) /* the last block can be smaller than blockSize. */
            blockSize = origSize-decompSize;
         i32Buf = (uint32_t*)rpos;
         uint32_t compressedBlockSize =  be32toht(*i32Buf);  /// is saved in be format
         rpos += 4;
         if(compressedBlockSize == blockSize) /* there was no compression */
         {
            memcpy(roBuf, rpos, blockSize);
         }
         else /* do the decompression */
         {
            int compressedBytes = LZ4_decompress_fast(rpos, roBuf, blockSize);
            if(compressedBytes != compressedBlockSize)
            {
               printf("decompressed size not the same: %d, != %d\n", compressedBytes, compressedBlockSize);
               goto error;
            }
         }

         rpos += compressedBlockSize;   /* advance the read pointer to the next block */
         roBuf += blockSize;            /* advance the write pointer */
         decompSize += blockSize;
      }
      free(*buf);
      *buf = outBuf;
      outBuf = NULL;
      ret_value = (size_t)origSize;  // should always work, as orig_size cannot be > 2GB (sizeof(size_t) < 4GB)
   }
   else /* forward filter */
   {
      if (nbytes > INT32_MAX)
      {
         /* can only compress chunks up to 2GB */
         goto error;
      }


      size_t blockSize;
      if(cd_nelmts > 0 && cd_values[0] > 0)
      {
         blockSize = cd_values[0];
      }
      else
      {
         blockSize = DEFAULT_BLOCK_SIZE;
      }
      if(blockSize > nbytes)
      {
         blockSize = nbytes;
      }
      size_t nBlocks = (nbytes-1)/blockSize +1 ;
      if (NULL==(outBuf = malloc(LZ4_COMPRESSBOUND(nbytes)
                                 + 4+8 + nBlocks*4)))
      {
         goto error;
      }

      char *rpos  = (char*)*buf;      /* pointer to current read position */
      char *roBuf = (char*)outBuf;    /* pointer to current write position */
      /* header */
      uint64_t * i64Buf = (uint64_t *) (roBuf);
      i64Buf[0] = htobe64t((uint64_t)nbytes); /* Store decompressed size in be format */
      roBuf += 8;

      uint32_t *i32Buf =  (uint32_t *) (roBuf);
      i32Buf[0] = htobe32t((uint32_t)blockSize); /* Store the block size in be format */
      roBuf += 4;

      size_t outSize = 12; /* size of the output buffer. Header size (12 bytes) is included */

      for(size_t block = 0; block < nBlocks; ++block)
      {
         size_t origWritten = block*blockSize;
         if(nbytes - origWritten < blockSize) /* the last block may be < blockSize */
            blockSize = nbytes - origWritten;

         uint32_t compBlockSize = LZ4_compress(rpos, roBuf+4, blockSize); /// reserve space for compBlockSize
         if(!compBlockSize)
            goto error;
         if(compBlockSize >= blockSize) /* compression did not save any space, do a memcpy instead */
         {
            compBlockSize = blockSize;
            memcpy(roBuf+4, rpos, blockSize);
         }

         i32Buf =  (uint32_t *) (roBuf);
         i32Buf[0] = htobe32t((uint32_t)compBlockSize);  /* write blocksize */
         roBuf += 4;

         rpos += blockSize;     	/* advance read pointer */
         roBuf += compBlockSize;       /* advance write pointer */
         outSize += compBlockSize + 4;
      }

      free(*buf);
      *buf = outBuf;
      *buf_size = outSize;
      outBuf = NULL;
      ret_value = outSize;

   }
done:
   if(outBuf)
      free(outBuf);
   return ret_value;


error:
   if(outBuf)
      free(outBuf);
   outBuf = NULL;
   return 0;

}


const H5Z_class2_t H5Z_LZ4[1] = {{
                                    H5Z_CLASS_T_VERS,                 /* H5Z_class_t version */
                                    (H5Z_filter_t)LZ4_FILTER,         /* Filter id number             */
                                    1,              /* encoder_present flag (set to true) */
                                    1,              /* decoder_present flag (set to true) */
                                    "HDF5 lz4 filter; see http://www.hdfgroup.org/services/contributions.html",
                                    /* Filter name for debugging    */
                                    NULL,           /* The "can apply" callback     */
                                    NULL,           /* The "set local" callback     */
                                    (H5Z_func_t)lz4_filter,         /* The actual filter function   */
                                 }};

//H5PL_type_t   H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
//const void *H5PLget_plugin_info(void) {return H5Z_LZ4;}

inline herr_t h5RegisterLZ4Comression() {
   return H5Zregister(H5Z_LZ4);
}

inline H5Z_filter_t h5LZ4FilterId() {
   return H5Z_LZ4->id;
}




#endif // LZ4FILTER_HPP

