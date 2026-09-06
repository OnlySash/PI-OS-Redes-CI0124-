#include "Client.h"

Client::Client(Queue &queueToIntermediary, Queue &queueFromIntermediary, Bitacora &bitacora) :
    queueToIntermediary(queueToIntermediary),
    queueFromIntermediary(queueFromIntermediary),
    bitacora(bitacora),
    status(Status::Waiting) {}

Client::~Client() {}

void Client::start(const Message &message) {
  // Registro de bitácora: inicialización
  this->bitacora.registrarAccion("Client", "inicialización");
  // Inicializa y ejecuta el hilo del Client
  this->thread = std::thread(&Client::execute, this, message);
}
 
void Client::stop() {
  // Registro de bitácora: detención
  this->bitacora.registrarAccion("Client", "detención");
  // Cambia el estado del Cliente a detenido
  this->status = Status::Stopped;
  // Espera a que el hilo del Cliente termine su ejecución
  if (this->thread.joinable()) {
    this->thread.join();
  }
}

void Client::execute(const Message &request) {
  // Se declara que el Client está procesando la solicitud del cliente
  this->status = Status::Processing;
  // Se encola el mensaje creado para el intermediario
  this->queueToIntermediary.enqueue(request);
  // Registro de bitácora: encolamiento de mensaje
  this->bitacora.registrarAccion("Client", "encolamiento de mensaje");
  // Se declara que el cliente está esperando su respuesta
  this->status = Status::Waiting;
  // Recibe la respuesta desde el intermediario
  Message answer = this->queueFromIntermediary.dequeue();
  // Registro de bitácora: desencolamiento de mensaje
  this->bitacora.registrarAccion("Client", "desencolamiento de mensaje");
  // Reporta el mensaje recibido
  std::cout << answer.content << std::endl;
  // Registro de bitácora: mensaje recibido
  this->bitacora.registrarAccion("Client", "mensaje recibido");
  // Detiene la ejecución del cliente
  this->status = Status::Stopped;
}
