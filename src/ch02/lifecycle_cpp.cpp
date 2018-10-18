
#include <chrono>
#include <iostream>
#include <thread>

void
thread_routine()
{
  std::cout << "Inside thread" << std::this_thread::get_id() << "\n";
}

int
main(int argc, char* argv[])
{
  std::cout << "Inside thread" << std::this_thread::get_id() << "\n";

  std::thread t(thread_routine);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "Main thread created thread " << t.get_id() << "\n";

  t.join();
}
