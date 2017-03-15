#include <dectris/neggia/WriteCbf.h>
#include <dlfcn.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <cstdint>

typedef void (* plugin_open_file)  (
                                   const char *,
                                   int *error_flag);

typedef void (* plugin_get_header) (int * nx, int * ny,
                                    int * nbytes,
                                    float * qx, float * qy,
                                    int *number_of_frames,
                                    int info_array[1024],
                                    int *error_flag);

typedef void (* plugin_get_data)   (int *frame_number,
                                    int * nx, int * ny,
                                    int data_array[],
                                    int *error_flag);

typedef void (* plugin_close_file) (int *error_flag);



int main( int argc, char *argv[] ) {
   void * lib = dlopen(TEST_PLUGIN_LIBRARY, RTLD_NOW);
   plugin_open_file dopen    = (plugin_open_file)  dlsym(lib, "plugin_open_file");
   plugin_get_header dheader = (plugin_get_header) dlsym(lib, "plugin_get_header");
   plugin_get_data ddata     = (plugin_get_data)   dlsym(lib, "plugin_get_data");
   plugin_close_file dclose  = (plugin_close_file) dlsym(lib, "plugin_close_file");


   std::string masterFile("");
   if( argc == 2 ) {
      masterFile = argv[1];
   }
   else if( argc > 2 ) {
      printf("Too many arguments supplied.\n");
      return 1;
   }
   else {
      printf("Expecting absolute filename of the masterfile (with path)\n");
      return 1;
   }
   // std::string masterFile("/mnt/NFS/E-32-0100/test_PSI/lysoHG4/lysoHG4_t0p01_0p1d_0p1s_d150_360deg_master.h5");
   // std::string masterFile("/home/vitb/Lyso_U2_1_master.h5");
   int error_flag=0;

   dopen(masterFile.c_str(), &error_flag);

   int nx;
   int ny;
   int nbytes;
   int number_of_frames;
   float qx, qy;
   int info[1024];

   dheader(&nx, &ny, &nbytes, &qx, &qy, &number_of_frames, info, &error_flag);

   std::cout << "NX:      " << nx << std::endl;
   std::cout << "NY:      " << ny << std::endl;
   std::cout << "NFRAMES: " << number_of_frames << std::endl;
   std::cout << "NBYTES:  " << nbytes << std::endl;
   std::cout << "QX:      " << qx << std::endl;
   std::cout << "QY:      " << qx << std::endl;
   std::cout << "INFO:    " << info[0] << " " << info[1] << std::endl;

   int * data = new int[nx*ny];
   auto t0 = std::chrono::system_clock::now();
   for(int i=1; i<=100; ++i) {
      ddata(&i, &nx, &ny, data, &error_flag);
      if(i%100 == 0) {
         auto t1 = std::chrono::system_clock::now();
         std::cout << "EXTRACTED " << i << " FRAMES" << std::endl;
         std::cout << 1.0e9*100/std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count() << "1/s" << std::endl;
         writeCbf(nx,ny, (const char*)data, masterFile, i, "/tmp/test_"+std::to_string(i)+".cbf");
         t0 = std::chrono::system_clock::now();
      }
   }

   // delete [] data;
   dclose(&error_flag);

   dlclose(lib);
}
