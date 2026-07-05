#include "Simulador.hpp"
#include "HDD.hpp"
#include "SSDSata.hpp"
#include "NVMe.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Simulador::Simulador() {
    discos.push_back(std::make_unique<HDD>());
    discos.push_back(std::make_unique<SSDSata>());
    discos.push_back(std::make_unique<NVMe>());
}

std::vector<ResultadoSimulacion> Simulador::simularArchivoUnico(double tamanoMB) {
    std::vector<ResultadoSimulacion> resultados;
    std::ostringstream etiqueta;
    etiqueta << "Archivo unico (" << tamanoMB << " MB)";

    for (const auto& disco : discos) {
        double t = disco->calcularTiempoArchivoUnico(tamanoMB);
        ResultadoSimulacion r{etiqueta.str(), disco->getNombre(), t};
        resultados.push_back(r);
        historial.push_back(r);
    }
    return resultados;
}

std::vector<ResultadoSimulacion> Simulador::simularFragmentado(double tamanoPorArchivoMB, long cantidadArchivos) {
    std::vector<ResultadoSimulacion> resultados;
    std::ostringstream etiqueta;
    etiqueta << cantidadArchivos << " archivos x " << tamanoPorArchivoMB << " MB (fragmentado)";

    for (const auto& disco : discos) {
        double t = disco->calcularTiempoFragmentado(tamanoPorArchivoMB, cantidadArchivos);
        ResultadoSimulacion r{etiqueta.str(), disco->getNombre(), t};
        resultados.push_back(r);
        historial.push_back(r);
    }
    return resultados;
}

const std::vector<ResultadoSimulacion>& Simulador::getHistorial() const { return historial; }
const std::vector<std::unique_ptr<Disco>>& Simulador::getDiscos() const { return discos; }

double Simulador::calcularFactorAceleracion(const std::vector<ResultadoSimulacion>& resultados) const {
    double tHDD = 0.0, tNVMe = 0.0;
    for (const auto& r : resultados) {
        if (r.disco == "HDD Mecanico") tHDD = r.tiempoSegundos;
        if (r.disco == "NVMe M.2") tNVMe = r.tiempoSegundos;
    }
    if (tNVMe <= 0.0) return 0.0;
    return tHDD / tNVMe;
}

void Simulador::exportarCSV(const std::string& ruta) const {
    std::ofstream out(ruta);
    if (!out.is_open()) throw std::runtime_error("No se pudo crear el archivo CSV: " + ruta);
    out << "Escenario,Disco,TiempoSegundos\n";
    for (const auto& r : historial) {
        out << "\"" << r.escenario << "\"," << r.disco << "," << r.tiempoSegundos << "\n";
    }
}

void Simulador::exportarJS(const std::string& ruta) const {
    std::ofstream out(ruta);
    if (!out.is_open()) throw std::runtime_error("No se pudo crear el archivo JS: " + ruta);
    out << "// Generado automaticamente por el backend en C++\n";
    out << "const resultadosSimulacion = [\n";
    for (size_t i = 0; i < historial.size(); ++i) {
        const auto& r = historial[i];
        out << "  { escenario: \"" << r.escenario << "\", disco: \"" << r.disco
            << "\", tiempoSegundos: " << r.tiempoSegundos << " }";
        out << (i + 1 < historial.size() ? ",\n" : "\n");
    }
    out << "];\n";
}
