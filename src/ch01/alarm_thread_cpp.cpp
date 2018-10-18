
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

struct Alarm {
  unsigned seconds = -1;
  std::string message;
};

using AlarmPtr = std::unique_ptr<Alarm>;

void
alarmHandler(AlarmPtr alarm)
{
  std::this_thread::sleep_for(std::chrono::seconds(alarm->seconds));
  std::cout << alarm->message << "\n";
}

int
main(int argc, char* argv[])
{
  std::string line;

  while (true) {
    std::cout << "Alarm> ";

    std::getline(std::cin, line);
    if (!std::cin.good())
      exit(0);

    if (line.empty())
      continue;

    std::stringstream in(line);

    AlarmPtr alarm(new Alarm());
    in >> alarm->seconds;
    alarm->message = in.str();

    if (alarm->seconds < 1 || alarm->message.empty()) {
      std::cout << "Bad command\n";
      continue;
    }

    std::thread t(alarmHandler, std::move(alarm));
    t.detach();
  }
}
