#pragma once
#include "Disco.hpp"

// Disco mecánico: 150 MB/s, 15.00 ms de latencia (movimiento del cabezal).
class HDD : public Disco {
public:
    HDD();
    std::string describirTecnologia() const override;

    // Sobrescritura: en un HDD real, el posicionamiento del cabezal penaliza
    // aún más el acceso fragmentado (movimiento mecánico repetido). Se
    // modela un pequeño factor de penalización adicional del 10% por
    // reposicionamiento en escenarios fragmentados, para reflejar el
    // comportamiento físico real y diferenciarlo de SSD/NVMe.
    double calcularTiempoFragmentado(double tamanoPorArchivoMB, long cantidadArchivos) const override;
};
