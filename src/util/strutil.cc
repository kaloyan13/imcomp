#include "util/strutil.h"

bool util::begins_with(const std::string str, const std::string prefix) {
  if ( str.substr(0, prefix.length()) == prefix ) {
    return true;
  } else {
    return false;
  }
}

size_t util::replace_all_instances(std::string& str, const std::string from, const std::string to) {
  size_t pos = str.find(from);
  size_t replace_count = 0;
  while( pos != std::string::npos ) {
    str.replace(pos, from.length(), to);
    replace_count++;
    pos = str.find(from);
  }
  return replace_count;
}

bool util::ends_with(const std::string str, const std::string suffix) {
  /*
  std::cout << "\nends_with() invoked" << std::flush;

  for ( size_t i=0; i<str.length(); i++ ) {
    std::cout << "\nstr[" << i << "] = " << str[i] << std::flush;
  }

  std::cout << "\nsuffix = {" << suffix << "} len=" << suffix.length() << std::flush;

  std::string s = str.substr( pos );
  std::cout << "\npos=" << pos << ", substr = {" << s << "} len=" << s.length() << std::flush;
  */
  size_t pos = str.length() - suffix.length();
  if ( str.substr( pos ) == suffix ) {
    return true;
  } else {
    return false;
  }
}
