#pragma once
#include <memory>
#include <vector>
#include "Disco.hpp"

/// Tipos de tecnología soportados por la fábrica.
enum class TipoDisco { HDD, SSD_SATA, NVME };

/// Implementa el patrón Factory Method: centraliza la creación de objetos
/// Disco para que el resto del sistema (Simulador, pruebas, futuras
/// extensiones) no dependa de las clases concretas HDD/SSDSata/NVMe.
/// Agregar una nueva tecnología en el futuro solo requiere modificar esta
/// fábrica, sin tocar el código que la consume (principio Abierto/Cerrado).
class DiscoFactory {
public:
    static std::unique_ptr<Disco> crear(TipoDisco tipo);
    static std::vector<std::unique_ptr<Disco>> crearConjuntoEstandar();
};
