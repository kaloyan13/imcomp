#include "util/sysutil.h"

// source: https://stackoverflow.com/a/478960/7814484
std::string util::execute(std::string cmd) {
  std::array<char, 128> buffer;
  std::ostringstream cmd_output;

  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");

  int c;

  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 80, pipe.get()) != nullptr) {
      cmd_output << buffer.data();
    }
  }
/*
  int index = 0;
  do {
    c = getc( pipe.get() );
    cmd_output << (char) (c);
    std::cout << "\nutil::execute() : " << index << ":" << c << ":" << (char)(c) << std::flush;
    index++;
  } while( c!= EOF );
*/
  return cmd_output.str();
}
