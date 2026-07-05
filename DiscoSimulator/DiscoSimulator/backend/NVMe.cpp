#include "NVMe.hpp"

NVMe::NVMe() : Disco("NVMe M.2", 7000.0, 0.02) {}

std::string NVMe::describirTecnologia() const {
    return "Unidad de estado solido conectada directamente al bus PCIe del "
           "CPU, eliminando el cuello de botella del controlador SATA. "
           "Latencia ultra baja y maximo ancho de banda.";
}
