#pragma once
#include "../backend/Simulador.hpp"
#include "Colores.hpp"

// Capa de presentacion (frontend). No contiene logica de calculo: solo
// se encarga de leer datos del usuario, invocar al backend (Simulador)
// y mostrar los resultados. Esto separa claramente responsabilidades
// (patron de capas backend/frontend).
class ConsoleUI {
private:
    Simulador& simulador;

    void mostrarBanner() const;
    void mostrarMenu() const;
    int leerOpcion() const;
    double leerDouble(const std::string& mensaje) const;
    long leerEntero(const std::string& mensaje) const;

    std::string colorParaDisco(const std::string& nombreDisco) const;

    void mostrarResultados(const std::vector<ResultadoSimulacion>& resultados) const;
    void flujoArchivoUnico();
    void flujoFragmentado();
    void flujoExportar() const;
    void flujoDescripciones() const;
    void flujoHistorial() const;

public:
    explicit ConsoleUI(Simulador& sim);
    void ejecutar();
};
