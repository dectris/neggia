#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import argparse
from dataclasses import dataclass
import hdf5plugin  # must be imported before h5py
import h5py
import numpy as np
import os


def describe(filename):
    with open(filename, "rb") as f:
        header = f.read(8)
        version = f.read(1)
    if header != b"\211HDF\r\n\032\n":
        raise RuntimeError("not a HDF5 file.")
    superblock_version = int(version[0])

    master = h5py.File(filename, "r")
    data_entries = [str(key) for key in master["entry"]["data"].keys()]
    if len(data_entries) == 0:
        raise RuntimeError("no data contained or linked in master file")

    # get type for x_pixel_size
    dtype = master[f"/entry/instrument/detector/x_pixel_size"][()].dtype
    if dtype == np.float32:
        float_type = "float"
    elif dtype == np.float64:
        float_type = "double"
    else:
        raise RuntimeError(f"unknown dtype for x_pixels_in_detector: {dtype}")

    # get type for x_pixels_in_detector
    dtype = master[f"/entry/instrument/detector/detectorSpecific/x_pixels_in_detector"][
        ()
    ].dtype
    if dtype == np.uint32:
        int_type = "uint32_t"
    elif dtype == np.uint64:
        int_type = "uint64_t"
    else:
        raise RuntimeError(f"unknown dtype for x_pixels_in_detector: {dtype}")

    # get pm checksum
    if "pixel_mask" in master["/entry/instrument/detector/detectorSpecific"].keys():
        pm_checksum = lookup3(
            master["/entry/instrument/detector/detectorSpecific/pixel_mask"][
                ()
            ].tobytes()
        )
        shape = master["/entry/instrument/detector/detectorSpecific/pixel_mask"][
            ()
        ].shape
    else:
        raise RuntimeError("no pixel_mask found.")

    # get ff checksum
    if "flatfield" in master["/entry/instrument/detector/detectorSpecific"].keys():
        ff_checksum = lookup3(
            master["/entry/instrument/detector/detectorSpecific/flatfield"][
                ()
            ].tobytes()
        )
    else:
        raise RuntimeError("no flatfield found.")

    # get data type
    dtype = master[f"/entry/data/{data_entries[0]}"][()].dtype
    if dtype == np.uint8:
        pixel_type = "uint8_t"
    elif dtype == np.uint16:
        pixel_type = "uint16_t"
    elif dtype == np.uint32:
        pixel_type = "uint32_t"
    else:
        raise RuntimeError(f"unknown dtype for data: {dtype}")

    # get data entries checksums (they vary because of random distribution of virtual pixels)
    data = []
    for i in range(0, len(data_entries)):
        data_shape = master[f"/entry/data/{data_entries[i]}"][()].shape
        data_chk = 0
        for j in range(0, data_shape[0]):
            data_chk = lookup3(
                master[f"/entry/data/{data_entries[i]}"][()][j, :, :].tobytes(),
                data_chk,
            )
        data.append(
            f'{{"{data_entries[i]}", {{{data_shape[0]}, {data_shape[1]}, {data_shape[2]}}}, {data_chk}}}'
        )
    data_entries = ", ".join(data)

    print(
        f"""CheckHdf5(
            "{filename}",
            ExpectedValues<{float_type}, {int_type}, {pixel_type}>{{
                    {superblock_version},
                    {shape[1]},
                    {shape[0]},
                    7.5e-5,
                    7.5e-5,
                    {ff_checksum},
                    {pm_checksum},
                    {{{data_entries}}},
            }});
"""
    )


def rot(x, k):
    return ((x) << (k)) | ((x) >> (32 - (k)))


def mix(a, b, c):
    a &= 0xFFFFFFFF
    b &= 0xFFFFFFFF
    c &= 0xFFFFFFFF
    a -= c
    a &= 0xFFFFFFFF
    a ^= rot(c, 4)
    a &= 0xFFFFFFFF
    c += b
    c &= 0xFFFFFFFF
    b -= a
    b &= 0xFFFFFFFF
    b ^= rot(a, 6)
    b &= 0xFFFFFFFF
    a += c
    a &= 0xFFFFFFFF
    c -= b
    c &= 0xFFFFFFFF
    c ^= rot(b, 8)
    c &= 0xFFFFFFFF
    b += a
    b &= 0xFFFFFFFF
    a -= c
    a &= 0xFFFFFFFF
    a ^= rot(c, 16)
    a &= 0xFFFFFFFF
    c += b
    c &= 0xFFFFFFFF
    b -= a
    b &= 0xFFFFFFFF
    b ^= rot(a, 19)
    b &= 0xFFFFFFFF
    a += c
    a &= 0xFFFFFFFF
    c -= b
    c &= 0xFFFFFFFF
    c ^= rot(b, 4)
    c &= 0xFFFFFFFF
    b += a
    b &= 0xFFFFFFFF
    return a, b, c


def final(a, b, c):
    a &= 0xFFFFFFFF
    b &= 0xFFFFFFFF
    c &= 0xFFFFFFFF
    c ^= b
    c &= 0xFFFFFFFF
    c -= rot(b, 14)
    c &= 0xFFFFFFFF
    a ^= c
    a &= 0xFFFFFFFF
    a -= rot(c, 11)
    a &= 0xFFFFFFFF
    b ^= a
    b &= 0xFFFFFFFF
    b -= rot(a, 25)
    b &= 0xFFFFFFFF
    c ^= b
    c &= 0xFFFFFFFF
    c -= rot(b, 16)
    c &= 0xFFFFFFFF
    a ^= c
    a &= 0xFFFFFFFF
    a -= rot(c, 4)
    a &= 0xFFFFFFFF
    b ^= a
    b &= 0xFFFFFFFF
    b -= rot(a, 14)
    b &= 0xFFFFFFFF
    c ^= b
    c &= 0xFFFFFFFF
    c -= rot(b, 24)
    c &= 0xFFFFFFFF
    return a, b, c


def hashlittle2(data, initval=0):
    def get_byte(a):
        return ord(a)

    if isinstance(data, bytes):

        def get_byte(a):
            return a

    length = lenpos = len(data)

    a = b = c = 0xDEADBEEF + (length) + initval

    a &= 0xFFFFFFFF
    b &= 0xFFFFFFFF
    c &= 0xFFFFFFFF

    p = 0  # string offset
    while lenpos > 12:
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
        a &= 0xFFFFFFFF
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        b &= 0xFFFFFFFF
        c += (
            get_byte(data[p + 8])
            + (get_byte(data[p + 9]) << 8)
            + (get_byte(data[p + 10]) << 16)
            + (get_byte(data[p + 11]) << 24)
        )
        c &= 0xFFFFFFFF
        a, b, c = mix(a, b, c)
        p += 12
        lenpos -= 12

    if lenpos == 12:
        c += (
            get_byte(data[p + 8])
            + (get_byte(data[p + 9]) << 8)
            + (get_byte(data[p + 10]) << 16)
            + (get_byte(data[p + 11]) << 24)
        )
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 11:
        c += (
            get_byte(data[p + 8])
            + (get_byte(data[p + 9]) << 8)
            + (get_byte(data[p + 10]) << 16)
        )
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 10:
        c += get_byte(data[p + 8]) + (get_byte(data[p + 9]) << 8)
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 9:
        c += get_byte(data[p + 8])
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 8:
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
            + (get_byte(data[p + 7]) << 24)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 7:
        b += (
            get_byte(data[p + 4])
            + (get_byte(data[p + 5]) << 8)
            + (get_byte(data[p + 6]) << 16)
        )
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 6:
        b += (get_byte(data[p + 5]) << 8) + get_byte(data[p + 4])
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 5:
        b += get_byte(data[p + 4])
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 4:
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
            + (get_byte(data[p + 3]) << 24)
        )
    if lenpos == 3:
        a += (
            get_byte(data[p + 0])
            + (get_byte(data[p + 1]) << 8)
            + (get_byte(data[p + 2]) << 16)
        )
    if lenpos == 2:
        a += get_byte(data[p + 0]) + (get_byte(data[p + 1]) << 8)
    if lenpos == 1:
        a += get_byte(data[p + 0])
    a &= 0xFFFFFFFF
    b &= 0xFFFFFFFF
    c &= 0xFFFFFFFF
    if lenpos == 0:
        return c, b

    a, b, c = final(a, b, c)

    return c, b


def lookup3(data, initval=0):
    c, b = hashlittle2(data, initval)
    return c


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--master", required=True, help="filename of master file")
    args = parser.parse_args()
    describe(args.master)
