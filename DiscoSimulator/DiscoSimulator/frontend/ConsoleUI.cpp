#include "ConsoleUI.hpp"
#include "../backend/Exceptions.hpp"
#include "../backend/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <limits>

#if defined(_WIN32)
#include <windows.h>
// Habilita el procesamiento de secuencias ANSI (colores) en consolas de
// Windows modernas (Windows 10+). Si falla, el programa sigue funcionando
// sin color, sin romperse.
static void habilitarColoresWindows() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD modo = 0;
    if (!GetConsoleMode(hOut, &modo)) return;
    modo |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, modo);
}
#else
static void habilitarColoresWindows() { /* no aplica fuera de Windows */ }
#endif

ConsoleUI::ConsoleUI(Simulador& sim) : simulador(sim) {
    habilitarColoresWindows();
}

std::string ConsoleUI::colorParaDisco(const std::string& nombreDisco) const {
    if (nombreDisco.find("HDD") != std::string::npos) return Color::ROJO;
    if (nombreDisco.find("SATA") != std::string::npos) return Color::VERDE;
    return Color::AZUL; // NVMe
}

void ConsoleUI::mostrarBanner() const {
    std::cout << Color::NEGRITA << Color::CIAN;
    std::cout << "========================================\n";
    std::cout << "   SIMULADOR DE RENDIMIENTO DE DISCOS    \n";
    std::cout << "   HDD  vs  SSD SATA  vs  NVMe M.2       \n";
    std::cout << "========================================\n";
    std::cout << Color::RESET;
}

void ConsoleUI::mostrarMenu() const {
    std::cout << "\n" << Color::NEGRITA << "--- MENU PRINCIPAL ---" << Color::RESET << "\n";
    std::cout << "1. Simular archivo unico (secuencial masivo)\n";
    std::cout << "2. Simular archivos fragmentados (conjunto masivo)\n";
    std::cout << "3. Ver descripcion tecnica de cada tecnologia\n";
    std::cout << "4. Ver historial de simulaciones\n";
    std::cout << "5. Exportar resultados (CSV + datos.js para reporte web)\n";
    std::cout << "6. Salir\n";
    std::cout << "Opcion: ";
}

int ConsoleUI::leerOpcion() const {
    int opcion;
    while (!(std::cin >> opcion)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << Color::AMARILLO << "Entrada invalida. Ingrese un numero de opcion: " << Color::RESET;
    }
    return opcion;
}

double ConsoleUI::leerDouble(const std::string& mensaje) const {
    double valor;
    std::cout << mensaje;
    while (!(std::cin >> valor) || valor <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << Color::AMARILLO << "Valor invalido. Ingrese un numero positivo: " << Color::RESET;
    }
    return valor;
}

long ConsoleUI::leerEntero(const std::string& mensaje) const {
    long valor;
    std::cout << mensaje;
    while (!(std::cin >> valor) || valor <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << Color::AMARILLO << "Valor invalido. Ingrese un numero entero positivo: " << Color::RESET;
    }
    return valor;
}

void ConsoleUI::mostrarResultados(const std::vector<ResultadoSimulacion>& resultados) const {
    std::cout << "\n" << Color::NEGRITA << std::left << std::setw(15) << "TECNOLOGIA"
              << " | " << std::setw(20) << "TIEMPO DE RESPUESTA" << Color::RESET << "\n";
    std::cout << std::string(40, '-') << "\n";
    for (const auto& r : resultados) {
        std::cout << colorParaDisco(r.disco) << std::left << std::setw(15) << r.disco << Color::RESET
                   << " | " << std::fixed << std::setprecision(6) << r.tiempoSegundos << " s\n";
    }
    double factor = simulador.calcularFactorAceleracion(resultados);
    std::cout << "\n" << Color::AMARILLO << "================ CONCLUSION ================\n";
    std::cout << "NVMe completo la tarea " << std::fixed << std::setprecision(1)
               << factor << "x mas rapido que el HDD clasico.\n";
    std::cout << "=============================================" << Color::RESET << "\n";
}

void ConsoleUI::flujoArchivoUnico() {
    double tamano = leerDouble("Tamano del archivo unico a simular (en MB): ");
    try {
        auto resultados = simulador.simularArchivoUnico(tamano);
        mostrarResultados(resultados);
    } catch (const EntradaInvalidaException& e) {
        std::cout << Color::ROJO << "Entrada rechazada: " << e.what() << Color::RESET << "\n";
    }
}

void ConsoleUI::flujoFragmentado() {
    double tamanoArchivo = leerDouble("Tamano de CADA archivo fragmentado (en MB): ");
    long cantidad = leerEntero("Cantidad de archivos a transferir: ");
    try {
        auto resultados = simulador.simularFragmentado(tamanoArchivo, cantidad);
        mostrarResultados(resultados);
    } catch (const EntradaInvalidaException& e) {
        std::cout << Color::ROJO << "Entrada rechazada: " << e.what() << Color::RESET << "\n";
    }
}

void ConsoleUI::flujoDescripciones() const {
    std::cout << "\n" << Color::NEGRITA << "--- DESCRIPCION TECNICA (polimorfismo) ---" << Color::RESET << "\n";
    for (const auto& disco : simulador.getDiscos()) {
        std::cout << "\n" << colorParaDisco(disco->getNombre()) << "[" << disco->getNombre() << "]" << Color::RESET << "\n";
        std::cout << "Velocidad: " << disco->getVelocidad() << " MB/s | "
                   << "Latencia: " << disco->getLatenciaMs() << " ms\n";
        std::cout << disco->describirTecnologia() << "\n";
    }
}

void ConsoleUI::flujoHistorial() const {
    const auto& historial = simulador.getHistorial();
    if (historial.empty()) {
        std::cout << "\n" << Color::GRIS << "Aun no se ha ejecutado ninguna simulacion." << Color::RESET << "\n";
        return;
    }
    std::cout << "\n" << Color::NEGRITA << "--- HISTORIAL DE SIMULACIONES (" << historial.size() << " registros) ---" << Color::RESET << "\n";
    std::string escenarioActual;
    for (const auto& r : historial) {
        if (r.escenario != escenarioActual) {
            escenarioActual = r.escenario;
            std::cout << "\n" << Color::CIAN << "> " << escenarioActual << Color::RESET << "\n";
        }
        std::cout << "  " << colorParaDisco(r.disco) << std::left << std::setw(15) << r.disco << Color::RESET
                   << " " << std::fixed << std::setprecision(6) << r.tiempoSegundos << " s\n";
    }
}

void ConsoleUI::flujoExportar() const {
    try {
        simulador.exportarCSV("resultados.csv");
        simulador.exportarJS("web/datos.js");
        std::cout << "\n" << Color::VERDE << "Exportado correctamente:" << Color::RESET << "\n";
        std::cout << " - resultados.csv\n";
        std::cout << " - web/datos.js (usado por el reporte grafico web/index.html)\n";
    } catch (const EntradaInvalidaException& e) {
        std::cout << Color::AMARILLO << e.what() << Color::RESET << "\n";
    } catch (const ArchivoException& e) {
        std::cout << Color::ROJO << "Error al exportar: " << e.what() << Color::RESET << "\n";
        Logger::log(Logger::Nivel::ERROR, e.what());
    }
}

void ConsoleUI::ejecutar() {
    mostrarBanner();
    bool continuar = true;
    while (continuar) {
        mostrarMenu();
        int opcion = leerOpcion();
        try {
            switch (opcion) {
                case 1: flujoArchivoUnico(); break;
                case 2: flujoFragmentado(); break;
                case 3: flujoDescripciones(); break;
                case 4: flujoHistorial(); break;
                case 5: flujoExportar(); break;
                case 6:
                    std::cout << "\nSaliendo del simulador. Hasta pronto.\n";
                    continuar = false;
                    break;
                default:
                    std::cout << Color::AMARILLO << "\nOpcion no valida.\n" << Color::RESET;
            }
        } catch (const SimuladorException& e) {
            // Red de seguridad: cualquier excepcion del dominio no debe
            // tumbar el programa; se informa al usuario y se continua.
            std::cout << Color::ROJO << "Ocurrio un error: " << e.what() << Color::RESET << "\n";
            Logger::log(Logger::Nivel::ERROR, e.what());
        }
    }
}
