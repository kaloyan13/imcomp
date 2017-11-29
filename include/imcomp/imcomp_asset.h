/** @file   imcomp_asset.h
 *  @brief  static resources (html, css, js, etc) of imcomp
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   29 Nov 2017
 */

#ifndef _IMCOMP_ASSET_H_
#define _IMCOMP_ASSET_H_

#include <map>
#include <string>

namespace imcomp {
namespace asset {

extern const std::map<std::string, std::string> files_;

}
}
#endif
