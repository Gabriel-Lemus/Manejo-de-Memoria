#include <fstream>  // open, write, close
#include <iostream> // printf
#include <sstream>  // stringstream
#include <stdio.h>  // fopen, fclose
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
  this->_primaryMemSize = primaryMemSize * sizeof(char) * 1000;
  this->_secondaryMemSize = this->_primaryMemSize * 8;
  this->_primaryMemBeginning = (char **)malloc(sizeof(char **) * this->_primaryMemSize);
  this->_secondaryMemBeginning = (char *)malloc(sizeof(char) * this->_secondaryMemSize);
  this->_blockSize = blockSize * 1000;
  this->_availableBlocks = this->_secondaryMemSize / this->_blockSize;
  this->_processNum = processNum;

  for (int i = 0; i < this->_processNum; i++) {
    this->_primaryMemBeginning[i] = (char *)malloc(sizeof(char));
  }

  this->setAvailableAddresses(); // Poblar vector con las memorias disponibles
  this->createMemMap();          // Crear mapa de memoria con base en las direcciones diponibles
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
  printf("\nDirección del bloque inicial de la memoria primaria: %p\n", this->_primaryMemBeginning);
  printf("Dirección del bloque inicial de la memoria secundaria: %p\n", this->_secondaryMemBeginning);
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
  std::system("clear");

  std::string opcionUsuario; // Opción de copia del usuario
  int opcion;                // Opción numérica de copia del usuario
  bool fileCopied;           // El archivo fue copiado exitosamente

  // Preguntarle al usuario de donde quiere copiar el archivo hasta que responda correctamente
  while (true) {
    printf("¿De dónde desea copiar el archivo?:\n\n");
    printf(" • Opción 1: Desde el sistema operativo.\n");
    printf(" • Opción 2: Desde el gestor de memoria.\n\n");
    std::cin >> opcionUsuario;

    if (this->isNumber(opcionUsuario)) {
      opcion = std::stoi(opcionUsuario);
      if (opcion == 1 || opcion == 2) {
        break;
      } else {
        std::system("clear");
        printf("\nHa elegido una opción incorrecta.\n\n");
      }
    } else {
      std::system("clear");
      printf("\nPor favor, ingrese solamente las opciones numéricas enlistadas.\n\n");
    }
  }

  std::system("clear");

  if (opcion == 1) {
    while (true) {
      std::string pathToFile; // Ruta del archivo por copiar
      FILE *file;

      printf("\nPor favor, indique la ruta del archivo por copiar: ");
      std::cin >> pathToFile;

      if ((file = fopen((char *)(pathToFile.c_str()), "r"))) {
        fclose(file);
        std::ifstream originalFile(pathToFile);
        std::stringstream streamCopy;

        printf("Copiando el archivo.\n\n");

        streamCopy << originalFile.rdbuf();
        std::cout << "\"\n" << streamCopy.str() << "\n\"" << std::endl;
        fileCopied = true;

        break;
      } else {
        printf("El archivo especificado no existe.\n");
      }
    }
  } else {
    bool availableFiles = false;

    for (int i = 0; i < (int)this->_addressesVector.size(); i++) {
      if (!this->_addressesVector[i].isFree) { // Si existe un bloque no disponible, quiere decir que existe algún archivo que puede ser copiado
        availableFiles = true;
      }
    }

    if (!availableFiles) {
      printf("\nNo existen archivos que copiar. Si desea copiar uno, cree uno antes o copiélo desde el sistema operativo.\n");
    } else {
      printf("\nExisten archivos disponibles.\n");
    }
  }

  if (fileCopied) {
    printf("\nCopiando archivo...\n");
    sleep(1);
    printf("\nArchivo copiado.\n");
  }

  // std::system("clear");
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

void MemoryManager::setAvailableAddresses() {
  int blockOffset = 0; // Desfase de la dirección actual

  for (int i = 0; i < this->_availableBlocks; i++) { // Ciclo de establecimiento de los bloques disponibles
    MemBlock newMemBlock{
        this->_blockSize,                                                  // Tamaño del bloque
        this->_secondaryMemBeginning + blockOffset,                        // Dirección de memoria inicial del bloque
        this->_secondaryMemBeginning + ((this->_blockSize * (i + 1)) - 1), // Dirección de memoria final del bloque
        true                                                               // Inicialmente, todos los bloques estarán libres
    };
    this->_addressesVector.push_back(newMemBlock);
    blockOffset += this->_blockSize;
  }
}

void MemoryManager::createMemMap() {
  // Crear y abrir el archivo del mapa de memoria
  std::ofstream memMapFile("./Mapa-de-Memoria.csv");

  // Escritura del encabezado del archivo csv
  memMapFile << "dirección,tipoDato,info\n";

  // Escribir al mapa de memoria las localidades presentes
  for (int i = 0; i < this->_availableBlocks; i++) { // Ciclo de escritura de los datos de cada bloque de memoria
    // Desfases de la metadata y data
    int metadataOffset = 0;
    int dataOffset = 0;
    for (int j = 0; j < 10; j++) { // Ciclo de escritura de las localidades de la metadata
      std::stringstream ss;        // Stringstream para almacenar la data de cada localidad de la metadata
      // Dirección de la localidad actual
      void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * i) + metadataOffset);
      ss << localityAddress << ",0,";      // Datos de la localidad
      memMapFile << ss.str() << std::endl; // Escribir al mapa de memoria
      metadataOffset += 100;               // Agregar al desfase de memoria para la siguiente localidad
    }

    for (int k = 0; k < 500; k++) { // Ciclo de escritura de las localidades de la data del mapa de memoria
      std::stringstream ss;         // Stringstream para almacenar la data de cada localidad de la data
      // Dirección de la localidad actual
      void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * i) + metadataOffset + dataOffset);
      ss << localityAddress << ",1,";      // Datos de la localidad
      memMapFile << ss.str() << std::endl; // Escribir al mapa de memoria
      dataOffset += 8;
    }
  }

  // Cerrar el archivo del mapa de memoria
  memMapFile.close();
}

// Métodos privados

bool MemoryManager::isNumber(const std::string &str) {
  std::string::const_iterator iterator = str.begin();

  while (iterator != str.end() && std::isdigit(*iterator)) {
    iterator++;
  }

  return (!str.empty() && iterator == str.end());
}
