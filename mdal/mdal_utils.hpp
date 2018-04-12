/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_UTILS_HPP
#define MDAL_UTILS_HPP

#include <string>
#include <vector>

namespace MDAL {

bool file_exists(const std::string& filename);

class String: public std::string {
public:
   String();
   String(const std::string& str);
   bool startsWith(const std::string& substr);
   std::vector<String> split(const std::string& delimiter);
   int toInt();
   double toDouble();

};

} // namespace MDAL
#endif //MDAL_UTILS_HPP
