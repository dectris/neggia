// SPDX-License-Identifier: MIT

#ifndef H5ERROR_H
#define H5ERROR_H
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

template <class... ARGS>
struct StringConcatenator;
template <class HEAD, class... TAIL>
struct StringConcatenator<HEAD, TAIL...> {
    static std::string getString(HEAD head, TAIL... tail);
};
template <>
struct StringConcatenator<> {
    static std::string getString() { return ""; }
};
template <class HEAD, class... TAIL>
std::string StringConcatenator<HEAD, TAIL...>::getString(HEAD head,
                                                         TAIL... tail) {
    std::stringstream ss;
    ss << head << StringConcatenator<TAIL...>::getString(tail...);
    return ss.str();
}

class H5Error : public std::runtime_error {
public:
    H5Error(int errorCode, const std::string& message)
          : std::runtime_error(message), _errorCode(errorCode) {}
    template <class... OUTPUT_ARGS>
    H5Error(int errorCode, OUTPUT_ARGS... args)
          : H5Error(errorCode,
                    StringConcatenator<OUTPUT_ARGS...>::getString(args...)) {}
    const char* what() const throw() { return std::runtime_error::what(); }
    int getErrorCode() const { return _errorCode; }

private:
    int _errorCode;
};

#endif  // H5ERROR_H
