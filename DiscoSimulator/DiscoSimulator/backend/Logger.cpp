#include "Logger.hpp"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string Logger::marcaTiempo() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::nivelToString(Nivel nivel) {
    switch (nivel) {
        case Nivel::INFO: return "INFO";
        case Nivel::ADVERTENCIA: return "ADVERTENCIA";
        case Nivel::FALLA: return "ERROR";
    }
    return "DESCONOCIDO";
}

void Logger::log(Nivel nivel, const std::string& mensaje) {
    std::ofstream out("simulador.log", std::ios::app);
    std::string linea = "[" + marcaTiempo() + "] [" + nivelToString(nivel) + "] " + mensaje;
    if (out.is_open()) {
        out << linea << "\n";
    }
    if (nivel == Nivel::FALLA) {
        std::cerr << linea << "\n";
    }
}
