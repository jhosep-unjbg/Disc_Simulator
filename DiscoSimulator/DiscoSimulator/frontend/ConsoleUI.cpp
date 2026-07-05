#include "ConsoleUI.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

ConsoleUI::ConsoleUI(Simulador& sim) : simulador(sim) {}

void ConsoleUI::mostrarBanner() const {
    std::cout << "========================================\n";
    std::cout << "   SIMULADOR DE RENDIMIENTO DE DISCOS    \n";
    std::cout << "   HDD  vs  SSD SATA  vs  NVMe M.2       \n";
    std::cout << "========================================\n";
}

void ConsoleUI::mostrarMenu() const {
    std::cout << "\n--- MENU PRINCIPAL ---\n";
    std::cout << "1. Simular archivo unico (secuencial masivo)\n";
    std::cout << "2. Simular archivos fragmentados (conjunto masivo)\n";
    std::cout << "3. Ver descripcion tecnica de cada tecnologia\n";
    std::cout << "4. Exportar resultados (CSV + datos.js para reporte web)\n";
    std::cout << "5. Salir\n";
    std::cout << "Opcion: ";
}

int ConsoleUI::leerOpcion() const {
    int opcion;
    while (!(std::cin >> opcion)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Entrada invalida. Ingrese un numero de opcion: ";
    }
    return opcion;
}

double ConsoleUI::leerDouble(const std::string& mensaje) const {
    double valor;
    std::cout << mensaje;
    while (!(std::cin >> valor) || valor <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Valor invalido. Ingrese un numero positivo: ";
    }
    return valor;
}

long ConsoleUI::leerEntero(const std::string& mensaje) const {
    long valor;
    std::cout << mensaje;
    while (!(std::cin >> valor) || valor <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Valor invalido. Ingrese un numero entero positivo: ";
    }
    return valor;
}

void ConsoleUI::mostrarResultados(const std::vector<ResultadoSimulacion>& resultados) const {
    std::cout << "\n" << std::left << std::setw(15) << "TECNOLOGIA"
              << " | " << std::setw(20) << "TIEMPO DE RESPUESTA" << "\n";
    std::cout << std::string(40, '-') << "\n";
    for (const auto& r : resultados) {
        std::cout << std::left << std::setw(15) << r.disco << " | "
                   << std::fixed << std::setprecision(6) << r.tiempoSegundos << " s\n";
    }
    double factor = simulador.calcularFactorAceleracion(resultados);
    std::cout << "\n================ CONCLUSION ================\n";
    std::cout << "NVMe completo la tarea " << std::fixed << std::setprecision(1)
               << factor << "x mas rapido que el HDD clasico.\n";
    std::cout << "=============================================\n";
}

void ConsoleUI::flujoArchivoUnico() {
    double tamano = leerDouble("Tamano del archivo unico a simular (en MB): ");
    auto resultados = simulador.simularArchivoUnico(tamano);
    mostrarResultados(resultados);
}

void ConsoleUI::flujoFragmentado() {
    double tamanoArchivo = leerDouble("Tamano de CADA archivo fragmentado (en MB): ");
    long cantidad = leerEntero("Cantidad de archivos a transferir: ");
    auto resultados = simulador.simularFragmentado(tamanoArchivo, cantidad);
    mostrarResultados(resultados);
}

void ConsoleUI::flujoDescripciones() const {
    std::cout << "\n--- DESCRIPCION TECNICA (polimorfismo) ---\n";
    for (const auto& disco : simulador.getDiscos()) {
        std::cout << "\n[" << disco->getNombre() << "]\n";
        std::cout << "Velocidad: " << disco->getVelocidad() << " MB/s | "
                   << "Latencia: " << disco->getLatenciaMs() << " ms\n";
        std::cout << disco->describirTecnologia() << "\n";
    }
}

void ConsoleUI::flujoExportar() const {
    if (simulador.getHistorial().empty()) {
        std::cout << "\nAun no hay resultados que exportar. Ejecute alguna simulacion primero.\n";
        return;
    }
    try {
        simulador.exportarCSV("resultados.csv");
        simulador.exportarJS("web/datos.js");
        std::cout << "\nExportado correctamente:\n";
        std::cout << " - resultados.csv\n";
        std::cout << " - web/datos.js (usado por el reporte grafico web/index.html)\n";
    } catch (const std::exception& e) {
        std::cout << "Error al exportar: " << e.what() << "\n";
    }
}

void ConsoleUI::ejecutar() {
    mostrarBanner();
    bool continuar = true;
    while (continuar) {
        mostrarMenu();
        int opcion = leerOpcion();
        switch (opcion) {
            case 1: flujoArchivoUnico(); break;
            case 2: flujoFragmentado(); break;
            case 3: flujoDescripciones(); break;
            case 4: flujoExportar(); break;
            case 5:
                std::cout << "\nSaliendo del simulador. Hasta pronto.\n";
                continuar = false;
                break;
            default:
                std::cout << "\nOpcion no valida.\n";
        }
    }
}
