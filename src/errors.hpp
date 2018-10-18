
#ifndef SRC_ERRORS_HPP_
#define SRC_ERRORS_HPP_

#include <exception>
#include <iostream>

#define err_abort(code, text)                                                  \
  do {                                                                         \
    std::cerr << text << " at \"" << __FILE__ << ":" << __LINE__               \
              << ": error no:" << code << std::endl;                           \
    std::terminate();                                                          \
  } while (false)

#define errno_abort(text)                                                      \
  do {                                                                         \
    std::cerr << text << " at \"" << __FILE__ << ":" << __LINE__ << std::endl; \
    std::terminate();                                                          \
  } while (false)

#endif  // SRC_ERRORS_HPP_
