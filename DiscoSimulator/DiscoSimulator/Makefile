CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
BACKEND = backend/Disco.cpp backend/HDD.cpp backend/SSDSata.cpp backend/NVMe.cpp backend/DiscoFactory.cpp backend/Simulador.cpp backend/Logger.cpp
BACKEND_SERVIDOR = $(BACKEND) backend/HttpServer.cpp backend/JsonUtil.cpp
FRONTEND = frontend/ConsoleUI.cpp
BIN = simulador_discos
BIN_SERVIDOR = servidor_discos
TEST_BIN = tests_simulador

# Windows requiere enlazar la libreria de sockets (Winsock)
ifeq ($(OS),Windows_NT)
    LDLIBS = -lws2_32
else
    LDLIBS =
endif

all: $(BIN) $(BIN_SERVIDOR)

$(BIN): main.cpp $(BACKEND) $(FRONTEND)
	$(CXX) $(CXXFLAGS) -o $(BIN) main.cpp $(BACKEND) $(FRONTEND)

$(BIN_SERVIDOR): main_servidor.cpp $(BACKEND_SERVIDOR)
	$(CXX) $(CXXFLAGS) -o $(BIN_SERVIDOR) main_servidor.cpp $(BACKEND_SERVIDOR) $(LDLIBS)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): tests/test_disco.cpp $(BACKEND)
	$(CXX) $(CXXFLAGS) -o $(TEST_BIN) tests/test_disco.cpp $(BACKEND)

clean:
	rm -f $(BIN) $(BIN_SERVIDOR) $(TEST_BIN) resultados.csv simulador.log web/datos.js
