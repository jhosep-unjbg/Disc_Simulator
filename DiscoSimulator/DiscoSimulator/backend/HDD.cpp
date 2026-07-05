#include "HDD.hpp"

HDD::HDD() : Disco("HDD Mecanico", 150.0, 15.00) {}

std::string HDD::describirTecnologia() const {
    return "Almacenamiento magnetico con partes moviles. La latencia es alta "
           "por el posicionamiento fisico del cabezal de lectura/escritura "
           "sobre el plato giratorio.";
}

double HDD::calcularTiempoFragmentado(double tamanoPorArchivoMB, long cantidadArchivos) const {
    const double FACTOR_PENALIZACION_MECANICA = 1.10; // 10% extra por seek repetido
    double base = Disco::calcularTiempoFragmentado(tamanoPorArchivoMB, cantidadArchivos);
    return base * FACTOR_PENALIZACION_MECANICA;
}
