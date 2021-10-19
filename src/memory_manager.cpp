#include <iostream> // printf
#include <stdlib.h> // malloc, free
#include <string>   // stoi
#include <unistd.h> // sleep

#include "../include/memory_manager.hpp"

// Miembros estáticos

bool MemoryManager::showedMemDeallocMsg = false;
bool MemoryManager::showedGoodByeMsg = false;

// Métodos públicos

MemoryManager::MemoryManager() {}

MemoryManager::MemoryManager(int primaryMemSize, int blockSize, int processNum) {
  this->_primaryMemSize = primaryMemSize * sizeof(char);
  this->_secondaryMemSize = (primaryMemSize * 8) * sizeof(char);
  this->_primaryMemBeginning = (char **)malloc(sizeof(char **) * this->_primaryMemSize);
  this->_secondaryMemBeginning = (char *)malloc(sizeof(char) * this->_secondaryMemSize);
  this->_blockSize = blockSize;
  this->_processNum = processNum;

  for (int i = 0; i < this->_processNum; i++) {
    this->_primaryMemBeginning[i] = (char *)malloc(sizeof(char));
  }
}

MemoryManager::~MemoryManager() {
  if (!showedMemDeallocMsg) {
    printf("\nDesalojando memoria utilizada...\n");
    showedMemDeallocMsg = true;
  }
  sleep(2);

  for (int i = 0; i < this->_processNum; i++) {
    free(this->_primaryMemBeginning[i]);
  }

  free(this->_primaryMemBeginning);
  free(this->_secondaryMemBeginning);

  if (!showedGoodByeMsg) {
    printf("\nTenga un buen día.\n");
    showedGoodByeMsg = true;
  }
}

void MemoryManager::showDetails() {
  printf("Dirección del bloque inicial de la memoria primaria: %p\n", this->_primaryMemBeginning);
  printf("Dirección del bloque inicial de la memoria secundaria: %p\n\n", this->_secondaryMemBeginning);
}

void MemoryManager::beginProgram() {
  std::string opcionUsuario;

  printf("\nBienvenido al gestor de memoria\n");

  // Ejecutar hasta que el usuario pida salir
  while (true) {
    printf("\nPor favor, ingrese una de las siguientes opciones para continuar:\n\n");
    printf(" • Opción 1: Crear archivo.\n");
    // printf(" • Opción 2: Abrir archivo.\n");
    printf(" • Opción 2: Copiar archivo.\n");
    printf(" • Opción 3: Editar archivo.\n");
    // printf(" • Opción 5: Borrar archivo.\n");
    printf(" • Opción 4: Finalizar ejecución.\n\n");

    std::cin >> opcionUsuario;

    if (this->isNumber(opcionUsuario)) {
      if (!this->loadProcess(std::stoi(opcionUsuario))) {
        break;
      }
    } else {
      printf("\nPor favor, ingrese solamente las opciones numéricas enlistadas.\n");
    }
  }
}

bool MemoryManager::loadProcess(int processNum) {
  // Cargar el proceso requerido de la memoria secundaria a la memoria principal
  // void (MemoryManager::*ptrCreateFile)() = &MemoryManager::createFile;
  // std::cout << (void *&)ptrCreateFile << std::endl;

  switch (processNum) {
  // Cargar el proceso de crear un archivo
  case 1:
    // this->_primaryMemBeginning[0] = &createFile;
    this->createFile();
    return true;

  // Abrir archivo
  // case 2:
  //   this->openFile();
  //   return true;

  // copiar archivo
  case 2:
    this->copyFile();
    return true;

  case 3:
    this->editFile();
    return true;

    // case 5:
    //   this->deleteFile();
    //   return true;

  case 4:
    return false;

  default:
    printf("\nHa ingresado una opción incorrecta.\n");
    return true;
  }
}

void MemoryManager::unloadProcess(int processNum) { printf("El proceso %d ha sido descargado.", processNum); }

bool MemoryManager::checkMemSpace() { return true; }

int MemoryManager::getMemBlock() { return 1; }

void MemoryManager::createFile() {
  printf("\nCreando archivo...\n");
  sleep(1);
  printf("\nArchivo creado.\n");
}

void MemoryManager::copyFile() {
  printf("\nCopiando archivo...\n");
  sleep(1);
  printf("\nArchivo copiado.\n");
}

void MemoryManager::openFile() {
  printf("\nAbriendo archivo...\n");
  sleep(1);
  printf("\nArchivo abierto.\n");
}

void MemoryManager::editFile() {
  printf("\nEditando archivo...\n");
  sleep(1);
  printf("\nArchivo editado.\n");
}

void MemoryManager::deleteFile() {
  printf("\nBorrando archivo...\n");
  sleep(1);
  printf("\nArchivo borrado\n");
}

// Métodos privados

bool MemoryManager::isNumber(const std::string &str) {
  std::string::const_iterator iterator = str.begin();

  while (iterator != str.end() && std::isdigit(*iterator)) {
    iterator++;
  }

  return (!str.empty() && iterator == str.end());
}
