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

#include "H5Path.h"
#include <assert.h>
#include <sstream>

H5Path::H5Path(const std::string &path):
    _isAbsolute(!path.empty() && path[0] == '/'),
    _path(splitPathStringIntoComponents(path))
{}

H5Path::H5Path(const H5Path &other, H5Path::const_iterator start):
    _isAbsolute(false)
{
    for(;start != other.end(); ++start) {
        _path.push_back(*start);
    }
}

H5Path H5Path::operator+(const H5Path &other) const
{
    assert(!other.isAbsolute());
    H5Path returnValue(*this);
    for(auto item: other) {
        returnValue._path.push_back(item);
    }
    return returnValue;
}

bool H5Path::isAbsolute() const
{
    return _isAbsolute;
}

H5Path::const_iterator H5Path::begin() const
{
    return _path.cbegin();
}

H5Path::const_iterator H5Path::end() const
{
    return _path.cend();
}

H5Path::operator std::string() const
{
    std::string returnValue(_isAbsolute? "/" : "");
    if(_path.size() > 0) {
        for(size_t i=0; i<_path.size()-1; ++i) returnValue += _path[i] + "/";
        returnValue += _path[_path.size()-1];
    }
}

std::vector<std::string> H5Path::splitPathStringIntoComponents(const std::string &path) const
{
    std::stringstream pathStream(path);
    std::string item;
    std::vector<std::string> returnValue;
    while(std::getline(pathStream, item, '/')) {
        if(!item.empty()) returnValue.push_back(item);
    }
    return returnValue;
}

