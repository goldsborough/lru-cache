#ifndef LRU_ERRORS_HPP
#define LRU_ERRORS_HPP

#include <exception>
#include <string>

namespace LRU {

struct NotMonitoredError : public std::runtime_error {
  using super = std::runtime_error;
  NotMonitoredError(const std::string& what =
                        "Requested key was not registered for monitoring")
  : super(what) {
  }
};
}

#endif /* LRU_ERRORS_HPP*/
