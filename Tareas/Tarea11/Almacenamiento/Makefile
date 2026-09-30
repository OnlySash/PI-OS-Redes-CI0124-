CXX = g++
CXXFLAGS = -std=c++17 -Wall

OBJS = main.o almacenamiento.o control_bloque.o producto.o
BIN = bodegas

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJS)

main.o: main.cpp Headers/almacenamiento.hpp Headers/bloques.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

almacenamiento.o: Code/almacenamiento.cpp Headers/almacenamiento.hpp Headers/control_bloque.hpp Headers/producto.hpp Headers/bloques.hpp
	$(CXX) $(CXXFLAGS) -c Code/almacenamiento.cpp 

control_bloque.o: Code/control_bloque.cpp Headers/control_bloque.hpp Headers/bloques.hpp
	$(CXX) $(CXXFLAGS) -IHeaders -c Code/control_bloque.cpp -o control_bloque.o
 
producto.o: Code/producto.cpp Headers/producto.hpp Headers/control_bloque.hpp
	$(CXX) $(CXXFLAGS) -IHeaders -c Code/producto.cpp -o producto.o

clean:
	rm -f $(OBJS) $(BIN) bodegas.data

run: $(BIN)
	./$(BIN)