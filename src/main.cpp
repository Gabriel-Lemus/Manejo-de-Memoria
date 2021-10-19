#include <iostream>

#include "../include/memory_manager.hpp"

int main() {
  MemoryManager memManager(10, 16, 4);
  memManager.showDetails();
  memManager.beginProgram();

  return 0;
}
