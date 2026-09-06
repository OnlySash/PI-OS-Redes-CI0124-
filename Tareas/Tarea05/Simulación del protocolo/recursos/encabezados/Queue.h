/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Cola.h: Definición de la estructura de cola
  *
  * (Fedora version)
  *
 **/

#ifndef QUEUE_H
#define QUEUE_H

// Bibliotecas incluidas
#include "Message.h"
#include <queue>
#include <mutex>
#include <condition_variable>


class Queue {
 public:
  // Guarda un mensaje entrante dentro de la cola y lo notifica al hilo consumidor
  void enqueue(const Message &msg);
  // Bloquea al hilo consumidor hasta que haya un mensaje disponible.
  Message dequeue();

 private:
  // Cola definida para almacenar los mensajes entrantes
  std::queue<Message> bufer;
  // Candado definido para proteger la cola de condiciones de carrera
  std::mutex lock;
  // Señal definida para notificar al hilo consumidor que hay un mensaje disponible
  std::condition_variable haveElements;
};

#endif
