// SPDX-License-Identifier: MIT

#ifndef H5PATH_H
#define H5PATH_H
#include <string>
#include <vector>

class H5Path {
public:
    using const_iterator = std::vector<std::string>::const_iterator;

    H5Path(const std::string& path);
    H5Path(const H5Path& other, const_iterator start);
    H5Path operator+(const H5Path& other) const;
    bool isAbsolute() const;
    operator std::string() const;
    const_iterator begin() const;
    const_iterator end() const;

private:
    std::vector<std::string> splitPathStringIntoComponents(
            const std::string& path) const;
    std::vector<std::string> _path;
    bool _isAbsolute;
};

#endif  // H5PATH_H
