#include <cstring>    // strcmp
#include <dirent.h>   // opendir
#include <fstream>    // open, write, close
#include <iostream>   // printf, cin, cout
#include <sstream>    // stringstream
#include <stdio.h>    // fopen, fclose
#include <stdlib.h>   // malloc, free
#include <string>     // stoi
#include <sys/stat.h> // mkdir
#include <unistd.h>   // sleep

#include "../include/memory_manager.hpp"

// Métodos públicos

MemoryManager::MemoryManager() {}

MemoryManager::MemoryManager(int primaryMemSize, int blockSize, int processNum) {
  this->_primaryMemSize = primaryMemSize * sizeof(char) * 1000;
  this->_secondaryMemSize = this->_primaryMemSize * 8;
  this->_primaryMemBeginning = (char *)malloc(sizeof(char) * this->_primaryMemSize);
  this->_secondaryMemBeginning = (char *)malloc(sizeof(char) * this->_secondaryMemSize);
  this->_secondaryMemBeginning = this->_secondaryMemBeginning - 8;
  this->_blockSize = blockSize * 1000;
  this->_availableBlocks = this->_secondaryMemSize / this->_blockSize;
  this->_processNum = processNum;
  this->filesDirectoryExists = this->checkIfFilesDirExists();

  this->setAvailableAddresses(); // Poblar vector con las memorias disponibles
  this->createMemMap();          // Crear mapa de memoria con base en las direcciones diponibles
}

MemoryManager::~MemoryManager() {
  int createdFiles = 0;

  for (int i = 0; i < (int)this->_addressesVector.size(); i++) {
    if (!this->_addressesVector[i].isFree) {
      createdFiles++;
    }
  }

  if (createdFiles > 0) {
    std::string response;
    std::vector<std::string> affirmativeResponses = {"Sí", "sí", "Si", "si", "sÍ", "SÍ", "sI", "SI"};

    createdFiles == 1 ? printf("\nEl gestor de memoria cuenta con 1 archivo creado. ¿Desea guardarlo (Sí/No)?: ") : printf("\nEl gestor de memoria cuenta con %d archivos creados. ¿Desea guardarlos (Sí/No)?: ", createdFiles);
    std::cin >> response;

    if (this->isInVector(response, affirmativeResponses)) {
      this->saveCreatedFiles();
    }

    // Eliminar los archivos creados
    system("rm -f ./Archivos/*");
  }

  printf("\nDesalojando memoria utilizada...\n");
  sleep(2);

  // Desalojamiento de los bloques de memoria primaria y secundaria
  free(this->_primaryMemBeginning);
  free(this->_secondaryMemBeginning + 8);

  printf("\nTenga un buen día.\n");
}

void MemoryManager::showDetails() {
  printf("\033[1;32m\nDirección del bloque inicial de la memoria primaria: %p\n", this->_primaryMemBeginning);
  printf("Dirección del bloque inicial de la memoria secundaria: %p\n\033[0m", this->_secondaryMemBeginning);
}

void MemoryManager::beginProgram() {
  std::string opcionUsuario;

  printf("\nBienvenido al gestor de memoria\n");

  // Ejecutar hasta que el usuario pida salir
  while (true) {
    printf("\nPor favor, ingrese una de las siguientes opciones para continuar:\n\n");
    printf(" • Opción 1: Crear archivo.\n");
    printf(" • Opción 2: Copiar archivo.\n");
    printf(" • Opción 3: Editar archivo.\n");
    printf(" • Opción 4: Finalizar ejecución.\n\n");
    std::cin >> opcionUsuario;

    if (this->isNumber(opcionUsuario)) {
      if (!this->loadProcess(std::stoi(opcionUsuario))) {
        break;
      }
    } else {
      this->printColoredText("\nPor favor, ingrese solamente las opciones numéricas enlistadas.\n", RED);
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

  case 2:
    this->copyFile();
    return true;

  case 3:
    this->editFile();
    return true;

  case 4:
    return false;

  default:
    this->printColoredText("\nHa ingresado una opción incorrecta.\n", RED);
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

  // Verificar que existen bloques disponibles de memoria
  if (this->checkMemSpace()) {
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
          this->printColoredText("\nHa elegido una opción incorrecta.\n\n", RED);
        }
      } else {
        std::system("clear");
        this->printColoredText("\nPor favor, ingrese solamente las opciones numéricas enlistadas.\n\n", RED);
      }
    }

    std::system("clear");

    if (opcion == 1) {
      // Copiar desde el sistema operativo
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
          printf("\n¿Cuál quiere que sea el nombre del archivo de copia?: ");
          std::cin >> copiedFileName;
          FILE *potentialFile;

          // Verificar si no existe ya algún archivo con el mismo nombre
          if ((potentialFile = fopen((char *)("./Archivos/" + copiedFileName).c_str(), "r"))) {
            fclose(potentialFile);
            copiedFileName = this->getValidFileName(copiedFileName);
            // printf("\nYa existe un archivo con ese nombre. Por favor, escoja otro nombre.\n");
          }

          // Creación del archivo
          std::ofstream fileCopy("./Archivos/" + copiedFileName);

          // Escribir el contenido del archivo original en el archivo de copia y luego, cerrarlo
          streamCopy << originalFile.rdbuf();
          fileCopy << streamCopy.str();
          fileCopy.close();

          // Impresión del contenido copiado
          printf("\nContenido copiado:\n\"\n");
          this->printFirstFileSegment("./Archivos/" + copiedFileName);
          printf("\n\"\n");
          // this->printColoredText(streamCopy.str(), CYAN);

          // Escribir la data en el mapa de memoria
          std::vector<std::string> copyFileMetadata = {"Título: " + copiedFileName, "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
          std::vector<std::string> copyFileData = this->getFileData("./Archivos/" + copiedFileName);
          this->writeToMemMap(this->getMemBlock(), copyFileMetadata, copyFileData);

          // Cerrar el arhivo de copia y salir de la iteración del proceso
          break;

        } else {
          printf("El archivo especificado no existe.\n");
        }
      }
    } else {
      // Copiar desde el gestor de memoria
      std::string copyFileName; // Nombre del archivo por copiar
      bool availableFiles = false;

      for (int i = 0; (int)this->_addressesVector.size() > i; i++) {
        if (!this->_addressesVector[i].isFree) {
          availableFiles = true;
          break;
        }
      }

      if (!availableFiles) {
        this->printColoredText("\nNo existen archivos que copiar. Si desea copiar uno, cree uno antes o copiélo desde el sistema operativo.\n", YELLOW);
      } else {
        // Ciclo para mostrar los archivos existentes y que el usuario escoja uno de ellos para copiarlo
        while (true) {
          // Lectura de los nombres de los archivos guardados
          DIR *dir;
          dir = opendir("./Archivos");
          struct dirent *ent;
          ent = readdir(dir);
          FILE *potentialFile;

          printf("\nArchivos existentes dentro del gestor de memoria:\n");

          while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
              printf("\n • %s", ent->d_name);
            }
          }
          printf("\n\nPor favor, ingrese uno de los archivos anteriores para copiarlo: ");
          std::cin >> copyFileName;

          if ((potentialFile = fopen((char *)("./Archivos/" + copyFileName).c_str(), "r"))) {
            break; // El archivo existe, puede ser copiado
          } else {
            // El archivo no existe, se vuelve a preguntar
            system("clear");
            this->printColoredText("\nNo existe un archivo con el nombre especificado.\n", RED);
          }
        }

        // Preguntar el nombre de la copia del archivo
        std::string copiedFileName;

        printf("\n¿Cuál quiere que sea el nombre del archivo de copia?: ");
        std::cin >> copiedFileName;
        FILE *potentialFile;
        std::stringstream streamCopy;
        std::ifstream originalFile("./Archivos/" + copyFileName);

        // Verificar si no existe ya algún archivo con el mismo nombre
        if ((potentialFile = fopen((char *)("./Archivos/" + copiedFileName).c_str(), "r"))) {
          fclose(potentialFile);
          copiedFileName = this->getValidFileName(copiedFileName);
          // printf("\nYa existe un archivo con ese nombre. Por favor, escoja otro nombre.\n");
        }

        // Creación del archivo
        std::ofstream fileCopy("./Archivos/" + copiedFileName);

        // Escribir el contenido del archivo original en el archivo de copia y luego, cerrarlo
        streamCopy << originalFile.rdbuf();
        fileCopy << streamCopy.str();
        fileCopy.close();

        // Impresión del contenido copiado
        printf("\nContenido copiado:\n\"\n");
        // this->printColoredText(streamCopy.str(), BLUE);
        this->printFirstFileSegment("./Archivos/" + copiedFileName);
        printf("\n\"\n");

        // Escribir la data en el mapa de memoria
        std::vector<std::string> copyFileMetadata = {"Título: " + copiedFileName, "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
        std::vector<std::string> copyFileData = this->getFileData("./Archivos/" + copiedFileName);
        this->writeToMemMap(this->getMemBlock(), copyFileMetadata, copyFileData);
      }
    }
  } else {
    this->printColoredText("\nNo existe suficiente espacio para copiar otro archivo.\n", YELLOW);
  }

  printf("\nPresione enter para continuar...");
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

  // Desfases de direcciones de la metadata y data
  int metadataOffset = 0;
  int dataOffset = 0;

  // Escribir al mapa de memoria las localidades presentes
  for (int i = 0; i < 10; i++) { // Ciclo de escritura de las localidades de la metadata
    std::stringstream ss;        // Stringstream para almacenar la data de cada localidad de la metadata
    // Dirección de la localidad actual
    void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * blockNum) + metadataOffset);
    ss << localityAddress << ",0,";                                                        // Datos de la localidad
    memMapFile << ss.str() << (i >= (int)metadata.size() ? "" : metadata[i]) << std::endl; // Escribir al mapa de memoria
    metadataOffset += 100;                                                                 // Desfase de la localidad actual
  }

  for (int i = 0; i < 500; i++) { // Ciclo de escritura de las localidades de la data del mapa de memoria
    std::stringstream ss;         // Stringstream para almacenar la data de cada localidad de la data
    // Dirección de la localidad actual
    void *localityAddress = static_cast<void *>(this->_secondaryMemBeginning + (this->_blockSize * blockNum) + metadataOffset + dataOffset);
    ss << localityAddress << ",1,";                                                       // Datos de la localidad
    memMapFile << ss.str() << (i >= (int)data.size() ? "|||||||" : data[i]) << std::endl; // Escribir al mapa de memoria
    dataOffset += 8;                                                                      // Desfase de la localidad actual
  }

  // Designar el bloque de memoria como ocupado
  this->_addressesVector[blockNum].isFree = false;

  // Cerrar el archivo del mapa de memoria
  memMapFile.close();
}

bool MemoryManager::checkIfFilesDirExists() {
  FILE *potentialFile;

  if ((potentialFile = fopen((char *)("./Archivos"), "r"))) {
    // El directorio de archivos existe
    fclose(potentialFile);

    // Eliminar los archivos que existían anteriormente
    system("rm -f ./Archivos/*");

    return true;
  } else {
    // El archivo no existe
    return false;
  }
}

void MemoryManager::createFilesDir() {
  if (mkdir("./Archivos", 0777)) { // Verificar si fue posible crear el direcotorio de archivos si no existe
    printf("No fue posible crear el directorio para almacenar los archivos.\n\n");
  } else {
    this->filesDirectoryExists = true;
  }
}

std::vector<std::string> MemoryManager::getFileData(std::string fileName) {
  std::vector<std::string> fileData;                           // Vector para almacenar la data del archivo
  std::ifstream file(fileName);                                // Archivo
  std::string line;                                            // Linea actual del archivo
  std::stringstream ss;                                        // Stringstream para almacenar la data de cada linea del archivo
  std::string localityData = "";                               // Data de la localidad actual
  int maxFileSize = this->_blockSize - (this->_blockSize / 5); // Tamaño máximo del archivo

  while (std::getline(file, line)) { // Ciclo de lectura de las lineas del archivo
    ss << line;
  }

  // Ciclo de escritura de las localidades del archivo
  // En caso de que el contenido del archivo supere el tamaño máximo del bloque,
  // se truncará el archivo y se copiará solo el tamaño máximo del bloque
  for (int i = 0; i < (int)ss.str().size() && i < maxFileSize; i++) {
    if ((int)localityData.size() < 7) {
      localityData += ss.str()[i];
    } else {
      for (int j = 0; j < (int)localityData.size(); j++) {
        if (localityData[j] == '\n' || localityData[j] == '\r') {
          localityData[j] = '\\';
        }
      }
      fileData.push_back(localityData);
      localityData = "";
      localityData += ss.str()[i];
    }
  }
  for (int j = 0; j < (int)localityData.size(); j++) {
    if (localityData[j] == '\n' || localityData[j] == '\r') {
      localityData[j] = '\\';
    }
  }

  while ((int)localityData.size() < 7) {
    localityData += '|';
  }

  fileData.push_back(localityData);
  file.close();

  return fileData;
}

void MemoryManager::printColoredText(std::string text, TextColor color) {
  std::string RESET = "\033[0m"; // Código ANSI para terminar de imprimir el color

  switch (color) {
  case 0:
    printf("\033[0m%s%s", text.c_str(), RESET.c_str()); // Negro
    break;
  case 1:
    printf("\033[1;31m%s%s", text.c_str(), RESET.c_str()); // Rojo
    break;
  case 2:
    printf("\033[1;32m%s%s", text.c_str(), RESET.c_str()); // Verde
    break;
  case 3:
    printf("\033[1;33m%s%s", text.c_str(), RESET.c_str()); // Amarillo
    break;
  case 4:
    printf("\033[1;34m%s%s", text.c_str(), RESET.c_str()); // Azul
    break;
  case 5:
    printf("\033[1;35m%s%s", text.c_str(), RESET.c_str()); // Magenta
    break;
  case 6:
    printf("\033[1;36m%s%s", text.c_str(), RESET.c_str()); // Cyan
    break;
  case 7:
    printf("\033[1;37m%s%s", text.c_str(), RESET.c_str()); // Blanco
    break;
  default:
    printf("\033[1;37m%s%s", text.c_str(), RESET.c_str()); // Blanco
    break;
  }
}

std::string MemoryManager::getValidFileName(std::string fileName) {
  FILE *potentialFile;
  int counter = 1;
  std::string potentialFileName;

  // Verificar si el nombre del archivo contiene la extensión .txt
  if (fileName.find(".txt") != std::string::npos) {
    potentialFileName = fileName.substr(0, fileName.find(".txt")) + "-copia-";
  } else {
    potentialFileName = fileName + "-copia-";
  }

  while ((potentialFile = fopen((char *)("./Archivos/" + potentialFileName + std::to_string(counter) + ".txt").c_str(), "r"))) {
    fclose(potentialFile);
    counter++;
  }

  return potentialFileName + std::to_string(counter) + ".txt";
}

void MemoryManager::saveCreatedFiles() {
  std::string savePath;

  while (true) {
    printf("\n¿En cuál ruta del sistema operativo desea guardar los archivos?: ");
    std::cin >> savePath;

    DIR *dir = opendir(savePath.c_str());
    if (dir) {
      closedir(dir);
      break;
    } else {
      system("clear");
      this->printColoredText("\nLa ruta ingresada no existe.\n", RED);
    }
  }

  // Copiar los archivos del gestor de memoria a la ruta indicada
  mkdir((savePath + "/Archivos del Gestor de Memoria").c_str(), 0777);
  // printf("%s", ("cp -r ./Archivos/* " + savePath + "/Archivos del Gestor de Memoria").c_str());
  if (system(("cp -r ./Archivos/* \"" + savePath + "/Archivos del Gestor de Memoria\"").c_str()) < 0) {
    printf("\nLos archivos no pudieron ser guardados en %s\n", (savePath + "/Archivos del Gestor de Memoria").c_str());
  } else {
    printf("\nLos archivos fueron guardados exitosamente en %s\n", (savePath + "/Archivos del Gestor de Memoria").c_str());
  }
}

void MemoryManager::printFirstFileSegment(std::string fileName) {
  std::ifstream file(fileName); // Archivo
  std::stringstream buffer;     // Buffer de lectura

  buffer << file.rdbuf();
  if ((int)buffer.str().size() <= 100) {
    this->printColoredText(buffer.str(), CYAN);
  } else {
    std::string contents = buffer.str().substr(0, 100);
    this->printColoredText(contents + "\n...", CYAN);
  }

  file.close();
}

// Métodos privados

bool MemoryManager::isNumber(const std::string &str) {
  std::string::const_iterator iterator = str.begin();

  while (iterator != str.end() && std::isdigit(*iterator)) {
    iterator++;
  }

  return (!str.empty() && iterator == str.end());
}

bool MemoryManager::isInVector(std::string element, std::vector<std::string> vector) {
  for (int i = 0; i < (int)vector.size(); i++) {
    if (vector[i] == element) {
      return true;
    }
  }
  return false;
}
