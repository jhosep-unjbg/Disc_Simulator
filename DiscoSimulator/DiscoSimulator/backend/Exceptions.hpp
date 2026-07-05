#pragma once
#include <stdexcept>
#include <string>

/// Excepción base de todo el subsistema del simulador. Permite capturar
/// cualquier error propio del dominio con un único catch si se desea.
class SimuladorException : public std::runtime_error {
public:
    explicit SimuladorException(const std::string& mensaje) : std::runtime_error(mensaje) {}
};

/// Se lanza cuando el usuario (o el código) intenta simular con parámetros
/// fuera de rango (tamaños negativos, cero, o absurdamente grandes).
class EntradaInvalidaException : public SimuladorException {
public:
    explicit EntradaInvalidaException(const std::string& mensaje) : SimuladorException(mensaje) {}
};

/// Se lanza ante fallos de lectura/escritura de archivos (CSV, JS, logs).
class ArchivoException : public SimuladorException {
public:
    explicit ArchivoException(const std::string& mensaje) : SimuladorException(mensaje) {}
};
