#include "SSDSata.hpp"

SSDSata::SSDSata() : Disco("SSD SATA", 550.0, 0.10) {}

std::string SSDSata::describirTecnologia() const {
    return "Unidad de estado solido con acceso puramente electronico a "
           "compuertas NAND. Sin partes moviles, por lo que la latencia "
           "es baja y estable.";
}
