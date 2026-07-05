CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
SRC = main.cpp backend/Disco.cpp backend/HDD.cpp backend/SSDSata.cpp backend/NVMe.cpp backend/Simulador.cpp frontend/ConsoleUI.cpp
BIN = simulador_discos

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN) resultados.csv web/datos.js
