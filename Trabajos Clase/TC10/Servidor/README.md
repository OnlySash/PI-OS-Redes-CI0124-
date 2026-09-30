# Servidor de PRUEBA para el cliente TicAmazon 

Un solo archivo, secuencial (una conexion a la vez), datos fijos en memoria.

# Compilación y Ejecución
- g++ -std=c++17 Servidor.cpp Socket.cpp Vsocket.cpp -o servidor

- ./servidor 8080

# Probar:
curl "http://127.0.0.1:8080/TicAmazon/list.php"
   ./cliente 127.0.0.1 8080