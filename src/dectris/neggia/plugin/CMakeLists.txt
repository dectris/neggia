#  The MIT License (MIT)
#
#  Copyright (c) 2017 Dectris Ltd., Taefernweg 1, 5405 Baden-Daettwil, Switzerland.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

add_definitions(-DVERSION=\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\")
add_library(dectris-neggia MODULE
  H5Error.h
  H5ToXds.cpp
  H5ToXds.h
  )
target_link_libraries(dectris-neggia neggia_static)
set_target_properties(dectris-neggia PROPERTIES PREFIX "" SUFFIX ".so")
install(TARGETS dectris-neggia LIBRARY DESTINATION lib)
set(DECTRIS_NEGGIA_XDS_PLUGIN ${CMAKE_CURRENT_BINARY_DIR}/dectris-neggia.so CACHE INTERNAL "")
