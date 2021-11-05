#pragma once

#include <vector>

// Struct que representa un bloque de memoria
struct MemBlock {
  int blockSize;        // Tamaño del bloque en KB
  char *initialAddress; // Dirección inicial del bloque
  char *finalAddress;   // Dirección final del bloque
  bool isFree;          // Indica si el bloque está libre o no
  std::string fileName; // Nombre del archivo
};

// Enums de los textos de colores
enum TextColor { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

// Clase que representa al gestor de memoria
class MemoryManager {
  // =========================== Miembros públicos ===========================
public:
  // Constructor default
  MemoryManager();

  // Método constructor
  MemoryManager(int primaryMemSize, int blockSize, int processNum);

  // Método destructor
  ~MemoryManager();

  // Imprimir la dirección del bloque inicial de memoria primaria y secundaria
  void showDetails();

  // Inicia la ejecución del programa, preguntando al usuario qué opción desea realizar
  void beginProgram();

  // Carga el proceso seleccionado de memoria secundaria a la primaria. Retorna un bool inidicando si el programa debe continuar o finalizar
  bool loadProcess(int processNum);

  // Descarga el proceso de memoria principal, liberando la memoria que había sido alojada
  void unloadProcess(int processNum);

  // Verificar si queda suficiente espacio en la memoria secundaria
  bool checkMemSpace();

  // Obtener el primer bloque de memoria libre en la memoria secundaria
  int getMemBlock();

  // Proceso para crear un archivo
  void createFile();

  // Proceso para abrir un archivo
  void openFile();

  // Proceso para copiar un archivo
  void copyFile();

  // Proceso para editar un archivo
  void editFile();

  // Proceso para borrar un archivo
  void deleteFile();

  // Preparar vector con las direcciones disponibles en la memoria secundaria
  void setAvailableAddresses();

  // Crear mapa de memoria
  void createMemMap();

  // Escribir al mapa de memoria
  void writeToMemMap(int blockNum, std::vector<std::string> metadata, std::vector<std::string> data);

  // Verificar si el directorio de archivos existe
  bool checkIfFilesDirExists();

  // Crear el directorio de archivos
  void createFilesDir();

  // Obtener la data del archivo especificado en un vector de strings
  std::vector<std::string> getFileData(std::string fileName);

  // Imprimir el contenido de colores
  void printColoredText(std::string text, TextColor color);

  // Obtener nombre válido de archivo en caso de que exista un archivo con el mismo nombre (e.g. 'archivo.txt' -> 'archivo-copia-1.txt')
  std::string getValidFileName(std::string fileName);

  // Guardar los archivos del gestor de memoria al sistema operativo
  void saveCreatedFiles();

  // Imprimir los primeros 100 caracteres de un archivo
  void printFirstFileSegment(std::string fileName);

  // =========================== Miembros privados ===========================
private:
  // Atributos
  int _primaryMemSize;                    // Tamaño de la memoria principal (en Kilobytes)
  int _secondaryMemSize;                  // Tamaño de la memoria secundaria (8 veces la memoria primaria)
  int _blockSize;                         // Tamaño de cada bloque en la memoria (en Kilobytes)
  char *_primaryMemBeginning;             // Dirección inicial de la memoria principal
  char *_secondaryMemBeginning;           // Dirección inicial de la memoria secundaria
  int _processNum;                        // Número de procesos
  int _availableBlocks;                   // Cantidad de bloques de memoria disponibles
  std::vector<MemBlock> _addressesVector; // Vector de direcciones disponibles en memoria
  bool filesDirectoryExists;              // Bool que indica si el directorio de archivos existe

  // Métodos

  // Verifica si un string puede ser transformado en un entero
  bool isNumber(const std::string &str);

  // Verifica si un elemento se encuentra dentro de un arreglo de strings
  bool isInVector(std::string element, std::vector<std::string> vector);
};
