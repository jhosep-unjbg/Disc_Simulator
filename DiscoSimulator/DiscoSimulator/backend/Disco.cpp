#include "Disco.hpp"

Disco::Disco(std::string nombre, double velocidadMBs, double latenciaMs)
    : nombre(std::move(nombre)), velocidadMBs(velocidadMBs), latenciaMs(latenciaMs) {}

double Disco::calcularTiempoArchivoUnico(double tamanoMB) const {
    double latenciaS = getLatenciaSegundos();
    double tiempoTransferencia = tamanoMB / velocidadMBs;
    return latenciaS + tiempoTransferencia;
}

double Disco::calcularTiempoFragmentado(double tamanoPorArchivoMB, long cantidadArchivos) const {
    double latenciaS = getLatenciaSegundos();
    double latenciaAcumulada = latenciaS * static_cast<double>(cantidadArchivos);
    double tamanoTotalMB = tamanoPorArchivoMB * static_cast<double>(cantidadArchivos);
    double tiempoTransferencia = tamanoTotalMB / velocidadMBs;
    return latenciaAcumulada + tiempoTransferencia;
}

std::string Disco::getNombre() const { return nombre; }
double Disco::getVelocidad() const { return velocidadMBs; }
double Disco::getLatenciaMs() const { return latenciaMs; }
double Disco::getLatenciaSegundos() const { return latenciaMs / 1000.0; }
