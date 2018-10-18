
#include "errors.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int
main(int argc, char* argv[])
{
  std::string line;
  std::string message;

  while (true) {
    std::cout << "Alarm> ";

    std::getline(std::cin, line);
    if (!std::cin.good())
      exit(0);

    if (line.empty())
      continue;

    std::istringstream in(line);

    unsigned seconds = -1;
    in >> seconds;
    message = in.str();

    if (seconds < 1 || message.empty()) {
      std::cout << "Bad command\n";
      continue;
    }

    pid_t pid = fork();
    if (pid == (pid_t) -1)
      errno_abort("Fork");
    if (pid == (pid_t) 0) {
      std::this_thread::sleep_for(std::chrono::seconds(seconds));
      std::cout << message << "\n";
      std::exit(0);
    }

    do {
      pid = waitpid((pid_t) -1, NULL, WNOHANG);
      if (pid == (pid_t) -1)
        errno_abort("Wait for child");
    } while (pid != (pid_t) 0);
  }
}
