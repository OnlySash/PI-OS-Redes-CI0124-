/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Principal.cc: Programa central del proyecto
  *
  *  Arma manualmente las 4 colas y un Intermediario real, y un servidor
  *
  * (Fedora version)
  *
 **/

#include "Client.h"
#include "Intermediary.h"
#include "Server.h"
#include "Queue.h"
#include "Message.h"
#include "Bitacora.h"

#include <iostream>
#include <vector>

// Método que testeará el proyecto
void test() {
  // Crea las colas de comunicación
  Queue clientToIntermediary;
  Queue clientFromIntermediary;
  Queue intermediaryToServer;
  Queue intermediaryFromServer;

  // Inicializa el mensaje a procesar
  Message message;
  message.idClient = 1;

  // Inicializa la bitácora y las entidades
  Bitacora bitacora(RUTA_BITACORA);
  Client client(clientToIntermediary, clientFromIntermediary, bitacora);
  Intermediary intermediary(intermediaryToServer, intermediaryFromServer, clientFromIntermediary, clientToIntermediary, bitacora);
  Server server(intermediaryFromServer, intermediaryToServer, bitacora);

  // Comienza la ejecución
  intermediary.iniciar();
  server.iniciar();

  // Mensaje 1: Consultar Producto
  message.reason = Reason::ConsultProduct;
  strcpy(message.content, "garbanzos");
  client.iniciar(message);
  client.detener();

  // Mensaje 2: Listar Productos
  message.reason = Reason::ListProducts;
  strcpy(message.content, "");
  client.start(message);
  client.stop();

  // Mensaje 3: Listar Categoría
  message.reason = Reason::ListCategory;
  strcpy(message.content, "condimentos");
  client.start(message);
  client.stop();

  // Mensaje 4: Consultar Producto (da error)
  message.reason = Reason::ConsultProduct;
  strcpy(message.content, "cantonés");
  client.start(message);
  client.stop();

  // Mensaje 5: Listar Categoría (da error)
  message.reason = Reason::ListCategory;
  strcpy(message.content, "higiene");
  client.start(message);
  client.stop();

  // Se detienen las entidades
  server.stop();
  intermediary.stop();
}

int main() {
  test();
  return 0;
}
