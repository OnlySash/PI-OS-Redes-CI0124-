#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <cstring>
#include "control_bloque.hpp"

struct ProductoTexto {
    std::string bodega;
    std::string categoria;
    std::string producto;
    std::string cantidad;
    std::string precio;
};

class Producto{
public: 
    explicit Producto(C_Bloques& bloques) : bloques(bloques) {}

     // Inserta un producto en la cadena de datos de una bodega
    bool insertar_producto(const std::string& nombre_bodega,const std::string& categoria,const std::string& producto,const std::string& cantidad,const std::string& precio);

    // Busca y elimina el primer producto que coincida con nombre_bodega mas producto
    bool extraer_producto(const std::string& nombre_bodega,const std::string& nombre_producto);
    
    std::vector<ProductoTexto> listar_productos(const std::string& nombre_bodega);

private:
    C_Bloques& bloques;
    std::mutex mtx;

    static std::string armar_registro(const std::string& bodega, const std::string& categoria, const std::string& producto, const std::string& cantidad,  const std::string& precio);
    static ProductoTexto parsear_registro(const std::string& registro);
};