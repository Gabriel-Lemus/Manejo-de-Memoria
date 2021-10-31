#include <fstream>    // open, write, close
#include <iostream>   // printf
#include <sstream>    // stringstream
#include <stdio.h>    // fopen, fclose
#include <stdlib.h>   // malloc, free
#include <string>     // stoi
#include <sys/stat.h> // mkdir
#include <unistd.h>   // sleep

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
  this->filesDirectoryExists = this->checkIfFilesDirExists();

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

bool MemoryManager::checkMemSpace() {
  // Iterar a través del vector de direcciones revisando si existe algún bloque de memoria libre
  for (int i = 0; i < (int)this->_addressesVector.size(); i++) {
    if (this->_addressesVector[i].isFree) {
      return true; // Hay un bloque de memoria disponible
    }
  }

  return false; // No hay bloques de memoria disponible
}

int MemoryManager::getMemBlock() {
  // Iterar a través del vector de direcciones revisando si existe algún bloque de memoria libre
  for (int i = 0; i < (int)this->_addressesVector.size(); i++) {
    if (this->_addressesVector[i].isFree) {
      return i; // Retornar la dirección del bloque de memoria disponible
    }
  }

  return -1; // Se retorna -1 si no se encontró ningún bloque
}

void MemoryManager::createFile() {
  printf("\nCreando archivo...\n");
  sleep(1);
  printf("\nArchivo creado.\n");
}

void MemoryManager::copyFile() {
  std::system("clear");

  std::string opcionUsuario; // Opción de copia del usuario
  int opcion;                // Opción numérica de copia del usuario

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

        // Verificar si el directorio de archivos existe y si no, crearlo
        if (!this->filesDirectoryExists) {
          this->createFilesDir();
        }
        std::string copiedFileName;

        // Obtener el nombre del archivo de copia
        system("clear");

        // Iterar hasta obtener un nombre de archivo que no exista
        while (true) {
          printf("\n¿Cuál quiere que sea el nombre del archivo de copia?: ");
          std::cin >> copiedFileName;

          // Verificar si no existe ya algún archivo con el mismo nombre
          if (fopen((char *)("./Archivos/" + copiedFileName).c_str(), "r")) {
            printf("\nYa existe un archivo con ese nombre. Por favor, escoja otro nombre.\n");
          } else {
            break;
          }
        }

        // Creación del archivo
        std::ofstream fileCopy("./Archivos/" + copiedFileName);

        // Escribir el contenido del archivo original en el archivo de copia y luego, cerrarlo
        streamCopy << originalFile.rdbuf();
        printf("\nContenido copiado: \n\"\n\033[1;34m%s\033[0m\n\"\n", streamCopy.str().c_str());
        fileCopy << streamCopy.str();
        fileCopy.close();

        // Escribir la data en el mapa de memoria
        std::vector<std::string> copyFileMetadata = {"Titulo: " + copiedFileName, "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
        std::vector<std::string> copyFileData = this->getFileData("./Archivos/" + copiedFileName);
        this->writeToMemMap(this->getMemBlock(), copyFileMetadata, copyFileData);

        // Cerrar el arhivo de copia y salir de la iteración del proceso
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

  printf("\nPresione enter para continuar.\n");
  std::cin.ignore();
  std::cin.get();
  system("clear");
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

  // Escritura del encabezado del archivo csv y cierre
  memMapFile << "dirección,tipoDato,info\n";
  memMapFile.close();
}

void MemoryManager::writeToMemMap(int blockNum, std::vector<std::string> metadata, std::vector<std::string> data) {
  // Abrir el archivo del mapa de memoria
  std::ofstream memMapFile("./Mapa-de-Memoria.csv", std::ios_base::app);

  // Escribir al mapa de memoria las localidades presentes
  for (int i = 0; i < 10; i++) { // Ciclo de escritura de las localidades de la metadata
    std::stringstream ss;        // Stringstream para almacenar la data de cada localidad de la metadata
    // Dirección de la localidad actual
    void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * blockNum) + (i * 100));
    ss << localityAddress << ",0,";                     // Datos de la localidad
    memMapFile << ss.str() << (i >= (int)metadata.size() ? "" : metadata[i]) << std::endl; // Escribir al mapa de memoria
  }

  for (int i = 0; i < 500; i++) { // Ciclo de escritura de las localidades de la data del mapa de memoria
    std::stringstream ss;         // Stringstream para almacenar la data de cada localidad de la data
    // Dirección de la localidad actual
    void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * blockNum) + (i * 8));
    ss << localityAddress << ",1,";                 // Datos de la localidad
    memMapFile << ss.str() << (i >= (int)data.size() ? "" : data[i]) << std::endl; // Escribir al mapa de memoria
  }

  // Cerrar el archivo del mapa de memoria
  memMapFile.close();
}

bool MemoryManager::checkIfFilesDirExists() { return (fopen((char *)("./Archivos"), "r")); }

void MemoryManager::createFilesDir() {
  if (mkdir("./Archivos", 0777)) { // Verificar si fue posible crear el direcotorio de archivos si no existe
    printf("No fue posible crear el directorio para almacenar los archivos.\n\n");
  }
}

std::vector<std::string> MemoryManager::getFileData(std::string fileName) {
  std::vector<std::string> fileData;
  std::ifstream file(fileName);
  std::string line;
  std::stringstream ss;
  std::string localityData = "";

  while (std::getline(file, line)) {
    ss << line;
  }

  for (int i = 0; i < (int)ss.str().size(); i++) {
    if ((int)localityData.size() < 8) {
      if (ss.str()[i] != '\n') {
        localityData += ss.str()[i];
      } else {
        localityData += "\n";
      }
    } else {
      fileData.push_back(localityData);
      localityData = "";

      if (ss.str()[i] != '\n') {
        localityData += ss.str()[i];
      } else {
        localityData += "\n";
      }
    }
  }
  fileData.push_back(localityData);

  return fileData;
}

// Métodos privados

bool MemoryManager::isNumber(const std::string &str) {
  std::string::const_iterator iterator = str.begin();

  while (iterator != str.end() && std::isdigit(*iterator)) {
    iterator++;
  }

  return (!str.empty() && iterator == str.end());
}
