#pragma once
#include "Disco.hpp"

// NVMe M.2 (PCIe Gen4): 7000 MB/s, 0.02 ms de latencia (bus directo a CPU).
class NVMe : public Disco {
public:
    NVMe();
    std::string describirTecnologia() const override;
};
