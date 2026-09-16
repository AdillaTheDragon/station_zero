#include <cstdlib>
#include <iostream>

#include "game.h"

int main() {
#ifdef _WIN32
  std::system("chcp 65001 > nul");
#endif

  Game game("assets");

  if (!game.Load()) {
    std::cout << "Не удалось запустить игру.\n";
    return 1;
  }

  game.Run();
  return 0;
}
