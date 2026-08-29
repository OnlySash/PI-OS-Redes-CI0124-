#include "Queue.h"
 
void Queue::enqueue(const Message &msg) {
  // Zona crítica en la que se agregan nuevos mensajes a la cola
  {
    // Activa el candado para evitar condiciones de carrera
    std::lock_guard<std::mutex> lock(this->lock);
    // Agrega el mensaje entrante a la cola
    this->bufer.push(msg);
  }
  // Notifica al hilo consumidor que hay un mensaje disponible
  this->haveElements.notify_one();
}
 
Message Queue::dequeue() {
  // Activa el candado para evitar condiciones de carrera
  std::unique_lock<std::mutex> lock(this->lock);
  // Se pone en espera el hilo consumidor hasta que haya un mensaje disponible y devuelve el acceso a la cola
  this->haveElements.wait(lock, [this] { return !this->bufer.empty(); });
  // Toma el primer mensaje de la cola
  Message msg = this->bufer.front();
  // Quita el mensaje de la cola
  this->bufer.pop();
  // Retorna el mensaje entrante
  return msg;
}
