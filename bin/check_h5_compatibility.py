#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import argparse
import hdf5plugin  # must be imported before h5py
import h5py
import numpy as np
import re
import sys


def print_ok(message):
    print(f"[ OK ] {message}")


def print_warn(message):
    print(f"[WARN] {message}")


def print_fail(message):
    print(f"[FAIL] {message}")
    sys.exit(1)


def check_file(filename):
    print(f"\nopening file '{filename}'\n")
    with open(filename, "rb") as f:
        header = f.read(8)
        version = f.read(1)
    if header != b"\211HDF\r\n\032\n":
        print_fail("not a HDF5 file.")
    if int(version[0]) in [0, 2, 3]:
        print_ok(f"superblock version {int(version[0])}")
    else:
        print_fail(f"superblock version {int(version[0])} not supported.")

    master = h5py.File(filename, "r")
    try:
        data = master["/entry/data"]
    except Exception:
        print_fail(f"error accessing '/entry/data'")
    try:
        detector = master["/entry/instrument/detector"]
    except Exception:
        print_fail(f"error accessing'/entry/instrument/detector'")
    try:
        detectorSpecific = detector["detectorSpecific"]
    except Exception:
        print_fail(f"error accessing '/entry/instrument/detector/detectorSpecific'")

    try:
        x_pixel_size = detector[f"x_pixel_size"][()]
        dtype = x_pixel_size.dtype
        if dtype not in [np.float32, np.float64]:
            print_fail(
                f"'/entry/instrument/detector/x_pixel_size' must be either float32 or float64"
            )
        else:
            print_ok(
                f"'/entry/instrument/detector/x_pixel_size' found [value: {x_pixel_size}, dtype: {dtype}]"
            )
    except KeyError:
        print_warn(
            "'/entry/instrument/detector/x_pixel_size' not found, neggia will return 0.0 for qx"
        )
    except Exception as e:
        print_fail(
            f"error accessing '/entry/instrument/detector/x_pixel_size': {str(e)}"
        )

    try:
        y_pixel_size = detector[f"y_pixel_size"][()]
        dtype = y_pixel_size.dtype
        if dtype not in [np.float32, np.float64]:
            print_fail(
                f"'/entry/instrument/detector/y_pixel_size' must be either float32 or float64"
            )
        else:
            print_ok(
                f"'/entry/instrument/detector/y_pixel_size' found [value: {y_pixel_size}, dtype: {dtype}]"
            )
    except KeyError:
        print_warn(
            "'/entry/instrument/detector/y_pixel_size' not found, neggia will return 0.0 for qy"
        )
    except Exception as e:
        print_fail(
            f"error accessing '/entry/instrument/detector/y_pixel_size': {str(e)}"
        )

    try:
        number_of_triggers = detectorSpecific[f"ntrigger"][()]
        if number_of_triggers.dtype not in [
            np.int8,
            np.int16,
            np.int32,
            np.int64,
            np.uint8,
            np.uint16,
            np.uint32,
            np.uint64,
        ]:
            print_fail(
                f"'/entry/instrument/detector/detectorSpecific/ntrigger' must be an integer [dtype: {number_of_triggers.dtype}]"
            )
        elif number_of_triggers <= 0:
            print_fail(
                f"'/entry/instrument/detector/detectorSpecific/ntrigger' must be a positive non-zero integer [value: {number_of_triggers}]"
            )
        else:
            print_ok(
                f"'/entry/instrument/detector/detectorSpecific/ntrigger' found [value: {number_of_triggers}]"
            )
    except KeyError:
        print_warn(
            f"'/entry/instrument/detector/detectorSpecific/ntrigger' not found. neggia will assume ntrigger = 1."
        )
    except Exception as e:
        print_fail(
            f"error accessing '/entry/instrument/detector/detectorSpecific/ntrigger': {str(e)}"
        )

    try:
        number_of_images = detectorSpecific[f"nimages"][()]
        if number_of_images.dtype not in [
            np.int8,
            np.int16,
            np.int32,
            np.int64,
            np.uint8,
            np.uint16,
            np.uint32,
            np.uint64,
        ]:
            print_fail(
                f"'/entry/instrument/detector/detectorSpecific/nimages' must be an integer [dtype: {number_of_images.dtype}]"
            )
        elif number_of_images <= 0:
            print_fail(
                f"'/entry/instrument/detector/detectorSpecific/nimages' must be a positive non-zero integer [value: {number_of_images}]"
            )
        else:
            print_ok(
                f"'/entry/instrument/detector/detectorSpecific/nimages' found [value: {number_of_images}]"
            )
    except Exception as e:
        print_fail(
            f"'error accessing '/entry/instrument/detector/detectorSpecific/nimages': {str(e)}"
        )

    try:
        pm_chunks = detectorSpecific["pixel_mask"].chunks
        if pm_chunks is not None:
            print_fail(
                f"""'/entry/instrument/detector/detectorSpecific/pixel_mask' 2D data may not be chunked.
       Please save your pixel mask with disabled chunking.
       In python you can do this by adding argument 'chunks=None' to method create_dataset.
"""
            )
        pm_shape = detectorSpecific["pixel_mask"].shape
        pm = detectorSpecific["pixel_mask"][()]
        if pm.dtype == np.uint32:
            print_ok(
                f"'/entry/instrument/detector/detectorSpecific/pixel_mask' found [dtype: {pm.dtype}, shape: {pm.shape}]"
            )
        else:
            print_warn(
                f"""'/entry/instrument/detector/detectorSpecific/pixel_mask' found [dtype: {pm.dtype}, shape: {pm.shape}]
       dtype of pixel_mask not uint32. wrong format?
       neggia will apply pixel_mask and set data to
             -1 for pixel_mask & 0b00001
             -2 for pixel_mask & 0b11110
             -1 if data value larger than max of signed int32
       neggia will convert to uint32 if type is different and
       throw an error if any value is negative"""
            )
            if (pm < 0).any():
                print_fail("there are negative values in your pixel_mask")
    except Exception as e:
        print_fail(
            f"'error accessing '/entry/instrument/detector/detectorSpecific/pixel_mask': {str(e)}"
        )

    data_entries = [str(key) for key in data.keys()]

    count_data_entries_correct_format = 0
    for entry in data_entries:
        path = f"/entry/data/{entry}"
        r = re.compile("data_\d{6}")
        if entry != "data" and r.match(entry) is None:
            print_warn(
                f"""path '{path}' wrong format to contain images
       (must be 'data_******' or 'data' for single file).
       neggia will ignore this entry."""
            )
        else:
            data_chunks = master[path].chunks
            data_shape = master[path].shape
            if data_chunks is None:
                print_fail(f"image data in '{path}' must be chunked")
            if len(data_shape) != 3:
                print_fail(f"expected 3-dimensional data for '{path}'")
            if (data_shape[1], data_shape[2]) != (pm_shape[0], pm_shape[1]):
                print_fail(
                    f"""expected image data shape (any, {pm_shape[0]}, {pm_shape[1]}) but got {data_shape}
       this shape is deduced from the shape of the pixel mask. (perhaps transposed?)"""
                )
            expected_chunks = (1, data_shape[1], data_shape[2])
            if data_chunks != expected_chunks:
                print_fail(f"expected chunks {expected_chunks} but got {data_chunks}")
            count_data_entries_correct_format += 1
    if count_data_entries_correct_format == 0:
        print_fail(f"no entries with correct format found in '/entry/data'")
    desc = "entry" if count_data_entries_correct_format == 1 else "entries"
    print_ok(
        f"found {count_data_entries_correct_format} {desc} for image data in '/entry/data'\n"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Check HDF5 files for compatibility with neggia"
    )
    parser.add_argument("filename", type=str, nargs="+", help="filenames to test")
    args = parser.parse_args()
    for master in args.filename:
        check_file(master)
