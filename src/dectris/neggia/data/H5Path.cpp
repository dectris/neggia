// SPDX-License-Identifier: MIT

#include "H5Path.h"
#include <assert.h>
#include <sstream>

H5Path::H5Path(const std::string& path)
      : _isAbsolute(!path.empty() && path[0] == '/'),
        _path(splitPathStringIntoComponents(path)) {}

H5Path::H5Path(const H5Path& other, H5Path::const_iterator start)
      : _isAbsolute(false) {
    for (; start != other.end(); ++start) {
        _path.push_back(*start);
    }
}

H5Path H5Path::operator+(const H5Path& other) const {
    assert(!other.isAbsolute());
    H5Path returnValue(*this);
    for (auto item : other) {
        returnValue._path.push_back(item);
    }
    return returnValue;
}

bool H5Path::isAbsolute() const {
    return _isAbsolute;
}

H5Path::const_iterator H5Path::begin() const {
    return _path.cbegin();
}

H5Path::const_iterator H5Path::end() const {
    return _path.cend();
}

H5Path::operator std::string() const {
    std::string returnValue(_isAbsolute ? "/" : "");
    if (_path.size() > 0) {
        for (size_t i = 0; i < _path.size() - 1; ++i)
            returnValue += _path[i] + "/";
        returnValue += _path[_path.size() - 1];
    }
    return returnValue;
}

std::vector<std::string> H5Path::splitPathStringIntoComponents(
        const std::string& path) const {
    std::stringstream pathStream(path);
    std::string item;
    std::vector<std::string> returnValue;
    while (std::getline(pathStream, item, '/')) {
        if (!item.empty())
            returnValue.push_back(item);
    }
    return returnValue;
}
