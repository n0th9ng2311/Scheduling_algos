#include "choice.h"

int main() {
  std::cout << "1) FCFS\n"
            << "2) RR\n"
            << "3) SJF\n"
            << "4) SRJF\n"
            << "5) Priority\n"
            << "Please choose your scheduling of choice: ";

  int choice{};
  std::cin >> choice;

  start(choice);

  return 0;
}