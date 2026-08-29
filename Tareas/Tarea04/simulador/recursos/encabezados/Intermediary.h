/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Intermediario.h: Definición de la clase Intermediario
  *
  * (Fedora version)
  *
 **/

#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

// Bibliotecas incluidas
#include "Bitacora.h"
#include "Queue.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <thread>
#include <atomic>

// Constantes enumeradas definidas para determinar el estado actual del intermediario
enum class Status {
    Waiting,
    Processing,
    Stopped
};

// Clase Intermediario
class Intermediary {
 public:
  // Método constructor de la clase Intermediario
  Intermediary(Queue &, Queue &, Queue &, Queue &);
  // Método destructor de la clase Intermediario
  ~Intermediary();
  // Método que permite iniciar el hilo del intermediario
  void start();
  // Método que permite detener el hilo del intermediario
  void stop();

 private:
  // Identificador del intermediario
  int id;
  // Variable atómica definida para almacenar el estado actual del intermediario
  std::atomic<Status> status;
  // Hilo definido para ejecutar el intermediario
  std::thread thread;
  // Cola en la que se encuentran las solicitudes para al servidor
  Queue &queueToServer;
  // Cola en la que se encuentran las respuestas recibidas desde el servidor
  Queue &queueFromServer;
  // Cola en la que se encuentran las respuestas para el cliente
  Queue &queueToClient;
  // Cola en la que se encuentran las solicitudes del cliente
  Queue &queueFromClient;
  Bitacora &bitacora;

  // Método que ejecuta el hilo del intermediario
  void execute();
  Message processRequest(const Message&);
};

#endif
