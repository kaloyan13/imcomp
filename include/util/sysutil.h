/** @file   sysutil.h
 *  @brief  System utilities
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   17 Nov 2017
 */

#ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>

namespace util {

std::string execute(std::string cmd);

}

#endif
