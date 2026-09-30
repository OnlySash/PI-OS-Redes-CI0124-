#include "../Headers/control_bloque.hpp"
#include <sstream>
#include <cstring>
 
// Lectura y escritura de bloque
void C_Bloques::leer_bloque(int32_t indice, Bloque& bloque) {
    archivo.seekg(static_cast<std::streamoff>(indice) * TAM_BLOQUE, std::ios::beg);
    archivo.read(bloque.raw, TAM_BLOQUE);
}
 
void C_Bloques::escribir_bloque(int32_t indice, const Bloque& bloque) {
    archivo.seekp(static_cast<std::streamoff>(indice) * TAM_BLOQUE, std::ios::beg);
    archivo.write(bloque.raw, TAM_BLOQUE);
    archivo.flush();
}
 
BloqueControl C_Bloques::leer_control() {
    Bloque b;
    leer_bloque(0, b);
    return b.control;
}
 
void C_Bloques::escribir_control(const BloqueControl& control) {
    Bloque b;
    b.control = control;
    escribir_bloque(0, b);
}
 
// Gestión de bloques libres
// El archivo no tiene un tope fijo de bloques: si la lista de libres está
// vacía, simplemente se agrega un bloque nuevo al final del archivo. Así
// el almacenamiento crece bajo demanda en vez de estar limitado a los
// bloques reservados al crear el archivo.
 
int32_t C_Bloques::obtener_bloque_libre() {
    BloqueControl control = leer_control();
 
    if (control.primer_bloque_libre != BLOQUE_INVALIDO) {
        // Reutilizar el primero de la lista de libres
        int32_t indice = control.primer_bloque_libre;
        Bloque b;
        leer_bloque(indice, b);
        control.primer_bloque_libre = b.libre.encabezado.bloque_siguiente;
        control.cantidad_bloques_libres--;
        escribir_control(control);
        return indice;
    }
 
    // No hay bloques libres: extender el archivo con uno nuevo al final
    int32_t indice = control.num_bloques_totales;
    control.num_bloques_totales++;
    escribir_control(control);
 
    Bloque vacio; // el constructor de Bloque ya deja todo en cero
    escribir_bloque(indice, vacio);
    return indice;
}
 
void C_Bloques::liberar_bloque(int32_t indice) {
    BloqueControl control = leer_control();
 
    Bloque b;
    b.libre.encabezado.tipo_bloque = TIPO_LIBRE;
    b.libre.encabezado.bloque_siguiente = control.primer_bloque_libre;
    b.libre.encabezado.cantidad_usada = 0;
    escribir_bloque(indice, b);
 
    control.primer_bloque_libre = indice;
    control.cantidad_bloques_libres++;
    escribir_control(control);
}
 
// Búsqueda en el directorio
bool C_Bloques::buscar_entrada_bodega(const std::string& nombre_bodega, int32_t& bloque_dir_out, int& pos_entrada_out, int32_t& primer_bloque_datos_out) {
    BloqueControl control = leer_control();
    int32_t indice = control.primer_bloque_directorio;
 
    while (indice != BLOQUE_INVALIDO) {
        Bloque b;
        leer_bloque(indice, b);
        for (int i = 0; i < MAX_ENTRADAS_DIR; i++) {
            const EntradaDirectorio& e = b.directorio.entradas[i];
            if (e.nombre_bodega[0] == '\0') continue;
            // nombre_bodega puede no venir terminado en '\0' si ocupa
            // exactamente TAM_NOMBRE_BODEGA caracteres.
            size_t len = strnlen(e.nombre_bodega, TAM_NOMBRE_BODEGA);
            if (len == nombre_bodega.size() &&
                std::memcmp(e.nombre_bodega, nombre_bodega.data(), len) == 0) {
                bloque_dir_out = indice;
                pos_entrada_out = i;
                primer_bloque_datos_out = e.primer_bloque_datos;
                return true;
            }
        }
        indice = b.directorio.encabezado.bloque_siguiente;
    }
    return false;
}