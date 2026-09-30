#ifndef SERVIDOR_HPP
#define SERVIDOR_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cctype>
#include "Socket.hpp"

struct Item {
   std::string bodega, categoria, descripcion;
   int cantidad;
   double precio;
};

extern std::vector<Item> inventario;

std::string Lower( std::string s );

std::string DecodificarUrl( const std::string & t );
 
std::string CodificarUrl( const std::string & t );

std::map<std::string, std::string> ParsearForm( const std::string & texto );

std::string Decimal( double v );
std::string Inicio( const std::string & t );

std::string PaginaCategorias();

std::string PaginaProductos( const std::string & cat );

int PaginaProforma( const std::string & cuerpo, std::string & html );
std::string Respuesta( int codigo, const std::string & cuerpo );

std::string LeerRequest( Socket & s );
void Atender( Socket & s );

#endif //SERVIDOR_HPP