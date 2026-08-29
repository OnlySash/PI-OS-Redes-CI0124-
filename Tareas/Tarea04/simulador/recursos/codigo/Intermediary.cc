#include "Intermediary.h"
 
Intermediary::Intermediary(Queue &queueToServer, Queue &queueFromServer, Queue &queueToClient, Queue &queueFromClient) :
    queueToServer(queueToServer),
    queueFromServer(queueFromServer),
    queueToClient(queueToClient),
    queueFromClient(queueFromClient) { 

  this->status = Status::Waiting;
    }

Intermediary::~Intermediary() {}

void Intermediary::start() {
  // Inicializa y ejecuta el hilo del intermediario
    this->bitacora.registrarAccion("Intermediario", "inicialización");
  this->thread = std::thread(&Intermediary::execute, this);
}
 
void Intermediary::stop() {
  Mensaje end;
  // Se declara la operación "FIN"
  end.idClient = END;
  // Se indica el motivo
  end.reason = Reason::end;
  // Se copia el contenido indicando el final
  strcpy(end.content, "TERMINAR");
  // Se ingresa el mensaje en la cola
  this->queueFromClient.enqueue(end);
  // Registro de bitácora: encolamiento de operación final
  this->bitacora.registrarAccion("Intermediario", "encolamiento de operación final");
  // Se cambia el estado del intermediario
  this->status = Status::Stopped;
  // Se finaliza el hilo de ejecución
  if (this->thread.joinable()) {
    this->thread.join();
  }
}

Message Intermediary::processRequest(const Message& request) {
  // Variable local para crear el mensaje a enviar
  Message answer = request;
  // Procesa el mensaje en caso de que no haya tenido error
  if (request.reason != Reason::Error) { 
    // Inicializa los atributos correspondientes
    answer.idClient = request.idClient;
    answer.reason = Reason::success;
    // Guarda el contenido original en una variable local
    std::string original = request.content;    
    // Se inicializa un nuevo string para crear un nuevo contenido
    std::string result;
    // Se agregan encabezados
    result += "CATEGORÍA\tPRODUCTO\tCANTIDAD\tPRECIO\n";
    // Guarda el contenido en una variable stringstream para ser evaluado
    std::stringstream ss(original);
    // Variable local que almacena la línea actual del recorrido
    std::string line;
    // Se ejecuta mientras haya contenido
    while (std::getline(ss, line)) {
       // Se reemplaza ':' por '\t'
      for (char& c : line) {
        if (c == ':') c = '\t';
      }
      // Se guarda la línea evaluada y se agrega un salto de línea
      result += line;
      result += '\n';
    }
    // Se copia el resultado al bufer del mensaje a enviar
    strcpy(answer.content, result.c_str());
  }
  // retorna la respuesta correspondiente
  return answer;
}


void Intermediary::execute() {
  // Variable local para guardar los mensajes enviados al intermediario
  Message message;
  // Se ejecuta mientras el estado del intermediario no sea detenido
  while (this->status != Status::Stopped) {
    // Se declara que el intermediario está esperando
    this->status = Status::Waiting;
    // Se recibe la solicitud del cliente
    Message request = this->queueFromClient.dequeue();
    // Se declara que el intermediario está procesando la solicitud del cliente
    this->status = Status::Processing;
    // Se crea la solicitud para el servidor (Pendiente)
    message = request;
    // Se encola el mensaje creado para el servidor
    this->queueToServer.enqueue(message);
    // Recibe la respuesta desde el servidor
    Message response = this->queueFromServer.dequeue();
    // Se crea la respuesta para el cliente (Pendiente)
    message = response;
    // Se encola el mensaje creado para el cliente
    this->queueToClient.enqueue(message);
  }
}
