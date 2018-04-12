/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_utils.hpp"
#include <fstream>

bool MDAL::file_exists(const std::string& filename) {
    std::ifstream in(filename);
    return in.good();
}


MDAL::String::String()
    :std::string()
{
}

MDAL::String::String(const std::string &str)
:std::string(str)
{
}

bool MDAL::String::startsWith(const std::string &substr)
{
    return rfind(substr, 0) == 0;
}

std::vector<MDAL::String> MDAL::String::split(const std::string &delimiter)
//https://stackoverflow.com/a/44495206/2838364
{
    std::string str(*this);

    std::vector<String> list;
    size_t pos = 0;
    String token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = String(str.substr(0, pos));
        list.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    list.push_back(str);
    return list;
}

int MDAL::String::toInt()
{
    return atoi(c_str());
}

double MDAL::String::toDouble()
{
    return atof( c_str() );
}
