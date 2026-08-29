# Linux Makefile para el simulador de la Isla 3
#

# Compiladores y flags
CXX := g++
BANDERASCXX := -std=c++11 -g -Wall -Irecursos/encabezados
BANDERASLD := -pthread
# BANDERASLD += -lssl -lcrypto

# Directorios
DIR_COD := recursos/codigo
DIR_ENC := recursos/encabezados
DIR_OBJ := recursos/objetos
EJECUTABLE := simulador.out

# Buscar .cc en codigo/ y en raíz
vpath %.cc $(DIR_COD)
vpath %.cc .

# Archivos fuente
RECURSOS := $(wildcard $(DIR_COD)/*.cc)
RECURSOS += $(wildcard *.cc)

# Objetos creados
OBJETOS := $(addprefix $(DIR_OBJ)/, $(notdir $(RECURSOS:.cc=.o)))
DEPENDENCIAS := $(wildcard $(DIR_ENC)/*.h)

# Compilar cada .cc de codigo/ a .o
$(DIR_OBJ)/%.o: $(DIR_COD)/%.cc $(DEPENDENCIAS)
	@mkdir -p $(DIR_OBJ)
	$(CXX) $(BANDERASCXX) -MMD -MP -c $< -o $@

# Incluir dependencias generadas (archivos .d)
-include $(OBJETOS:.o=.d)

# Regla por defecto
all: $(EJECUTABLE)

# Enlazar
$(EJECUTABLE): $(OBJETOS)
	$(CXX) $^ $(BANDERASLD) -o $@

# Compilar todos los .cc
$(DIR_OBJ)/%.o: %.cc $(DEPENDENCIAS)
	@mkdir -p $(DIR_OBJ)
	$(CXX) $(BANDERASCXX) -MMD -MP -c $< -o $@

# Incluir dependencias automáticas
-include $(OBJETOS:.o=.d)

# Limpia los objetos y ejecutables creados
limpiar:
	rm -rf $(DIR_OBJ) $(EJECUTABLE)

.PHONY: all limpiar
