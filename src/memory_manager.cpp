#include <cstring>    // strcmp
#include <ctime>      // time_t, time
#include <dirent.h>   // opendir
#include <fstream>    // open, write, close
#include <iostream>   // printf, cin, cout
#include <pthread.h>  // pthread_create, pthread_join, pthread_t
#include <sstream>    // stringstream
#include <stdio.h>    // fopen, fclose
#include <stdlib.h>   // malloc, free
#include <string>     // stoi
#include <sys/stat.h> // mkdir
#include <unistd.h>   // sleep

#include "../include/memory_manager.hpp"

// Subrutinas

// Struct utilizado para la subrutina de copia
struct copyRoutineArgs {
  std::string originalFilePath;
  std::string copiedFileName;
};

// Subrutina de copia
void *copyRoutine(void *arg) {
  std::stringstream streamCopy; // Stream para la copia del contenido
  std::string originalFilePath; // Ruta del archivo original
  std::string copiedFileName;   // Nombre del archivo copiado

  // Argumentos de copia
  originalFilePath = ((copyRoutineArgs *)arg)->originalFilePath;
  copiedFileName = ((copyRoutineArgs *)arg)->copiedFileName;

  // Apertura del archivo original
  std::ifstream originalFile(originalFilePath);

  // Creación del archivo
  std::ofstream fileCopy("./Archivos/" + copiedFileName);

  // Copia del contenido del archivo
  streamCopy << originalFile.rdbuf();
  fileCopy << streamCopy.str();
  fileCopy.close();

  pthread_exit(NULL);
}

// Métodos públicos

MemoryManager::MemoryManager() {}

MemoryManager::MemoryManager(int primaryMemSize, int blockSize, int processNum) {
  this->_primaryMemSize = primaryMemSize * sizeof(char) * 1000;
  this->_secondaryMemSize = this->_primaryMemSize * 8;
  this->_primaryMemBeginning = malloc(this->_primaryMemSize);
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
  // Simulación del proceso de carga a la memoria principal
  switch (processNum) {
  // Cargar el proceso de crear un archivo
  case 1:
    this->_primaryMemBeginning = (void *)(void (*)()) & MemoryManager::createFile; // Simulación de carga del proceso de creación a la memoria principal
    ((void (*)(void))this->_primaryMemBeginning)();                                // Ejecución del proceso
    return true;

  case 2:
    // Cargar el proceso de copiar un archivo
    this->_primaryMemBeginning = (void *)(void (*)()) & MemoryManager::copyFile; // Simulación de carga del proceso de copia a la memoria principal
    ((void (*)(void))this->_primaryMemBeginning)();                              // Ejecución del proceso
    return true;

  case 3:
    // Cargar el proceso de editar un archivo
    this->_primaryMemBeginning = (void *)(void (*)()) & MemoryManager::editFile; // Simulación de carga del proceso de edición a la memoria principal
    ((void (*)(void))this->_primaryMemBeginning)();                              // Ejecución del proceso
    return true;

  case 4:
    return false;

  default:
    this->printColoredText("\nHa ingresado una opción incorrecta.\n", RED);
    return true;
  }
}

void MemoryManager::unloadProcess() {
  // Como se pasó una referencia hacia el método que había sido cargado a la memoria principal, se deshace la referencia para simular el proceso de descarga
  this->_primaryMemBeginning = nullptr;
}

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
  clock_t start_t, end_t;
  double total_t;
  start_t = clock();

  /*
    pthread_t tid;
    //printf("\nprueba2");
    int crear;
    bool archivo_existente;


    crear = pthread_create(&tid,NULL, &createFileThread,NULL);           //Creación del hilo
    if(crear != 0){
      printf("error");
    }
  */
  // Trasladar a subrutina

  std::system("clear");

  // declaracion de strings

  std::string rutaArchivo;
  std::string nombreArchivo;

  std::cout << std::endl << "Ingresa la ruta destino del archivo" << std::endl;
  std::cin >> rutaArchivo;

  // Declaracion de variables para la verificacion del directorio destino

  struct stat buffer;
  char *charRuta = &rutaArchivo[0];

  // Verificación de si el directorio ingresado es valido

  while (stat(charRuta, &buffer) != 0) {
    std::system("clear");
    std::cout << std::endl << "La ruta es invalida" << std::endl;

    std::cout << std::endl << "Ingresa la ruta destino del archivo" << std::endl;
    std::cin >> rutaArchivo;
    charRuta = &rutaArchivo[0];
  }

  std::system("clear");

  std::cout << std::endl << "Ingresa el nombre del archivo" << std::endl;
  std::cin >> nombreArchivo;

  // Declaracion de variables para la verificación de un archivo existente

  FILE *file;
  bool archivoValido = false;               // Condicion del while
  int nuevoNombre = 1;                      // contador para renombramiento
  std::string strNuevoNombre;               // string del contador
  std::string verificacion = nombreArchivo; // variable auxiliar

  std::system("clear");

  // Vefificacion de la existencia del archivo y renombramiento

  while (archivoValido == false) {

    // Se intenta abrir el archivo para verificar si existe
    if ((file = fopen((char *)("./Archivos/" + verificacion + ".txt").c_str(), "r"))) {
      strNuevoNombre = std::__cxx11::to_string(nuevoNombre); // Contador a string
      fclose(file);
      verificacion = (nombreArchivo + "_" + strNuevoNombre); // Variable auxiliar con el valor del contador
      nuevoNombre++;                                         // Aumento del contador para el siguiente ciclo
    } else {
      archivoValido = true;         // Cambio de la variable para finalizar el ciclo
      nombreArchivo = verificacion; // Nombre final del archivo

      // En el primer ciclo el contador permanece igual, si cambia, existe un renombramiento
      if (nuevoNombre != 1) {
        std::cout << std::endl << "El nombre del archivo ya existe, el archivo ha sido renombrado a: " << nombreArchivo << std::endl;
      }
    }
  }

  // nombreArchivo = nombreArchivo ;
  std::string rutaFinal = "./Archivos/" + nombreArchivo + ".txt";

  // Creación del archivo

  printf("\nCreando archivo...\n");
  std::ofstream archivo;
  archivo.open(rutaFinal, std::ofstream::out);
  archivo.close();
  printf("\nArchivo creado.\n");

  end_t = clock();
  total_t = difftime(end_t, start_t) / CLOCKS_PER_SEC;
  std::cout << "Tiempo Total utilizado: " << total_t << std::endl;

  int memBlock = this->getMemBlock();
  this->_addressesVector[memBlock].fileName = nombreArchivo;

  // Escribir la data en el mapa de memoria
  std::vector<std::string> copyFileMetadata = {"Título: " + nombreArchivo + ".txt", "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
  std::vector<std::string> copyFileData = this->getFileData("./Archivos/" + nombreArchivo);
  this->writeToMemMap(memBlock, copyFileMetadata, copyFileData);

  // std::cout<<std::endl<< rutaArchivo<<std::endl;
  std::cout << std::endl << "Se ha creado exitosamente el archivo: " << nombreArchivo << ".txt" << std::endl;

  // Mostrar detalles del proceso
  this->showProcessSummary(memBlock, nombreArchivo, total_t, "\nProceso ejecutado: Creación de Archivo\n");

  // Descargar el proceso
  this->unloadProcess();
}

void MemoryManager::copyFile() {
  std::system("clear");
  clock_t start_t, end_t;
  double total_t;
  start_t = clock();

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

          // Hilos de copia del contenido del archivo
          pthread_t threads[this->_processNum];

          // Verificar si no existe ya algún archivo con el mismo nombre
          if ((potentialFile = fopen((char *)("./Archivos/" + copiedFileName).c_str(), "r"))) {
            fclose(potentialFile);
            std::string newFileName = this->getValidFileName(copiedFileName);
            printf("\nYa existe un archivo con el nombre ");
            this->printColoredText(copiedFileName, BLUE);
            printf(", por lo que el archivo será guardado con el nombre: ");
            this->printColoredText(newFileName + "\n", BLUE);
            copiedFileName = newFileName;
          }

          // Creación del archivo
          std::ofstream fileCopy("./Archivos/" + copiedFileName);
          copyRoutineArgs args = {pathToFile, copiedFileName};

          // Ejecución del los hilos de copia
          for (int i = 0; i < this->_processNum; i++) {
            if (pthread_create(&threads[i], NULL, &copyRoutine, (void *)&args) != 0) {
              perror("Hubo un error al tratar de crear el hilo de copia.\n");
            }
          }

          // Join de los hilos de copia
          for (int i = 0; i < this->_processNum; i++) {
            if (pthread_join(threads[i], NULL) != 0) {
              perror("Hubo un error al tratar de unir el hilo de copia.\n");
            }
          }

          end_t = clock();
          total_t = difftime(end_t, start_t) / CLOCKS_PER_SEC;

          // Impresión del resumen del archivo creado
          int memBlock = this->getMemBlock();
          this->_addressesVector[memBlock].fileName = copiedFileName;
          this->showProcessSummary(memBlock, copiedFileName, total_t, "\nProceso ejecutado: Copia de Archivo (Desde el Sistema Operativo)\n");

          // Impresión del contenido copiado
          printf("\nContenido copiado:\n\"\n");
          this->printFirstFileSegment("./Archivos/" + copiedFileName);
          printf("\n\"\n");

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

        // Hilos de copia del contenido del archivo
        pthread_t threads[this->_processNum];

        // Verificar si no existe ya algún archivo con el mismo nombre
        if ((potentialFile = fopen((char *)("./Archivos/" + copiedFileName).c_str(), "r"))) {
          fclose(potentialFile);
          std::string newFileName = this->getValidFileName(copiedFileName);
          printf("\nYa existe un archivo con el nombre ");
          this->printColoredText(copiedFileName, BLUE);
          printf(", por lo que el archivo será guardado con el nombre: ");
          this->printColoredText(newFileName + "\n", BLUE);
          copiedFileName = newFileName;
        }

        // Creación del archivo
        std::ofstream fileCopy("./Archivos/" + copiedFileName);
        copyRoutineArgs args = {"./Archivos/" + copyFileName, copiedFileName};

        // Ejecución del los hilos de copia
        for (int i = 0; i < this->_processNum; i++) {
          if (pthread_create(&threads[i], NULL, &copyRoutine, (void *)&args) != 0) {
            perror("Hubo un error al tratar de crear el hilo de copia.\n");
          }
        }

        // Join de los hilos de copia
        for (int i = 0; i < this->_processNum; i++) {
          if (pthread_join(threads[i], NULL) != 0) {
            perror("Hubo un error al tratar de unir el hilo de copia.\n");
          }
        }

        end_t = clock();
        total_t = difftime(end_t, start_t) / CLOCKS_PER_SEC;

        // Impresión del resumen del archivo creado
        int memBlock = this->getMemBlock();
        this->_addressesVector[memBlock].fileName = copiedFileName;
        this->showProcessSummary(memBlock, copiedFileName, total_t, "\nProceso ejecutado: Copia de Archivo (Desde el Gestor de Memoria)\n");

        // Impresión del contenido copiado
        printf("\nContenido copiado:\n\"\n");
        this->printFirstFileSegment("./Archivos/" + copiedFileName);
        printf("\n\"\n");

        // Escribir la data en el mapa de memoria
        std::vector<std::string> copyFileMetadata = {"Título: " + copiedFileName, "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
        std::vector<std::string> copyFileData = this->getFileData("./Archivos/" + copiedFileName);
        this->writeToMemMap(memBlock, copyFileMetadata, copyFileData);
      }
    }
  } else {
    this->printColoredText("\nNo existe suficiente espacio para copiar otro archivo.\n", YELLOW);
  }

  printf("\nPresione enter para continuar...");
  std::cin.ignore();
  std::cin.get();
  system("clear");

  // Descargar el proceso
  this->unloadProcess();
}

void MemoryManager::openFile() {
  printf("\nAbriendo archivo...\n");
  sleep(1);
  printf("\nArchivo abierto.\n");
}

void MemoryManager::editFile() {
  clock_t start_t, end_t;
  double total_t;
  start_t = clock();

  printf("\nEscoja el directorio del archivo que desea editar. \nEscriba 'root' si el archivo está en el directorio raíz del sistema\n\n");
  std::string path;
  std::string archivo;
  int caso;
  std::string direccion;
  bool existe = true;

  // Ingresar el directorio del documento, root = Archivos
  std::cin >> path;

  printf("\nEscoja el nombre del archivo que desea editar\n\n");

  // Ingresar el nombre del archivo
  std::cin >> archivo;

  // Se guarda el path completo en la variable dirección
  if (path == "root") {
    direccion = "./Archivos/" + archivo;
  } else {
    direccion = "./Archivos/" + path + "/" + archivo;
  }

  while (true) {
    FILE *file;

    if ((file = fopen((char *)(direccion.c_str()), "r"))) {
      fclose(file);
      std::ifstream originalFile(direccion);
      std::stringstream streamCopy;

      break;

    } else {
      this->printColoredText("\nEl archivo especificado no existe. Intente crear un archivo con ese nombre primero.\n\n", YELLOW);
      // printf("\nEl archivo especificado no existe\nIntente de nuevo\n\n");
      existe = false;
      break;
    }
  }

  if (existe == true) {
    // cout<<"\nSe editará el archivo "<<direccion<<endl<<endl;
    this->printColoredText("\nSe editará el archivo " + direccion + " \n\n", CYAN);

    printf("\n¿Qué acción desea realizar?\n");
    printf("\nEscriba 1 para eliminar una parte del contenido del archivo\n");
    printf("\nEscriba 2 para agregar contenido al archivo\n\n");

    std::cin >> caso;

    // Ciclo hasta que se elija una opción correcta
    while (true) {
      if (caso == 1 || caso == 2) {
        break;
      } else {
        printf("\nPor Favor ingrese una opción correcta\n\n");
        std::cin >> caso;
      }
    }

    switch (caso) {

    // Eliminar el archivo
    case 1: {
      printf("\nEliminar contenido del archivo...\n");

      std::string input;
      std::string changed_content;
      std::string search;
      std::fstream file;
      std::string file_name = direccion;

      // Escoger una palabra existente en la línea que desea borrar
      std::cout << "\nEscriba un término de la línea que quiera eliminar:\n ";
      std::cin >> search;

      file.open(file_name, std::ios::in);

      if (file.is_open()) {
        while (!file.eof()) {
          getline(file, input);
          // Chequea si la línea existe
          if (input.find(search) == std::string::npos) {
            // Guarda el contenido en un string
            changed_content += input;
            changed_content += '\n';
          }

          input.clear(); // elimina el input
        }
        file.close();
        file.open(file_name, std::ios::out | std::ios::trunc); // Reabrir y limpiar
        if (file.is_open()) {
          file << changed_content; // Reescribir sin lo eliminado
          file.close();            // Cerrar el archivo
        } else {
          std::cout << "No se abrió el archivo para reescribir\n";
        }
      } else {
        std::cout << "No so abrió el archivo correctamente\n";
      }

      break;
    }

    case 2: {

      printf("\nAgregar contenido al archivo...\n");
      std::string s = "\n";
      std::ofstream file;

      std::cout << "\nPuede comenzar a escribir, cuando desee salir del modo de edición escriba '-' en una línea" << std::endl;

      // Se abre el archivo y se coloca al final del archivo
      file.open(direccion, std::ios::app);

      if (file.fail()) {
        std::cout << "No es posible abrir el archivo\n";
        exit(1); // Si falla, se sale del archivo
      }

      // Mientras el string no sea el caracter de salida seguir con el ciclo
      while (s != "-") {
        std::getline(std::cin, s);
        if (s != "-") {
          file << s << "\n"; // Escribir una nueva línea en el archivo
        }
      }
      file.close();

      break;
    }
    }

    std::vector<std::string> editFileMetadata = {"Título: " + archivo, "Autor: Anónimo", "Fecha de creación: ", "Fecha de modificación: "};
    std::vector<std::string> editFileData = this->getFileData(direccion);
    int memBlock = 0;

    for (int i = 0; i < (int)this->_addressesVector.size(); i++) {
      if (this->_addressesVector[i].fileName == archivo) {
        memBlock = i;
        break;
      }
    }

    this->writeToMemMap(memBlock, editFileMetadata, editFileData);
    this->printColoredText("\nArchivo editado correctamente\n\n", CYAN);
  }

  end_t = clock();
  total_t = difftime(end_t, start_t) / CLOCKS_PER_SEC;
  std::cout << "Tiempo Total utilizado: " << total_t << std::endl;

  // Descargar el proceso
  this->unloadProcess();
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
        true,                                                              // Inicialmente, todos los bloques estarán libres
        "",                                                                // Inicialmente, no hay ningún archivo en el bloque
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
  // Verificar si el bloque está libre
  bool blockIsFree = this->_addressesVector[blockNum].isFree;

  if (blockIsFree) {
    // Si el bloque está libre, se está creando o copiando un archivo, por lo que se agrega la nueva data al mapa de memoria

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
  } else {
    // No existe una forma de editar líneas específicas en C++, por lo que si se desea editar la información de un bloque del mapa de memoria,
    // se debe eliminar el archivo y volver a crearlo con la data nueva

    // Si el bloque no está libre, se está editando un archivo, por lo que es necesario reemplazar la data del mapa de memoria

    // Crear y abrir el archivo temporal del mapa de memoria
    std::ofstream memMapFile("./Mapa-de-Memoria-Temp.csv");

    // Abrir el archivo antiguo del mapa de memoria
    std::ifstream oldMemMapFile("./Mapa-de-Memoria.csv");

    int counter = 1;                                 // Contador de las líneas del mapa de memoria
    std::string line;                                // Línea actual del mapa de memoria
    int invalidLowerRange = blockNum * 510 + 2;      // Línea inicial de la data del bloque de memoria
    int invalidUpperRange = invalidLowerRange + 509; // Línea final de la data del bloque de memoria
    bool writtenData = false;                        // Bandera para indicar si se escribió la data del bloque de memoria

    while (std::getline(oldMemMapFile, line)) {
      if (invalidLowerRange <= counter && counter <= invalidUpperRange && !writtenData) { // Si la línea actual está dentro del rango de la data del bloque de memoria y aún no se ha escrito la información
        // Si la línea actual pertenece al bloque de memoria por editar, se sobrescribe con la data nueva

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

        writtenData = true; // Se escribió la data del bloque de memoria
      } else {
        if (counter < invalidLowerRange || invalidUpperRange < counter) {
          // Si la línea actual no pertenece al bloque de memoria por editar, se escribe la data correspondiente
          memMapFile << line << std::endl;
        }
      }
      counter++;
    }
    oldMemMapFile.close();
    memMapFile.close();

    // Borrar el mapa de memoria antiguo
    std::remove("./Mapa-de-Memoria.csv");

    // Renombrar el mapa de memoria temporal como el mapa de memoria
    std::rename("./Mapa-de-Memoria-Temp.csv", "./Mapa-de-Memoria.csv");
  }
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
    potentialFileName = fileName.substr(0, fileName.find(".txt")) + "_";
  } else {
    potentialFileName = fileName + "_";
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
  system(("rm -f " + savePath + "/Archivos\\ del\\ Gestor\\ de\\ Memoria/*").c_str());
  if (system(("cp -r ./Archivos/* \"" + savePath + "/Archivos del Gestor de Memoria\"").c_str()) < 0) {
    printf("\nLos archivos no pudieron ser guardados en %s\n", (savePath + "/Archivos del Gestor de Memoria").c_str());
  } else {
    printf("\nLos archivos fueron guardados exitosamente en %s\n", (savePath + "/Archivos\\ del\\ Gestor\\ de\\ Memoria/").c_str());
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

void MemoryManager::showProcessSummary(int memBlock, std::string fileName, double totalTime, std::string processName) {
  void *fileInitialAddress = static_cast<void *>(this->_addressesVector[memBlock].initialAddress); // Dirección inicial del bloque de memoria
  void *fileFinalAddress = static_cast<void *>(this->_addressesVector[memBlock].finalAddress);     // Dirección final del bloque de memoria
  struct stat stat_buf;                                                                            // Estructura para obtener información del archivo
  int rc = stat(("./Archivos/" + fileName).c_str(), &stat_buf);                                    // Obtener información del archivo
  int size = rc == 0 ? stat_buf.st_size : 0;                                                       // Obtener tamaño del archivo
  std::stringstream ss;                                                                            // Buffer de lectura de la dirección inicial del bloque
  std::stringstream ss2;                                                                           // Buffer de lectura de la dirección final del bloque

  // Conversión de la dirección de los bloques a strings
  ss << fileInitialAddress;
  ss2 << fileFinalAddress;

  // Impresión del resumen del proceso
  this->printColoredText("\nResumen del Proceso:", GREEN);
  this->printColoredText("\nNombre del archivo: ", GREEN);
  this->printColoredText(fileName, GREEN);
  this->printColoredText("\nDirección inicial del archivo: ", GREEN);
  this->printColoredText(ss.str(), GREEN);
  this->printColoredText("\nDirección final del archivo: ", GREEN);
  this->printColoredText(ss2.str(), GREEN);
  this->printColoredText("\nTamaño del archivo: ", GREEN);
  this->printColoredText(std::to_string(size) + " bytes", GREEN);
  this->printColoredText(processName, GREEN);
  this->printColoredText("\nRendimiento:", MAGENTA);
  this->printColoredText("\nTiempo promedio de lectura y escritura: " + std::to_string(totalTime) + " segs.", MAGENTA);
  this->printColoredText("\nBytes promedio de lectura y escritura: " + std::to_string(size / totalTime) + " B/s\n", MAGENTA);
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
