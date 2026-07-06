#pragma once
#include <string>

/// Logger minimalista (sin dependencias externas) que registra eventos
/// con marca de tiempo en "simulador.log". Los errores también se
/// reflejan en std::cerr para visibilidad inmediata en consola.
class Logger {
public:
    enum class Nivel { INFO, ADVERTENCIA, FALLA };

    static void log(Nivel nivel, const std::string& mensaje);

private:
    static std::string marcaTiempo();
    static std::string nivelToString(Nivel nivel);
};
