#pragma once
#include <fstream>
#include <cstring>
#include <cstdint>
#include "bloques.hpp"
 
class C_Bloques {
public:    
    explicit C_Bloques(std::fstream& archivo) : archivo(archivo) {}

    void leer_bloque(int32_t indice, Bloque& bloque);
    void escribir_bloque(int32_t indice, const Bloque& bloque);
 
    BloqueControl leer_control(); 
    void escribir_control(const BloqueControl& control);

    // Devuelve el indice de un bloque disponible
    int32_t obtener_bloque_libre();
 
    // Marca un bloque como libre y lo mete al inicio de la lista de libres
    void liberar_bloque(int32_t indice);

    // Busca la entrada de una bodega en el directorio
    bool buscar_entrada_bodega(const std::string& nombre_bodega,int32_t& bloque_dir_out, int& pos_entrada_out, int32_t& primer_bloque_datos_out); 

private:
    std::fstream& archivo;
};