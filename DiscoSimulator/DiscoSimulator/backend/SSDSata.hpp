#pragma once
#include "Disco.hpp"

// SSD SATA: 550 MB/s, 0.10 ms de latencia (acceso electronico NAND).
class SSDSata : public Disco {
public:
    SSDSata();
    std::string describirTecnologia() const override;
};
