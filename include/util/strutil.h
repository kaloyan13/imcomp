/** @file   strutil.h
 *  @brief  String utilities
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   16 Nov 2017
 */

#ifndef _STR_UTIL_H_
#define _STR_UTIL_H_

#include <iostream>
#include <string>

namespace util {

bool begins_with(const std::string str, const std::string prefix);
bool ends_with(const std::string str, const std::string suffix);
size_t replace_all_instances(std::string& str, const std::string from, const std::string to);
}

#endif
