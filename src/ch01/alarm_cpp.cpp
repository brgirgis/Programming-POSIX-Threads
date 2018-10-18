
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

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

    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::cout << message << "\n";
  }
}
