#pragma once

#include <vector>

// Struct que representa un bloque de memoria
struct MemBlock {
  int blockSize;        // Tamaño del bloque en KB
  char *initialAddress; // Dirección inicial del bloque
  char *finalAddress;   // Dirección final del bloque
  bool isFree;          // Indica si el bloque está libre o no
};

// Enums de los textos de colores
enum TextColor {
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE
};

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

  // Miembros estáticos
  static bool showedMemDeallocMsg; // Establece si se ha mostrado el mensaje de desalojamiento de la memoria al llamar al destructor de la clase
  static bool showedGoodByeMsg;    // Bool que establece si se ha mostrado el mensaje de salida al ejecutar el destructor de la clase

  // =========================== Miembros privados ===========================
private:
  // Atributos
  int _primaryMemSize;                    // Tamaño de la memoria principal (en Kilobytes)
  int _secondaryMemSize;                  // Tamaño de la memoria secundaria (8 veces la memoria primaria)
  int _blockSize;                         // Tamaño de cada bloque en la memoria (en Kilobytes)
  char **_primaryMemBeginning;            // Dirección inicial de la memoria principal
  char *_secondaryMemBeginning;           // Dirección inicial de la memoria secundaria
  int _processNum;                        // Número de procesos
  int _availableBlocks;                   // Cantidad de bloques de memoria disponibles
  std::vector<MemBlock> _addressesVector; // Vector de direcciones disponibles en memoria
  bool filesDirectoryExists;              // Bool que indica si el directorio de archivos existe

  // Métodos

  // Verifica si un string puede ser transformado en un entero
  bool isNumber(const std::string &str);
};
