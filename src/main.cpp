#include <iostream>

#include "../include/memory_manager.hpp"

int main() {
  MemoryManager memManager(15, 5, 4);
  memManager.showDetails();
  memManager.beginProgram();

  return 0;
}
