// SPDX-License-Identifier: MIT

#include <dectris/neggia/plugin/H5ToXds.h>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Please provide the filename of the hdf5 file that you "
                     "would like to test against neggia plugin\n";
        return -1;
    }
    std::string filename = argv[1];
    int error_flag;
    int info_array[1024];
    std::cerr << "\nOpening file " << filename << "\n";
    plugin_open(filename.c_str(), info_array, &error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_open returned error " << error_flag << "\n";
        return -1;
    }
    std::cerr << "[ OK ] plugin_open successful\n";
    int nx, ny, nbytes, nframes;
    float qx, qy;
    plugin_get_header(&nx, &ny, &nbytes, &qx, &qy, &nframes, info_array,
                      &error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_get_header returned error " << error_flag
                  << "\n";
        std::cerr << "       please make sure that at least the first "
                     "data-file\n";
        std::cerr << "       is present in the directory of master-file!\n";
        return -1;
    }
    std::cerr << "[ OK ] plugin_get_header successful\n";
    std::cerr << "   nx      " << nx << "\n";
    std::cerr << "   ny      " << ny << "\n";
    std::cerr << "   nbytes  " << nbytes << "\n";
    std::cerr << "   nframes " << nframes << "\n";
    std::cerr << "   qx      " << qx << "\n";
    std::cerr << "   qy      " << qy << "\n";
    if (qx <= 0.0 || qy <= 0.0) {
        std::cerr << "[WARN] qx/qy should be positive (pixel sizes) - either "
                     "missing or wrong\n";
    }

    auto dataArrayExtracted = std::unique_ptr<int[]>(new int[nx * ny]);

    for (int frame = 1; frame <= nframes; ++frame) {
        plugin_get_data(&frame, &nx, &ny, dataArrayExtracted.get(), info_array,
                        &error_flag);
        if (error_flag != 0) {
            std::cerr << "[FAIL] plugin_get_data for frame " << frame
                      << " returned error " << error_flag << "\n";
            return -1;
        }
    }

    plugin_close(&error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_close returned error " << error_flag
                  << "\n";
        return -1;
    }
    std::cerr << "[ OK ] plugin_close successful\n";
    return 0;
}