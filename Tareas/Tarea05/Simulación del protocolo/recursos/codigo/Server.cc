#include "Server.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>

Server::Server( Queue &queueToIntermediary, Queue &queueFromIntermediary, Bitacora &bitacora) :
    status(StatusServer::Waiting),
    queueToIntermediary(queueToIntermediary),
    queueFromIntermediary(queueFromIntermediary),
    bitacora(bitacora) {
     this->archive.open(RUTA_SISTEMA_ARCHIVOS, std::ios::in);
    
    }

Server::~Server() {
  this->archive.close();
}

void Server::start() {
  // Inicializa y ejecuta el hilo del servidor
  this->bitacora.registrarAccion("Servidor", "inicialización");
  this->thread = std::thread(&Server::execute, this);
}

void Server::stop() {
  Message end;
  // Se declara la operación "FIN"
  end.idClient = -1;
  // Se indica el motivo
  end.reason = Reason::End;
  // Se copia el contenido indicando el final
  strcpy(end.content, "TERMINAR");
  // Se ingresa el mensaje en la cola
  this->queueFromIntermediary.enqueue(end);
  // Registro de bitácora: encolamiento de operación final
  this->bitacora.registrarAccion("Servidor", "encolamiento de operación final");
 
  
  // Cambia el estado del servidor a detenido
  this->status = StatusServer::Stopped;
  // Espera a que el hilo del servidor termine su ejecución
  if (this->thread.joinable()) {
    this->thread.join();
  }
}

Message Server::processRequest(const Message& request) {
  if (request.reason == Reason::ConsultProduct) {
    return consultProduct(request);
  } else if (request.reason == Reason::ListCategory) {
    return listCategory(request);
  } else if (request.reason == Reason::ListProducts) {
    return listProducts(request);
  } else {
    return invalidMessage(request);
  }

}

Message Server::invalidMessage(const Message& request) {
  // Se crea una variable local para guardar la respuesta
  Message response;
  // Se asigna la identificación del cliente
  response.idClient = request.idClient;
  // Se agregaa el motivo del mensaje
  response.reason = Reason::Error;
  // Se copia el contenido del mensaje
  strcpy(response.content, "Respuesta Inválida\n");
  // Se retorna la estructura creada
  return response;
}

Message Server::consultProduct(const Message& request) {
  // Variable local para crear el mensaje a enviar
  Message response;
  // Inicializa los atributos correspondientes
  response.idClient = request.idClient;
  // Variable local para guardar el contenido encontrado
  std::string content;
  // Limpia cualquier estado anterior del archivo
  archive.clear();
  // Coloca el cursor de lectura al inicio
  archive.seekg(0, std::ios::beg);
  // Comienza a leer cada línea de la bodega hasta llegar al último producto
  while (std::getline(archive, content)) {
    // Guarda el contenido en una variable stringstream para ser evaluado
    std::stringstream ss(content);
    // Crea una variable local para guardar el producto a buscar
    std::string product;
    // Busca en la línea 2 veces para encontrar el producto entre los separadores
    std::getline(ss, product, ':');
    std::getline(ss, product, ':');
    // Compara el producto consultado
    if (request.content == product){
      // Si coinicide, para la operación y retorna la respuesta
      strcpy(response.content, content.c_str());
      response.reason = Reason::Success;
      return response;
    }
  }
  // Si no lo encuentra, retorna la respuesta correspondiente
  strcpy(response.content, "Producto no encontrado\n");
  response.reason = Reason::Error;
  return response;
}

Message Server::listCategory(const Message& request) {
  // Variable local para crear el mensaje a enviar
  Message response;
  // Inicializa los atributos correspondientes
  response.idClient = request.idClient;
  // Variable local para guardar el contenido encontrado
  std::string content;
  // Acumula los productos que calcen con lo solicitado
  std::string acumulated;
  // Limpia cualquier estado anterior del archivo
  archive.clear();
  // Coloca el cursor de lectura al inicio
  archive.seekg(0, std::ios::beg);
  // Comienza a leer cada línea de la bodega hasta llegar al último producto
  while (std::getline(archive, content)) {
    // Guarda el contenido en una variable stringstream para ser evaluado
    std::stringstream ss(content);
    // Crea una variable local para guardar la categoria a buscar
    std::string category;
    // Busca en la línea 1 vez para encontrar la categoria entre los separadores
    std::getline(ss, category, ':');
    // Compara la categoria consultada
    if (request.content == category) {
      // Si coinicide, para la operación y retorna la respuesta
      acumulated += content;
      // Agrega un separador de productos
      acumulated += '\n';
    }
  }
  // Evalua si el resultado fue vacío o si encontró productos
  if (!acumulated.empty()) {
    // Elimina el último separador extra
    acumulated.pop_back();
    strcpy(response.content, acumulated.c_str());
    response.reason = Reason::Success;
  } else {
    strcpy(response.content, "Categoría no encontrada\n");
    response.reason = Reason::Error;
  }
  // Retorna la respuesta correspondiente
  return response;
}

Message Server::listProducts(const Message& request) {
  // Variable local para crear el mensaje a enviar
  Message response;
  // Inicializa los atributos correspondientes
  response.idClient = request.idClient;
  // Variable local para guardar el contenido encontrado
  std::string content;
  // Acumula los productos que calcen con lo solicitado
  std::string acumulated;
  // Limpia cualquier estado anterior del archivo
  archive.clear();
  // Coloca el cursor de lectura al inicio
  archive.seekg(0, std::ios::beg);
  // Comienza a leer cada línea de la bodega hasta llegar al último producto
  while (std::getline(archive, content)) {
    // Guarda la línea leída en el archivo
    acumulated += content;
    // Agrega un separador de productos
    acumulated += '\n';
  }
  // Evalua si el resultado fue vacío o si encontró productos
  if (!acumulated.empty()) {
    strcpy(response.content, acumulated.c_str());
    response.reason = Reason::Success;
  } else {
    strcpy(response.content, "Lista vacía\n");
    response.reason = Reason::Error;
  }
  // Retorna la respuesta correspondiente
  return response;
}

void Server::execute() {
  // Variable local para guardar los mensajes enviados al servidor
  //Mensaje mensaje;
  // Se ejecuta mientras el estado del servidor no sea detenido
  while (this->status != StatusServer::Stopped) {
    // Se declara que el servidor está esperando
    this->status = StatusServer::Waiting;
    // Se recibe la solicitud del intermediario
    Message request = this->queueFromIntermediary.dequeue();
    // Se declara que el servidor está procesando la solicitud del intermediario
    this->bitacora.registrarAccion("Servidor", "desencolamiento de mensaje");

    this->status = StatusServer::Processing;
    // Se crea la solicitud para el intermediario (Pendiente)
    Message response = this->processRequest(request);  
    // Se encola el mensaje creado para el intermediario
    this->queueToIntermediary.enqueue(response);
    this->bitacora.registrarAccion("Servidor", "encolamiento de mensaje");

  }
}
