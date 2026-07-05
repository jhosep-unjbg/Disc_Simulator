#include "DiscoFactory.hpp"
#include "HDD.hpp"
#include "SSDSata.hpp"
#include "NVMe.hpp"
#include "Exceptions.hpp"

std::unique_ptr<Disco> DiscoFactory::crear(TipoDisco tipo) {
    switch (tipo) {
        case TipoDisco::HDD:      return std::make_unique<HDD>();
        case TipoDisco::SSD_SATA: return std::make_unique<SSDSata>();
        case TipoDisco::NVME:     return std::make_unique<NVMe>();
    }
    throw SimuladorException("Tipo de disco no reconocido por la fabrica.");
}

std::vector<std::unique_ptr<Disco>> DiscoFactory::crearConjuntoEstandar() {
    std::vector<std::unique_ptr<Disco>> discos;
    discos.push_back(crear(TipoDisco::HDD));
    discos.push_back(crear(TipoDisco::SSD_SATA));
    discos.push_back(crear(TipoDisco::NVME));
    return discos;
}
