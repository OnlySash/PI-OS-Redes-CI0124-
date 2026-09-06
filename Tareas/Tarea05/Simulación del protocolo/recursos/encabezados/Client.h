/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Cliente.h: Definición de la clase Cliente
  *
  * (Fedora version)
  *
 **/

#ifndef CLIENT_H
#define CLIENT_H

// Bibliotecas incluidas
#include "Bitacora.h"
#include "Queue.h"
#include <cstring>
#include <thread>
#include <atomic>
#include <iostream>

// Clase Intermediario
class Client {
 public:
  // Método constructor de la clase Cliente
  Client(Queue &, Queue &, Bitacora &);
  // Método destructor de la clase Cliente
  ~Client();
  // Método que permite iniciar el hilo del cliente
  void start(const Message &);
  // Método que permite detener el hilo del cliente
  void stop();

 private:
  // Cola en la que se encuentran las solicitudes para el intermediario
  Queue &queueToIntermediary;
  // Cola en la que se encuentran las respuestas recibidas desde el intermediario
  Queue &queueFromIntermediary;
  // Referencia a la bitácora del proyecto
  Bitacora &bitacora;
  // Variable atómica definida para almacenar el estado actual del cliente
  std::atomic<Status> status;
  // Hilo definido para ejecutar el cliente
  std::thread thread;

  // Método que ejecuta el hilo del cliente
  void execute(const Message &);
};

#endif
