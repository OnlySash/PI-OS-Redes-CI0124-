#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <mutex>
#include "bloques.hpp"
#include "control_bloque.hpp"
#include "producto.hpp"


class Almacenamiento {
public:
    Almacenamiento(): bloques(archivo), productos(bloques) {} 
   
    // Crea el archivo desde cero bloque de control y un bloque de directorio vacio
    bool crear_archivo(const std::string& ruta);

    // Abre un archivo para trabajar con el
    bool abrir_archivo(const std::string& ruta);

    void cerrar_archivo();

    // Crea una bodega nueva le agrega su entrada en el directorioy le reserva su primer bloque de datos, vacio
    bool crear_bodega(const std::string& nombre_bodega);

   // Utilidades para ver el estado del archivo
    std::vector<std::string> listar_bodegas();

    // Acceso a las operaciones de producto (insertar/extraer/listar)
    Producto& obtener_productos() { return productos; }

private:
    std::fstream archivo;
    std::mutex mtx;
    C_Bloques bloques;
    Producto productos;

};