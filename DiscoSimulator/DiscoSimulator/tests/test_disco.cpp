// Suite de pruebas unitarias minimalista (sin frameworks externos como
// GoogleTest, para no depender de descargas de red). Cada prueba compara
// un valor obtenido contra el esperado con una tolerancia razonable para
// aritmetica de punto flotante.
#include "../backend/DiscoFactory.hpp"
#include "../backend/Exceptions.hpp"
#include "../backend/Simulador.hpp"
#include <cmath>
#include <cstdio>
#include <string>

static int pruebasEjecutadas = 0;
static int pruebasFallidas = 0;

static void verificar(bool condicion, const std::string& nombrePrueba) {
    pruebasEjecutadas++;
    if (condicion) {
        std::printf("[PASS] %s\n", nombrePrueba.c_str());
    } else {
        std::printf("[FAIL] %s\n", nombrePrueba.c_str());
        pruebasFallidas++;
    }
}

static bool cercaDe(double a, double b, double tolerancia = 1e-6) {
    return std::fabs(a - b) < tolerancia;
}

static void test_hdd_archivo_unico() {
    auto hdd = DiscoFactory::crear(TipoDisco::HDD);
    // 150 MB a 150 MB/s = 1s de transferencia + 0.015s de latencia
    double t = hdd->calcularTiempoArchivoUnico(150.0);
    verificar(cercaDe(t, 1.015), "HDD: tiempo archivo unico de 150MB debe ser ~1.015s");
}

static void test_ssd_archivo_unico() {
    auto ssd = DiscoFactory::crear(TipoDisco::SSD_SATA);
    // 550 MB a 550 MB/s = 1s + 0.0001s de latencia
    double t = ssd->calcularTiempoArchivoUnico(550.0);
    verificar(cercaDe(t, 1.0001), "SSD SATA: tiempo archivo unico de 550MB debe ser ~1.0001s");
}

static void test_nvme_archivo_unico() {
    auto nvme = DiscoFactory::crear(TipoDisco::NVME);
    // 7000 MB a 7000 MB/s = 1s + 0.00002s de latencia
    double t = nvme->calcularTiempoArchivoUnico(7000.0);
    verificar(cercaDe(t, 1.00002), "NVMe: tiempo archivo unico de 7000MB debe ser ~1.00002s");
}

static void test_hdd_penalizacion_fragmentada() {
    auto hdd = DiscoFactory::crear(TipoDisco::HDD);
    // Base sin penalizacion: 10 archivos * 0.015s latencia + (10*1MB)/150MB/s
    double base = 10 * 0.015 + (10.0 / 150.0);
    double esperado = base * 1.10; // HDD aplica 10% extra por reposicionamiento
    double obtenido = hdd->calcularTiempoFragmentado(1.0, 10);
    verificar(cercaDe(obtenido, esperado), "HDD: fragmentado debe incluir 10% de penalizacion mecanica");
}

static void test_ssd_sin_penalizacion_fragmentada() {
    auto ssd = DiscoFactory::crear(TipoDisco::SSD_SATA);
    double esperado = 10 * (0.10 / 1000.0) + (10.0 / 550.0);
    double obtenido = ssd->calcularTiempoFragmentado(1.0, 10);
    verificar(cercaDe(obtenido, esperado), "SSD SATA: fragmentado NO debe llevar penalizacion mecanica");
}

static void test_factory_crea_conjunto_estandar() {
    auto discos = DiscoFactory::crearConjuntoEstandar();
    verificar(discos.size() == 3, "Factory: el conjunto estandar debe tener 3 tecnologias");
    verificar(discos[0]->getNombre() == "HDD Mecanico", "Factory: primer elemento debe ser HDD");
    verificar(discos[1]->getNombre() == "SSD SATA", "Factory: segundo elemento debe ser SSD SATA");
    verificar(discos[2]->getNombre() == "NVMe M.2", "Factory: tercer elemento debe ser NVMe M.2");
}

static void test_simulador_rechaza_entradas_invalidas() {
    Simulador sim;
    bool lanzoExcepcion = false;
    try {
        sim.simularArchivoUnico(-5.0);
    } catch (const EntradaInvalidaException&) {
        lanzoExcepcion = true;
    }
    verificar(lanzoExcepcion, "Simulador: debe rechazar tamanos negativos con EntradaInvalidaException");

    lanzoExcepcion = false;
    try {
        sim.simularFragmentado(1.0, 0);
    } catch (const EntradaInvalidaException&) {
        lanzoExcepcion = true;
    }
    verificar(lanzoExcepcion, "Simulador: debe rechazar cantidad de archivos igual a cero");
}

static void test_simulador_calcula_aceleracion() {
    Simulador sim;
    auto resultados = sim.simularArchivoUnico(1000.0);
    double factor = sim.calcularFactorAceleracion(resultados);
    verificar(factor > 1.0, "Simulador: NVMe debe ser mas rapido que HDD (factor > 1)");
}

int main() {
    std::printf("=== Ejecutando pruebas unitarias ===\n\n");

    test_hdd_archivo_unico();
    test_ssd_archivo_unico();
    test_nvme_archivo_unico();
    test_hdd_penalizacion_fragmentada();
    test_ssd_sin_penalizacion_fragmentada();
    test_factory_crea_conjunto_estandar();
    test_simulador_rechaza_entradas_invalidas();
    test_simulador_calcula_aceleracion();

    std::printf("\n=== Resultado: %d/%d pruebas exitosas ===\n",
                pruebasEjecutadas - pruebasFallidas, pruebasEjecutadas);

    return pruebasFallidas == 0 ? 0 : 1;
}
