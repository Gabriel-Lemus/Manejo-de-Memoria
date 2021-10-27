#pragma once

#include <vector>

// Struct que representa un bloque de memoria
struct MemBlock {
  int blockSize;        // Tamaño del bloque en KB
  char *initialAddress; // Dirección inicial del bloque
  char *finalAddress;   // Dirección final del bloque
  bool isFree;          // Indica si el bloque está libre o no
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
  void writeToMemMap();

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

  // Métodos

  // Verifica si un string puede ser transformado en un entero
  bool isNumber(const std::string &str);
};
