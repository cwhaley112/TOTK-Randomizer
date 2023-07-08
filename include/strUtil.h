#ifndef STRUTIL_H
#define STRUTIL_H

// taken from https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring

#include <algorithm> 
#include <cctype>
#include <locale>

// trim from start (in place)
static inline void _ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void _rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void _trim(std::string &s) {
    _rtrim(s);
    _ltrim(s);
}

// trim from start 
static inline std::string ltrim(std::string s) {
    _ltrim(s);
    return s;
}

// trim from end 
static inline std::string rtrim(std::string s) {
    _rtrim(s);
    return s;
}

// trim from both ends 
static inline std::string trim(std::string s) {
    _trim(s);
    return s;
}

#endif