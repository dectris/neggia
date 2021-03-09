// SPDX-License-Identifier: MIT
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "JenkinsLookup3Checksum.h"
#include <stdint.h>
#include <stdlib.h>

#define H5_lookup3_rot(x, k) (((x) << (k)) ^ ((x) >> (32 - (k))))
#define H5_lookup3_mix(a, b, c)     \
    {                               \
        a -= c;                     \
        a ^= H5_lookup3_rot(c, 4);  \
        c += b;                     \
        b -= a;                     \
        b ^= H5_lookup3_rot(a, 6);  \
        a += c;                     \
        c -= b;                     \
        c ^= H5_lookup3_rot(b, 8);  \
        b += a;                     \
        a -= c;                     \
        a ^= H5_lookup3_rot(c, 16); \
        c += b;                     \
        b -= a;                     \
        b ^= H5_lookup3_rot(a, 19); \
        a += c;                     \
        c -= b;                     \
        c ^= H5_lookup3_rot(b, 4);  \
        b += a;                     \
    }
#define H5_lookup3_final(a, b, c)   \
    {                               \
        c ^= b;                     \
        c -= H5_lookup3_rot(b, 14); \
        a ^= c;                     \
        a -= H5_lookup3_rot(c, 11); \
        b ^= a;                     \
        b -= H5_lookup3_rot(a, 25); \
        c ^= b;                     \
        c -= H5_lookup3_rot(b, 16); \
        a ^= c;                     \
        a -= H5_lookup3_rot(c, 4);  \
        b ^= a;                     \
        b -= H5_lookup3_rot(a, 14); \
        c ^= b;                     \
        c -= H5_lookup3_rot(b, 24); \
    }

/*
-------------------------------------------------------------------------------
H5_checksum_lookup3() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  length  : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Two keys differing by one or two bits will have
totally different hash values.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (uint8_t **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = H5_checksum_lookup( k[i], len[i], h);

By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
-------------------------------------------------------------------------------
*/

uint32_t H5_checksum_lookup3(const void* key, size_t length, uint32_t initval) {
    const uint8_t* k = (const uint8_t*)key;
    uint32_t a, b, c; /* internal state */

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

    /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
    while (length > 12) {
        a += k[0];
        a += ((uint32_t)k[1]) << 8;
        a += ((uint32_t)k[2]) << 16;
        a += ((uint32_t)k[3]) << 24;
        b += k[4];
        b += ((uint32_t)k[5]) << 8;
        b += ((uint32_t)k[6]) << 16;
        b += ((uint32_t)k[7]) << 24;
        c += k[8];
        c += ((uint32_t)k[9]) << 8;
        c += ((uint32_t)k[10]) << 16;
        c += ((uint32_t)k[11]) << 24;
        H5_lookup3_mix(a, b, c);
        length -= 12;
        k += 12;
    }

    /*-------------------------------- last block: affect all 32 bits of (c) */
    switch (length) /* all the case statements fall through */ {
        case 12:
            c += ((uint32_t)k[11]) << 24;
        case 11:
            c += ((uint32_t)k[10]) << 16;
        case 10:
            c += ((uint32_t)k[9]) << 8;
        case 9:
            c += k[8];
        case 8:
            b += ((uint32_t)k[7]) << 24;
        case 7:
            b += ((uint32_t)k[6]) << 16;
        case 6:
            b += ((uint32_t)k[5]) << 8;
        case 5:
            b += k[4];
        case 4:
            a += ((uint32_t)k[3]) << 24;
        case 3:
            a += ((uint32_t)k[2]) << 16;
        case 2:
            a += ((uint32_t)k[1]) << 8;
        case 1:
            a += k[0];
            break;
        case 0:
            goto done;
    }

    H5_lookup3_final(a, b, c);

done:
    return c;
}

uint32_t JenkinsLookup3Checksum(const std::string& str, uint32_t initval) {
    return H5_checksum_lookup3(str.data(), str.length(), initval);
}
