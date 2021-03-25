// SPDX-License-Identifier: MIT

#include <dectris/neggia/plugin/H5ToXds.h>
#include <dlfcn.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>

using dl_plugin_open = decltype(&plugin_open);
using dl_plugin_get_header = decltype(&plugin_get_header);
using dl_plugin_get_data = decltype(&plugin_get_data);
using dl_plugin_close = decltype(&plugin_close);

int main(int argc, char* argv[]) {
    dl_plugin_open open_file;
    dl_plugin_get_header get_header;
    dl_plugin_get_data get_data;
    dl_plugin_close close_file;
    switch (argc) {
        case 2: {
            std::cerr << "using neggia methods directly\n";
            open_file = plugin_open;
            get_header = plugin_get_header;
            get_data = plugin_get_data;
            close_file = plugin_close;
            break;
        }
        case 3: {
            std::string so_filename = argv[2];
            std::cerr << "dynamically linking methods from " << so_filename
                      << "\n";
            void* pluginHandle = dlopen(so_filename.c_str(), RTLD_NOW);
            open_file = (dl_plugin_open)dlsym(pluginHandle, "plugin_open");
            get_header = (dl_plugin_get_header)dlsym(pluginHandle,
                                                     "plugin_get_header");
            get_data =
                    (dl_plugin_get_data)dlsym(pluginHandle, "plugin_get_data");
            close_file = (dl_plugin_close)dlsym(pluginHandle, "plugin_close");
            break;
        }
        default:
            std::cerr
                    << "Please provide the filename of the hdf5 file that you "
                       "would like to test against neggia plugin\n";
            return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    int error_flag;
    int info_array[1024];
    open_file(filename.c_str(), info_array, &error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_open returned error " << error_flag << "\n";
        return EXIT_FAILURE;
    }
    std::cerr << "[ OK ] plugin_open successful\n";
    int nx, ny, nbytes, nframes;
    float qx, qy;
    get_header(&nx, &ny, &nbytes, &qx, &qy, &nframes, info_array, &error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_get_header returned error " << error_flag
                  << "\n";
        std::cerr << "       please make sure that at least the first "
                     "data-file\n";
        std::cerr << "       is present in the directory of master-file!\n";
        return -1;
    }
    std::cerr << "[ OK ] plugin_get_header successful\n";
    std::cerr << "    nx      " << nx << "\n";
    std::cerr << "    ny      " << ny << "\n";
    std::cerr << "    nbytes  " << nbytes << "\n";
    std::cerr << "    nframes " << nframes << "\n";
    std::cerr << "    qx      " << qx << "\n";
    std::cerr << "    qy      " << qy << "\n";
    if (nframes <= 0) {
        std::cerr << "[FAIL] nframes must be positive\n";
        return EXIT_FAILURE;
    }
    if (qx <= 0.0 || qy <= 0.0) {
        std::cerr << "[WARN] qx/qy should be positive (pixel sizes) - either "
                     "missing or wrong\n";
    }

    auto dataArrayExtracted = std::unique_ptr<int[]>(new int[nx * ny]);
    std::cerr << "  trying to extract all data frames. please wait...\n";
    std::cerr << "    [" << std::setw(6) << std::right << "1"
              << " / " << nframes << " ]\n";
    for (int frame = 1; frame <= nframes; ++frame) {
        if ((frame % 100) == 0) {
            std::cerr << "    [" << std::setw(6) << std::right << frame << " / "
                      << nframes << " ]\n";
        }
        get_data(&frame, &nx, &ny, dataArrayExtracted.get(), info_array,
                 &error_flag);
        if (error_flag != 0) {
            std::cerr << "[FAIL] plugin_get_data for frame " << frame
                      << " returned error " << error_flag << "\n";
            return EXIT_FAILURE;
        }
    }

    close_file(&error_flag);
    if (error_flag != 0) {
        std::cerr << "[FAIL] plugin_close returned error " << error_flag
                  << "\n";
        return EXIT_FAILURE;
    }
    std::cerr << "[ OK ] plugin_close successful\n";
    return EXIT_SUCCESS;
}