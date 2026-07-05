#pragma once
#include <string>

// Clase base abstracta que modela el comportamiento de un dispositivo de
// almacenamiento. Las subclases (HDD, SSDSata, NVMe) especializan los
// parámetros físicos y pueden sobrescribir el comportamiento mediante
// polimorfismo.
class Disco {
protected:
    std::string nombre;
    double velocidadMBs;   // Tasa de transferencia en MB/s
    double latenciaMs;     // Latencia promedio en milisegundos

public:
    Disco(std::string nombre, double velocidadMBs, double latenciaMs);
    virtual ~Disco() = default;

    // Tiempo Total (s) = Latencia (s) + Tamaño(MB) / Velocidad(MB/s)
    // Escenario: transferencia de UN solo archivo (secuencial, masivo).
    virtual double calcularTiempoArchivoUnico(double tamanoMB) const;

    // Escenario: transferencia de N archivos fragmentados de tamaño fijo.
    // La latencia se paga una vez POR CADA archivo (acceso independiente),
    // mientras que el tiempo de transferencia se acumula (sumatoria Σ).
    virtual double calcularTiempoFragmentado(double tamanoPorArchivoMB, long cantidadArchivos) const;

    // Método polimórfico: cada subclase explica su propia tecnología.
    virtual std::string describirTecnologia() const = 0;

    std::string getNombre() const;
    double getVelocidad() const;
    double getLatenciaMs() const;
    double getLatenciaSegundos() const;
};
