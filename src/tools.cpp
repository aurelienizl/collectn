// src/tools.cpp
#include "tools.h"

#include <cctype>
#include <cerrno>
#include <fstream>
#include <sstream>

int readLines(const std::string &path, std::vector<std::string> &lines)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return -1;
    std::string line;
    while (std::getline(ifs, line))
    {
        lines.push_back(std::move(line));
    }
    return 0;
}

void trim(std::string &s)
{
    size_t i = 0, j = s.size();
    while (i < j && std::isspace((unsigned char)s[i]))
        ++i;
    while (j > i && std::isspace((unsigned char)s[j - 1]))
        --j;
    s = s.substr(i, j - i);
}

bool splitKeyVal(const std::string &line, std::string &key, std::string &val)
{
    auto pos = line.find(':');
    if (pos == std::string::npos)
        return false;
    key = line.substr(0, pos);
    val = line.substr(pos + 1);
    trim(key);
    trim(val);
    return true;
}

int readSingleValue(const std::string &path, int &out)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return -1;
    long v;
    if (!(ifs >> v))
        return -1;
    out = static_cast<int>(v);
    return 0;
}