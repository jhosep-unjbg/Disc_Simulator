#pragma once
#include <string>

/// Códigos de escape ANSI para dar color a la salida de consola. Se
/// utilizan para diferenciar visualmente cada tecnología (mismo criterio
/// de color que el frontend web: rojo=HDD, verde=SSD, azul=NVMe).
namespace Color {
    inline const std::string RESET   = "\033[0m";
    inline const std::string NEGRITA = "\033[1m";
    inline const std::string ROJO    = "\033[31m";
    inline const std::string VERDE   = "\033[32m";
    inline const std::string AMARILLO= "\033[33m";
    inline const std::string AZUL    = "\033[34m";
    inline const std::string CIAN    = "\033[36m";
    inline const std::string GRIS    = "\033[90m";
}
