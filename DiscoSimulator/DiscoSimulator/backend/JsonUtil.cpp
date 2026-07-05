#include "JsonUtil.hpp"
#include <stdexcept>
#include <cctype>

std::string JsonUtil::escapar(const std::string& texto) {
    std::string resultado;
    resultado.reserve(texto.size());
    for (char c : texto) {
        if (c == '"' || c == '\\') resultado += '\\';
        resultado += c;
    }
    return resultado;
}

static double extraerNumeroInterno(const std::string& json, const std::string& clave) {
    std::string patron = "\"" + clave + "\"";
    size_t pos = json.find(patron);
    if (pos == std::string::npos) {
        throw std::runtime_error("Falta el campo requerido: " + clave);
    }
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        throw std::runtime_error("JSON malformado cerca de: " + clave);
    }
    pos++;
    // Saltar espacios en blanco
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;

    size_t inicio = pos;
    while (pos < json.size() &&
           (std::isdigit(static_cast<unsigned char>(json[pos])) || json[pos] == '-' || json[pos] == '.' || json[pos] == '+')) {
        pos++;
    }
    std::string numeroStr = json.substr(inicio, pos - inicio);
    if (numeroStr.empty()) {
        throw std::runtime_error("Valor numerico invalido para: " + clave);
    }
    try {
        return std::stod(numeroStr);
    } catch (...) {
        throw std::runtime_error("No se pudo interpretar el numero de: " + clave);
    }
}

double JsonUtil::extraerNumero(const std::string& json, const std::string& clave) {
    return extraerNumeroInterno(json, clave);
}

long JsonUtil::extraerEntero(const std::string& json, const std::string& clave) {
    return static_cast<long>(extraerNumeroInterno(json, clave));
}
