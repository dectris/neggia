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

#include <dectris/neggia/data/H5FilterMsg.h>
#include <gtest/gtest.h>

TEST(TestH5FilterMsgV1, CanBeParsed) {
    // see "Filter Pipeline Message - Version 1"
    // https://support.hdfgroup.org/HDF5/doc/H5.format.html#FilterMessage
    // clang-format off
    const unsigned char data[100] = {
        0x01, 0x01, 0x00, 0x00, // version 1, number of filters 1
        0x00, 0x00, 0x00, 0x00,
        0x08, 0x7d, 0x40, 0x00, // filter id 32008, name length 64
        0x00, 0x00, 0x05, 0x00, // flags 0x0000, number client data values 5
        0x62, 0x69, 0x74, 0x73, // name
        0x68, 0x75, 0x66, 0x66,
        0x6c, 0x65, 0x3b, 0x20,
        0x73, 0x65, 0x65, 0x20,
        0x68, 0x74, 0x74, 0x70,
        0x73, 0x3a, 0x2f, 0x2f,
        0x67, 0x69, 0x74, 0x68,
        0x75, 0x62, 0x2e, 0x63,
        0x6f, 0x6d, 0x2f, 0x6b,
        0x69, 0x79, 0x6f, 0x2d,
        0x6d, 0x61, 0x73, 0x75,
        0x69, 0x2f, 0x62, 0x69,
        0x74, 0x73, 0x68, 0x75,
        0x66, 0x66, 0x6c, 0x65,
        0x00, 0x00, 0x00, 0x00, // null-terminated and padded to 8-byte multiple
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, // client data #1: 0
        0x02, 0x00, 0x00, 0x00, // client data #2: 2
        0x02, 0x00, 0x00, 0x00, // client data #3: 2
        0x00, 0x00, 0x00, 0x00, // client data #4: 0
        0x02, 0x00, 0x00, 0x00, // client data #5: 2
    };
    // clang-format on
    const auto msg = H5FilterMsg((const char*)data, 0);
    ASSERT_EQ(msg.version(), 1);
    ASSERT_EQ(msg.nFilters(), 1);
    ASSERT_EQ(msg.filterName(0),
              "bitshuffle; see https://github.com/kiyo-masui/bitshuffle");
    ASSERT_EQ(msg.filterId(0), 32008);
    ASSERT_EQ(msg.clientData(0), (std::vector<int32_t>{0, 2, 2, 0, 2}));
}

TEST(TestH5FilterMsgV2, CanBeParsed) {
    // see "Filter Pipeline Message - Version 2"
    // https://support.hdfgroup.org/HDF5/doc/H5.format.html#FilterMessage
    // clang-format off
    const unsigned char data[87] = {
        0x02, 0x01,             // version 2, number of filters 1
        0x08, 0x7d, 0x39, 0x00, // filter id 32008, name length 57
        0x00, 0x00, 0x05, 0x00, // flags 0x0000 (not optional), number client data values 5
        0x62, 0x69, 0x74, 0x73, // name (not null terminated)
        0x68, 0x75, 0x66, 0x66,
        0x6c, 0x65, 0x3b, 0x20,
        0x73, 0x65, 0x65, 0x20,
        0x68, 0x74, 0x74, 0x70,
        0x73, 0x3a, 0x2f, 0x2f,
        0x67, 0x69, 0x74, 0x68,
        0x75, 0x62, 0x2e, 0x63,
        0x6f, 0x6d, 0x2f, 0x6b,
        0x69, 0x79, 0x6f, 0x2d,
        0x6d, 0x61, 0x73, 0x75,
        0x69, 0x2f, 0x62, 0x69,
        0x74, 0x73, 0x68, 0x75,
        0x66, 0x66, 0x6c, 0x65,
        0x00,                   // no padding to multiple of eight as in version 1
        0x00, 0x00, 0x00, 0x00, // client data #1: 0
        0x03, 0x00, 0x00, 0x00, // client data #2: 3
        0x02, 0x00, 0x00, 0x00, // client data #3: 2
        0x00, 0x00, 0x00, 0x00, // client data #4: 0
        0x02, 0x00, 0x00, 0x00, // client data #5: 2
    };
    // clang-format on
    const auto msg = H5FilterMsg((const char*)data, 0);
    ASSERT_EQ(msg.version(), 2);
    ASSERT_EQ(msg.nFilters(), 1);
    ASSERT_EQ(msg.filterName(0),
              "bitshuffle; see https://github.com/kiyo-masui/bitshuffle");
    ASSERT_EQ(msg.filterId(0), 32008);
    ASSERT_EQ(msg.clientData(0), (std::vector<int32_t>{0, 3, 2, 0, 2}));
}