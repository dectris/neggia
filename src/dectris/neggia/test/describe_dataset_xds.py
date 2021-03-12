#!/usr/bin/env python3
# SPDX-License-Identifier: MIT

import argparse
from dataclasses import dataclass
import hdf5plugin  # must be imported before h5py
import h5py
import numpy as np
import os


def describe(filename):
    master = h5py.File(filename, "r")
    number_of_images = master[f"/entry/instrument/detector/detectorSpecific/nimages"][
        ()
    ]
    try:
        number_of_triggers = master[
            f"/entry/instrument/detector/detectorSpecific/ntrigger"
        ][()]
    except Exception:
        number_of_triggers = 1
    shape = master["/entry/instrument/detector/detectorSpecific/pixel_mask"][()].shape
    print(
        f"""    CheckXdsPlugin(
        "{filename}",
        {shape[1]},
        {shape[0]},
        7.5e-5,
        7.5e-5,
        {number_of_images},
        {number_of_triggers}
    );
"""
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--master", required=True, help="filename of master file")
    args = parser.parse_args()
    describe(args.master)
