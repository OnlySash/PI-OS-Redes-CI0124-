/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Servidor.h: Definición de la clase Servidor
  *
  * (Fedora version)
  *
 **/
 
#ifndef SERVER_H
#define SERVER_H
 
// Bibliotecas incluidas
#include "Bitacora.h"
#include "Queue.h"
#include "Product.h"
#include <thread>
#include <atomic>
#include <string>
#include <vector>

// Constantes enumeradas definidas para determinar el estado actual del servidor
enum class StatusServer {
  Waiting,
  Processing,
  Stopped
};
 
// Clase Servidor
class Server {
 public:
  // Método constructor de la clase Servidor
  Server(int, Queue &, Queue &, Bitacora &); // Método destructor de la clase Servidor
  ~Server();
  // Método que permite iniciar el hilo del servidor
  void start();
  // Método que permite detener el hilo del servidor
  void stop();
 
 private:
  // Variable atómica definida para almacenar el estado actual del servidor
  std::atomic<StatusServer> status;
  // Hilo definido para ejecutar el servidor
  std::thread thread;
  // Cola en la que se encuentran las respuestas creadas para el intermediario
  Queue &queueToIntermediary;
  // Cola en la que se encuentran las solicitudes enviadas desde el intermediario
  Queue &queueFromIntermediary;
  // Método que ejecuta el hilo del servidor
  std::vector<Product> stock;
  Bitacora &bitacora;
  std::fstream archivo;

  void loadStock(const std::string &url);
  Message processRequest(const Message &request);
  Message processRequest(const Message&);
  // Método para responder ante una solicitud inválida
  Message invalidMessage(const Message&);
  // Método para reportar la información de un producto
  Message consultProduct(const Message&);
  // Método para reportar la información de los productos de una categoría
  Message listCategory(const Message&);
  // Método para reportar la información de todos los productos
  Message listProducts(const Message&);
  
  void execute();

  private:
    std::string url;
    
};
 
#endif
