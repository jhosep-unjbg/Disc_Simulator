#include "backend/Simulador.hpp"
#include "frontend/ConsoleUI.hpp"

int main() {
    Simulador simulador;      // Backend: logica y calculo
    ConsoleUI ui(simulador);  // Frontend: interaccion con el usuario
    ui.ejecutar();
    return 0;
}
