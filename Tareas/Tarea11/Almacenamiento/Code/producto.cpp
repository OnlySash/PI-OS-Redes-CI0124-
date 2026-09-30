#include "../Headers/producto.hpp"

// Registros de producto 
std::string Producto::armar_registro(const std::string& bodega, const std::string& categoria,
                                            const std::string& producto, const std::string& cantidad,
                                            const std::string& precio) {
    return bodega + "," + categoria + "," + producto + "," + cantidad + "," + precio;
}

ProductoTexto Producto::parsear_registro(const std::string& registro) {
    ProductoTexto p;
    std::stringstream ss(registro);
    std::getline(ss, p.bodega, ',');
    std::getline(ss, p.categoria, ',');
    std::getline(ss, p.producto, ',');
    std::getline(ss, p.cantidad, ',');
    std::getline(ss, p.precio, '.'); // el último campo termina en punto
    return p;
}

// Insertar / extraer productos

bool Producto::insertar_producto(const std::string& nombre_bodega, const std::string& categoria, const std::string& producto, const std::string& cantidad, const std::string& precio) {
    std::lock_guard<std::mutex> lock(mtx);

    int32_t bloque_dir; int pos_entrada; int32_t bloque_datos;
    if (!bloques.buscar_entrada_bodega(nombre_bodega, bloque_dir, pos_entrada, bloque_datos)) {
        return false; // la bodega no existe
    }

    std::string registro = armar_registro(nombre_bodega, categoria, producto, cantidad, precio) + "\n";
    if (registro.size() > (size_t)TAM_TEXTO_DATOS) {
        return false; // un solo registro no puede exceder el bloque
    }

    // Caminar hasta el último bloque de datos de la cadena
    int32_t indice = bloque_datos;
    Bloque b;
    while (true) {
        bloques.leer_bloque(indice, b);
        if (b.datos.encabezado.bloque_siguiente == BLOQUE_INVALIDO) break;
        indice = b.datos.encabezado.bloque_siguiente;
    }

    int espacio_libre = TAM_TEXTO_DATOS - b.datos.encabezado.cantidad_usada;
    if ((int)registro.size() <= espacio_libre) {
        std::memcpy(b.datos.texto + b.datos.encabezado.cantidad_usada, registro.data(), registro.size());
        b.datos.encabezado.cantidad_usada += (int32_t)registro.size();
        bloques.escribir_bloque(indice, b);
        return true;
    }

    // No cabe: pedir un bloque nuevo y encadenarlo
    int32_t nuevo = bloques.obtener_bloque_libre();
    Bloque nuevo_bloque;
    nuevo_bloque.datos.encabezado.tipo_bloque = TIPO_DATOS;
    nuevo_bloque.datos.encabezado.bloque_siguiente = BLOQUE_INVALIDO;
    nuevo_bloque.datos.encabezado.cantidad_usada = (int32_t)registro.size();
    std::memset(nuevo_bloque.datos.texto, 0, TAM_TEXTO_DATOS);
    std::memcpy(nuevo_bloque.datos.texto, registro.data(), registro.size());
    bloques.escribir_bloque(nuevo, nuevo_bloque);

    // Releer el último bloque (por si el archivo creció) y enlazarlo
    bloques.leer_bloque(indice, b);
    b.datos.encabezado.bloque_siguiente = nuevo;
    bloques.escribir_bloque(indice, b);

    return true;
}

bool Producto::extraer_producto(const std::string& nombre_bodega, const std::string& nombre_producto) {
    std::lock_guard<std::mutex> lock(mtx);

    int32_t bloque_dir; int pos_entrada; int32_t bloque_datos;
    if (!bloques.buscar_entrada_bodega(nombre_bodega, bloque_dir, pos_entrada, bloque_datos)) {
        return false;
    }

    int32_t indice = bloque_datos;
    int32_t anterior = BLOQUE_INVALIDO;

    while (indice != BLOQUE_INVALIDO) {
        Bloque b;
        bloques.leer_bloque(indice, b);

        int32_t usado = b.datos.encabezado.cantidad_usada;
        int32_t pos = 0;
        while (pos < usado) {
            // localizar el próximo '\n' dentro de la zona usada
            int32_t fin_linea = pos;
            while (fin_linea < usado && b.datos.texto[fin_linea] != '\n') fin_linea++;
            if (fin_linea >= usado) break; // línea incompleta, no debería pasar

            std::string registro(b.datos.texto + pos, fin_linea - pos);
            ProductoTexto p = parsear_registro(registro);

            if (p.producto == nombre_producto) {
                int32_t largo_linea = fin_linea - pos + 1; // incluye '\n'
                int32_t resto = usado - fin_linea - 1;
                std::memmove(b.datos.texto + pos, b.datos.texto + fin_linea + 1, resto);
                std::memset(b.datos.texto + pos + resto, 0, largo_linea);
                b.datos.encabezado.cantidad_usada -= largo_linea;

                // Si el bloque quedó vacío y no es el primero de la cadena
                // de la bodega, se libera y se desengancha.
                if (b.datos.encabezado.cantidad_usada == 0 && indice != bloque_datos) {
                    Bloque bloque_anterior;
                    bloques.leer_bloque(anterior, bloque_anterior);
                    bloque_anterior.datos.encabezado.bloque_siguiente = b.datos.encabezado.bloque_siguiente;
                    bloques.escribir_bloque(anterior, bloque_anterior);
                    bloques.liberar_bloque(indice);
                } else {
                    bloques.escribir_bloque(indice, b);
                }
                return true;
            }
            pos = fin_linea + 1;
        }
        anterior = indice;
        indice = b.datos.encabezado.bloque_siguiente;
    }
    return false;
}

std::vector<ProductoTexto> Producto::listar_productos(const std::string& nombre_bodega) {
    //Manejo de hilos
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<ProductoTexto> resultado;

    int32_t bloque_dir; int pos_entrada; int32_t bloque_datos;
    if (!bloques.buscar_entrada_bodega(nombre_bodega, bloque_dir, pos_entrada, bloque_datos)) {
        return resultado; // bodega inexistente -> lista vacía
    }

    int32_t indice = bloque_datos;
    while (indice != BLOQUE_INVALIDO) {
        Bloque b;
        bloques.leer_bloque(indice, b);

        int32_t usado = b.datos.encabezado.cantidad_usada;
        int32_t pos = 0;
        while (pos < usado) {
            int32_t fin_linea = pos;
            while (fin_linea < usado && b.datos.texto[fin_linea] != '\n') fin_linea++;
            if (fin_linea >= usado) break;

            std::string registro(b.datos.texto + pos, fin_linea - pos);
            resultado.push_back(parsear_registro(registro));
            pos = fin_linea + 1;
        }

        indice = b.datos.encabezado.bloque_siguiente;
    }
    return resultado;
}