#pragma once
#include <string>

/// Utilidades minimas para construir y leer JSON sin depender de una
/// libreria externa (evita descargas de red en el entorno del curso).
/// No es un parser JSON completo: esta pensado para el contrato simple
/// y controlado entre nuestro propio frontend y backend.
namespace JsonUtil {

    /// Escapa comillas y barras invertidas para incluir un string dentro
    /// de un valor JSON de forma segura.
    std::string escapar(const std::string& texto);

    /// Busca "clave": <numero> dentro de un cuerpo JSON y devuelve el
    /// numero como double. Lanza std::runtime_error si la clave no existe
    /// o el valor no es numerico.
    double extraerNumero(const std::string& json, const std::string& clave);

    /// Igual que extraerNumero pero como entero largo.
    long extraerEntero(const std::string& json, const std::string& clave);
}
