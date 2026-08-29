/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Isla 3: Grupo 2
  *
  *  ******  Mensaje.h: Definición de la estructura de mensaje
  *
  * (Fedora version)
  *
 **/

#ifndef MESSAGE_H
#define MESSAGE_H

// Constante definida para el tamaño máximo del contenido del mensaje
#define MAX_SIZE 256

// Constantes enumeradas definidas para detectar el motivo del mensaje
enum class Reason {
  ConsultProduct,
  ListCategory,
  Success,
  Error,
  End
};

enum class Status {
  Waiting,
  Processing,
  Stopped
};

// Estructura definida para los mensajes entre el cliente y el intermediario
struct Message {
  // Identificador del cliente que envía el mensaje
  int idClient;
  // Motivo del mensaje
  Reason reason;
  // Contenido del mensaje
  char content[MAX_SIZE];
};

#endif
