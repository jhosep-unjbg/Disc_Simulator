#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Disco.hpp"

// Representa un resultado individual: en qué escenario, con qué disco,
// y el tiempo total obtenido. Es la estructura de datos que viaja desde
// el backend (cálculo) hacia el frontend (consola y reporte web).
struct ResultadoSimulacion {
    std::string escenario;   // Ej: "Archivo unico 40000 MB"
    std::string disco;       // Ej: "HDD Mecanico"
    double tiempoSegundos;
};

class Simulador {
private:
    std::vector<std::unique_ptr<Disco>> discos;
    std::vector<ResultadoSimulacion> historial;

public:
    Simulador();

    // Escenario 1: un unico archivo secuencial masivo (ej. pelicula de 40 GB)
    std::vector<ResultadoSimulacion> simularArchivoUnico(double tamanoMB);

    // Escenario 2: conjunto masivo de archivos fragmentados (ej. 15000 fotos de 1 MB)
    std::vector<ResultadoSimulacion> simularFragmentado(double tamanoPorArchivoMB, long cantidadArchivos);

    const std::vector<ResultadoSimulacion>& getHistorial() const;
    const std::vector<std::unique_ptr<Disco>>& getDiscos() const;

    // Factor de aceleracion NVMe vs HDD para un mismo conjunto de resultados
    double calcularFactorAceleracion(const std::vector<ResultadoSimulacion>& resultados) const;

    void exportarCSV(const std::string& ruta) const;
    void exportarJS(const std::string& ruta) const; // datos.js para el frontend web
};
