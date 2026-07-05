#include "backend/Simulador.hpp"
#include "backend/HttpServer.hpp"
#include "backend/JsonUtil.hpp"
#include "backend/Exceptions.hpp"
#include "backend/Logger.hpp"
#include <sstream>
#include <iostream>
#include <cmath>

// Construye el JSON de un arreglo de resultados de simulacion.
static std::string resultadosAJson(const std::vector<ResultadoSimulacion>& resultados, double factorAceleracion) {
    std::ostringstream json;
    json << "{\"resultados\":[";
    for (size_t i = 0; i < resultados.size(); ++i) {
        const auto& r = resultados[i];
        json << "{\"escenario\":\"" << JsonUtil::escapar(r.escenario) << "\","
             << "\"disco\":\"" << JsonUtil::escapar(r.disco) << "\","
             << "\"tiempoSegundos\":" << r.tiempoSegundos << "}";
        if (i + 1 < resultados.size()) json << ",";
    }
    json << "],\"factorAceleracion\":" << factorAceleracion << "}";
    return json.str();
}

static std::string errorAJson(const std::string& mensaje) {
    return "{\"error\":\"" + JsonUtil::escapar(mensaje) + "\"}";
}

int main(int argc, char** argv) {
    int puerto = 8080;
    if (argc > 1) {
        puerto = std::atoi(argv[1]);
        if (puerto <= 0) puerto = 8080;
    }

    Simulador simulador; // Backend: toda la logica de negocio vive aqui

    HttpServer servidor(puerto);
    servidor.servirArchivosEstaticos("web"); // Frontend estatico (HTML/CSS/JS)

    // --- GET /api/discos: informacion tecnica de cada tecnologia ---
    servidor.agregarRuta("GET", "/api/discos", [&simulador](const HttpRequest&) -> HttpResponse {
        std::ostringstream json;
        json << "[";
        const auto& discos = simulador.getDiscos();
        for (size_t i = 0; i < discos.size(); ++i) {
            const auto& d = discos[i];
            json << "{\"nombre\":\"" << JsonUtil::escapar(d->getNombre()) << "\","
                 << "\"velocidadMBs\":" << d->getVelocidad() << ","
                 << "\"latenciaMs\":" << d->getLatenciaMs() << ","
                 << "\"descripcion\":\"" << JsonUtil::escapar(d->describirTecnologia()) << "\"}";
            if (i + 1 < discos.size()) json << ",";
        }
        json << "]";
        HttpResponse resp;
        resp.cuerpo = json.str();
        return resp;
    });

    // --- POST /api/simular/archivo-unico  body: {"tamanoMB": 40000} ---
    servidor.agregarRuta("POST", "/api/simular/archivo-unico", [&simulador](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp;
        try {
            double tamanoMB = JsonUtil::extraerNumero(req.cuerpo, "tamanoMB");
            auto resultados = simulador.simularArchivoUnico(tamanoMB);
            double factor = simulador.calcularFactorAceleracion(resultados);
            resp.cuerpo = resultadosAJson(resultados, factor);
        } catch (const EntradaInvalidaException& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        } catch (const std::exception& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        }
        return resp;
    });

    // --- POST /api/simular/fragmentado  body: {"tamanoPorArchivoMB":1,"cantidadArchivos":15000} ---
    servidor.agregarRuta("POST", "/api/simular/fragmentado", [&simulador](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp;
        try {
            double tamanoPorArchivoMB = JsonUtil::extraerNumero(req.cuerpo, "tamanoPorArchivoMB");
            long cantidadArchivos = JsonUtil::extraerEntero(req.cuerpo, "cantidadArchivos");
            auto resultados = simulador.simularFragmentado(tamanoPorArchivoMB, cantidadArchivos);
            double factor = simulador.calcularFactorAceleracion(resultados);
            resp.cuerpo = resultadosAJson(resultados, factor);
        } catch (const EntradaInvalidaException& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        } catch (const std::exception& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        }
        return resp;
    });

    // --- GET /api/historial: todas las simulaciones ejecutadas en la sesion ---
    servidor.agregarRuta("GET", "/api/historial", [&simulador](const HttpRequest&) -> HttpResponse {
        const auto& historial = simulador.getHistorial();
        std::ostringstream json;
        json << "[";
        for (size_t i = 0; i < historial.size(); ++i) {
            const auto& r = historial[i];
            json << "{\"escenario\":\"" << JsonUtil::escapar(r.escenario) << "\","
                 << "\"disco\":\"" << JsonUtil::escapar(r.disco) << "\","
                 << "\"tiempoSegundos\":" << r.tiempoSegundos << "}";
            if (i + 1 < historial.size()) json << ",";
        }
        json << "]";
        HttpResponse resp;
        resp.cuerpo = json.str();
        return resp;
    });

    // --- POST /api/exportar: genera resultados.csv y web/datos.js en disco ---
    servidor.agregarRuta("POST", "/api/exportar", [&simulador](const HttpRequest&) -> HttpResponse {
        HttpResponse resp;
        try {
            simulador.exportarCSV("resultados.csv");
            simulador.exportarJS("web/datos.js");
            resp.cuerpo = "{\"ok\":true,\"mensaje\":\"Exportado a resultados.csv y web/datos.js\"}";
        } catch (const std::exception& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        }
        return resp;
    });

    // --- GET /api/analisis/curva-tamano?min=1&max=1000000&puntos=25 ---
    // Genera, para cada tecnologia, una curva Tiempo(s) vs Tamano(MB) usando
    // puntos espaciados logaritmicamente. NO se guarda en el historial: es
    // analisis puro, no una "simulacion" puntual.
    servidor.agregarRuta("GET", "/api/analisis/curva-tamano", [&simulador](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp;
        try {
            double minMB = req.query.count("min") ? std::stod(req.query.at("min")) : 1.0;
            double maxMB = req.query.count("max") ? std::stod(req.query.at("max")) : 1000000.0;
            int puntos = req.query.count("puntos") ? std::stoi(req.query.at("puntos")) : 25;
            if (minMB <= 0 || maxMB <= minMB || puntos < 2 || puntos > 500) {
                throw std::runtime_error("Rango invalido para la curva (min/max/puntos).");
            }

            double logMin = std::log10(minMB);
            double logMax = std::log10(maxMB);

            std::ostringstream json;
            json << "{\"eje_x\":\"Tamano del archivo (MB)\",\"eje_y\":\"Tiempo (s)\",\"series\":[";
            const auto& discos = simulador.getDiscos();
            for (size_t d = 0; d < discos.size(); ++d) {
                json << "{\"disco\":\"" << JsonUtil::escapar(discos[d]->getNombre()) << "\",\"puntos\":[";
                for (int i = 0; i < puntos; ++i) {
                    double t = logMin + (logMax - logMin) * (static_cast<double>(i) / (puntos - 1));
                    double tamanoMB = std::pow(10.0, t);
                    double tiempo = discos[d]->calcularTiempoArchivoUnico(tamanoMB);
                    json << "{\"x\":" << tamanoMB << ",\"y\":" << tiempo << "}";
                    if (i + 1 < puntos) json << ",";
                }
                json << "]}";
                if (d + 1 < discos.size()) json << ",";
            }
            json << "]}";
            resp.cuerpo = json.str();
        } catch (const std::exception& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        }
        return resp;
    });

    // --- GET /api/analisis/curva-fragmentacion?tamanoArchivo=1&min=1&max=100000&puntos=25 ---
    // Genera, para cada tecnologia, una curva Tiempo(s) vs Cantidad de
    // archivos, con tamano por archivo fijo. Expone el crecimiento lineal
    // de la penalizacion por latencia (y la penalizacion mecanica extra del HDD).
    servidor.agregarRuta("GET", "/api/analisis/curva-fragmentacion", [&simulador](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp;
        try {
            double tamanoArchivoMB = req.query.count("tamanoArchivo") ? std::stod(req.query.at("tamanoArchivo")) : 1.0;
            long minArchivos = req.query.count("min") ? std::stol(req.query.at("min")) : 1;
            long maxArchivos = req.query.count("max") ? std::stol(req.query.at("max")) : 100000;
            int puntos = req.query.count("puntos") ? std::stoi(req.query.at("puntos")) : 25;
            if (tamanoArchivoMB <= 0 || minArchivos <= 0 || maxArchivos <= minArchivos || puntos < 2 || puntos > 500) {
                throw std::runtime_error("Rango invalido para la curva (tamanoArchivo/min/max/puntos).");
            }

            double logMin = std::log10(static_cast<double>(minArchivos));
            double logMax = std::log10(static_cast<double>(maxArchivos));

            std::ostringstream json;
            json << "{\"eje_x\":\"Cantidad de archivos\",\"eje_y\":\"Tiempo (s)\",\"series\":[";
            const auto& discos = simulador.getDiscos();
            for (size_t d = 0; d < discos.size(); ++d) {
                json << "{\"disco\":\"" << JsonUtil::escapar(discos[d]->getNombre()) << "\",\"puntos\":[";
                for (int i = 0; i < puntos; ++i) {
                    double t = logMin + (logMax - logMin) * (static_cast<double>(i) / (puntos - 1));
                    long cantidad = static_cast<long>(std::round(std::pow(10.0, t)));
                    double tiempo = discos[d]->calcularTiempoFragmentado(tamanoArchivoMB, cantidad);
                    json << "{\"x\":" << cantidad << ",\"y\":" << tiempo << "}";
                    if (i + 1 < puntos) json << ",";
                }
                json << "]}";
                if (d + 1 < discos.size()) json << ",";
            }
            json << "]}";
            resp.cuerpo = json.str();
        } catch (const std::exception& e) {
            resp.codigo = 400;
            resp.cuerpo = errorAJson(e.what());
        }
        return resp;
    });

    servidor.iniciar(); // bloqueante
    return 0;
}
