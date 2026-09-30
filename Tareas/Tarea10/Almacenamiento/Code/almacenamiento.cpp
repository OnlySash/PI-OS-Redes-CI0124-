#include "../Headers/almacenamiento.hpp"

// Crear / abrir / cerrar archivo
bool Almacenamiento::crear_archivo(const std::string& ruta) {
    //Manejo de hilos con mutex
    std::lock_guard<std::mutex> lock(mtx);

    // std::ios::trunc crea el archivo si no existe, o lo vacía si existe
    archivo.open(ruta, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!archivo.is_open()) return false;

    // Bloque 0: control. Arranca con solo el bloque de control (0) y el
    // primer bloque de directorio (1) ya reservados.
    BloqueControl control{};
    control.num_bloques_totales = 2;
    control.primer_bloque_directorio = 1;
    control.primer_bloque_libre = BLOQUE_INVALIDO;
    control.cantidad_bloques_libres = 0;
    control.cantidad_bodegas = 0;
    bloques.escribir_control(control);

    //Primer Bloque es directorio vacío
    Bloque dir;
    dir.directorio.encabezado.tipo_bloque = TIPO_DIRECTORIO;
    dir.directorio.encabezado.bloque_siguiente = BLOQUE_INVALIDO;
    dir.directorio.encabezado.cantidad_usada = 0;
    for (int i = 0; i < MAX_ENTRADAS_DIR; i++) {
        dir.directorio.entradas[i].nombre_bodega[0] = '\0';
        dir.directorio.entradas[i].primer_bloque_datos = BLOQUE_INVALIDO;
    }

    bloques.escribir_bloque(1, dir);
    return true;
    //return abrir_archivo(ruta)
}

bool Almacenamiento::abrir_archivo(const std::string& ruta) {
    //Manejo de hilos
    std::lock_guard<std::mutex> lock(mtx);
    archivo.open(ruta, std::ios::in | std::ios::out | std::ios::binary);
    return archivo.is_open();
}

void Almacenamiento::cerrar_archivo() {
    //Manejo de hilos
    std::lock_guard<std::mutex> lock(mtx);
    if (archivo.is_open()) archivo.close();
}

bool Almacenamiento::crear_bodega(const std::string& nombre_bodega) {
    std::lock_guard<std::mutex> lock(mtx);

    if (nombre_bodega.empty() || nombre_bodega.size() >= (size_t)TAM_NOMBRE_BODEGA) {
        return false; // no cabe en el campo fijo
    }

    int32_t bloque_dir_existente; int pos_existente; int32_t datos_existente;
    if (bloques.buscar_entrada_bodega(nombre_bodega, bloque_dir_existente, pos_existente, datos_existente)) {
        return false; // ya existe
    }

    BloqueControl control = bloques.leer_control();

    // Recorrer la cadena de directorio buscando un espacio libre
    int32_t indice = control.primer_bloque_directorio;
    int32_t ultimo_indice = indice;
    Bloque b;
    int pos_libre = -1;

    while (true) {
        bloques.leer_bloque(indice, b);
        for (int i = 0; i < MAX_ENTRADAS_DIR; i++) {
            if (b.directorio.entradas[i].nombre_bodega[0] == '\0') {
                pos_libre = i;
                break;
            }
        }
        if (pos_libre != -1) break;

        ultimo_indice = indice;
        if (b.directorio.encabezado.bloque_siguiente == BLOQUE_INVALIDO) {
            // No hay espacio en ningún bloque de directorio existente:
            // encadenar uno nuevo.
            int32_t nuevo = bloques.obtener_bloque_libre();

            Bloque nuevo_bloque;
            nuevo_bloque.directorio.encabezado.tipo_bloque = TIPO_DIRECTORIO;
            nuevo_bloque.directorio.encabezado.bloque_siguiente = BLOQUE_INVALIDO;
            nuevo_bloque.directorio.encabezado.cantidad_usada = 0;
            for (int i = 0; i < MAX_ENTRADAS_DIR; i++) {
                nuevo_bloque.directorio.entradas[i].nombre_bodega[0] = '\0';
                nuevo_bloque.directorio.entradas[i].primer_bloque_datos = BLOQUE_INVALIDO;
            }
            bloques.escribir_bloque(nuevo, nuevo_bloque);

            // Enlazar el bloque anterior al nuevo (releer control por si
            // obtener_bloque_libre lo modificó)
            Bloque anterior;
            bloques.leer_bloque(ultimo_indice, anterior);
            anterior.directorio.encabezado.bloque_siguiente = nuevo;
            bloques.escribir_bloque(ultimo_indice, anterior);

            indice = nuevo;
            b = nuevo_bloque;
            pos_libre = 0;
            break;
        }
        indice = b.directorio.encabezado.bloque_siguiente;
    }

    // Reservar el primer bloque de datos de la bodega, vacío
    int32_t bloque_datos = bloques.obtener_bloque_libre();
    Bloque datos;
    datos.datos.encabezado.tipo_bloque = TIPO_DATOS;
    datos.datos.encabezado.bloque_siguiente = BLOQUE_INVALIDO;
    datos.datos.encabezado.cantidad_usada = 0;
    std::memset(datos.datos.texto, 0, TAM_TEXTO_DATOS);
    bloques.escribir_bloque(bloque_datos, datos);

    // Volver a leer el bloque de directorio (por si obtener_bloque_libre
    // lo desplazó al crecer el archivo) y escribir la entrada
    bloques.leer_bloque(indice, b);
    std::memset(b.directorio.entradas[pos_libre].nombre_bodega, 0, TAM_NOMBRE_BODEGA);
    std::memcpy(b.directorio.entradas[pos_libre].nombre_bodega, nombre_bodega.data(), nombre_bodega.size());
    b.directorio.entradas[pos_libre].primer_bloque_datos = bloque_datos;
    b.directorio.encabezado.cantidad_usada++;
    bloques.escribir_bloque(indice, b);

    control = bloques.leer_control();
    control.cantidad_bodegas++;
    bloques.escribir_control(control);

    return true;
}

// Consultas
std::vector<std::string> Almacenamiento::listar_bodegas() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::string> resultado;

    BloqueControl control = bloques.leer_control();
    int32_t indice = control.primer_bloque_directorio;

    while (indice != BLOQUE_INVALIDO) {
        Bloque b;
        bloques.leer_bloque(indice, b);
        for (int i = 0; i < MAX_ENTRADAS_DIR; i++) {
            const EntradaDirectorio& e = b.directorio.entradas[i];
            if (e.nombre_bodega[0] == '\0') continue;
            size_t len = strnlen(e.nombre_bodega, TAM_NOMBRE_BODEGA);
            resultado.emplace_back(e.nombre_bodega, len);
        }
        indice = b.directorio.encabezado.bloque_siguiente;
    }
    return resultado;
}