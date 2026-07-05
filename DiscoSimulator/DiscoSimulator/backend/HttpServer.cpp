#include "HttpServer.hpp"
#include "Logger.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define CERRAR_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
    #define CERRAR_SOCKET(s) close(s)
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

namespace {
    // Inicializacion/limpieza de Winsock. En plataformas POSIX no hace nada.
    struct InicializadorRed {
        InicializadorRed() {
#if defined(_WIN32)
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        }
        ~InicializadorRed() {
#if defined(_WIN32)
            WSACleanup();
#endif
        }
    };
}

HttpServer::HttpServer(int puerto) : puerto(puerto) {}
HttpServer::~HttpServer() {}

void HttpServer::agregarRuta(const std::string& metodo, const std::string& ruta, ManejadorRuta manejador) {
    rutas[metodo + " " + ruta] = std::move(manejador);
}

void HttpServer::servirArchivosEstaticos(const std::string& directorioBase) {
    directorioEstatico = directorioBase;
}

std::string HttpServer::tipoMimePorExtension(const std::string& ruta) {
    auto terminaCon = [&](const std::string& ext) {
        return ruta.size() >= ext.size() && ruta.compare(ruta.size() - ext.size(), ext.size(), ext) == 0;
    };
    if (terminaCon(".html")) return "text/html; charset=utf-8";
    if (terminaCon(".js"))   return "application/javascript; charset=utf-8";
    if (terminaCon(".css"))  return "text/css; charset=utf-8";
    if (terminaCon(".json")) return "application/json; charset=utf-8";
    if (terminaCon(".svg"))  return "image/svg+xml";
    return "application/octet-stream";
}

bool HttpServer::leerArchivo(const std::string& ruta, std::string& contenido) {
    std::ifstream in(ruta, std::ios::binary);
    if (!in.is_open()) return false;
    std::ostringstream oss;
    oss << in.rdbuf();
    contenido = oss.str();
    return true;
}

HttpResponse HttpServer::manejarSolicitud(const HttpRequest& req) {
    // CORS: se permite cualquier origen para facilitar pruebas locales
    // (ej. abrir el HTML directamente como archivo mientras se depura).
    std::string clave = req.metodo + " " + req.ruta;
    auto it = rutas.find(clave);
    if (it != rutas.end()) {
        return it->second(req);
    }

    // Si no coincide con ninguna ruta de API, se intenta servir como
    // archivo estatico del frontend.
    if (req.metodo == "GET" && !directorioEstatico.empty()) {
        std::string rutaArchivo = req.ruta;
        if (rutaArchivo == "/" ) rutaArchivo = "/index.html";
        std::string rutaCompleta = directorioEstatico + rutaArchivo;

        std::string contenido;
        if (leerArchivo(rutaCompleta, contenido)) {
            HttpResponse resp;
            resp.codigo = 200;
            resp.tipoContenido = tipoMimePorExtension(rutaCompleta);
            resp.cuerpo = contenido;
            return resp;
        }
    }

    HttpResponse resp;
    resp.codigo = 404;
    resp.tipoContenido = "application/json";
    resp.cuerpo = "{\"error\":\"Ruta no encontrada\"}";
    return resp;
}

void HttpServer::iniciar() {
    static InicializadorRed inicializador; // se ejecuta una sola vez

    socket_t servidorFd = socket(AF_INET, SOCK_STREAM, 0);
    if (servidorFd == INVALID_SOCKET) {
        Logger::log(Logger::Nivel::ERROR, "No se pudo crear el socket del servidor.");
        std::cerr << "Error: no se pudo crear el socket.\n";
        return;
    }

    int opt = 1;
#if defined(_WIN32)
    setsockopt(servidorFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(servidorFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in direccion{};
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(static_cast<unsigned short>(puerto));

    if (bind(servidorFd, reinterpret_cast<sockaddr*>(&direccion), sizeof(direccion)) == SOCKET_ERROR) {
        std::cerr << "Error: no se pudo enlazar el puerto " << puerto
                  << ". ¿Ya hay otro proceso usandolo?\n";
        Logger::log(Logger::Nivel::ERROR, "No se pudo enlazar el puerto " + std::to_string(puerto));
        CERRAR_SOCKET(servidorFd);
        return;
    }

    if (listen(servidorFd, 16) == SOCKET_ERROR) {
        std::cerr << "Error: no se pudo escuchar en el puerto " << puerto << ".\n";
        CERRAR_SOCKET(servidorFd);
        return;
    }

    std::cout << "Servidor backend escuchando en http://localhost:" << puerto << "\n";
    std::cout << "Abre esa direccion en tu navegador para ver el frontend conectado.\n";
    std::cout << "Presiona Ctrl+C para detener el servidor.\n\n";
    Logger::log(Logger::Nivel::INFO, "Servidor iniciado en el puerto " + std::to_string(puerto));

    const size_t TAM_BUFFER = 65536;
    std::string buffer(TAM_BUFFER, '\0');

    while (true) {
        sockaddr_in clienteAddr{};
#if defined(_WIN32)
        int clienteLen = sizeof(clienteAddr);
#else
        socklen_t clienteLen = sizeof(clienteAddr);
#endif
        socket_t clienteFd = accept(servidorFd, reinterpret_cast<sockaddr*>(&clienteAddr), &clienteLen);
        if (clienteFd == INVALID_SOCKET) continue;

        // --- Leer la solicitud completa (encabezados + cuerpo) ---
        std::string solicitudCruda;
        long contentLength = -1;
        size_t finEncabezados = std::string::npos;

        while (true) {
#if defined(_WIN32)
            int recibidos = recv(clienteFd, &buffer[0], static_cast<int>(TAM_BUFFER), 0);
#else
            ssize_t recibidos = recv(clienteFd, &buffer[0], TAM_BUFFER, 0);
#endif
            if (recibidos <= 0) break;
            solicitudCruda.append(buffer.data(), static_cast<size_t>(recibidos));

            if (finEncabezados == std::string::npos) {
                finEncabezados = solicitudCruda.find("\r\n\r\n");
                if (finEncabezados != std::string::npos) {
                    size_t posCL = solicitudCruda.find("Content-Length:");
                    if (posCL != std::string::npos && posCL < finEncabezados) {
                        contentLength = std::strtol(solicitudCruda.c_str() + posCL + 16, nullptr, 10);
                    } else {
                        contentLength = 0;
                    }
                }
            }

            if (finEncabezados != std::string::npos) {
                size_t cuerpoRecibido = solicitudCruda.size() - (finEncabezados + 4);
                if (static_cast<long>(cuerpoRecibido) >= contentLength) break;
            }
            if (static_cast<size_t>(recibidos) < TAM_BUFFER) {
                // El cliente probablemente ya no tiene mas que enviar.
                if (finEncabezados != std::string::npos) break;
            }
        }

        // --- Parsear linea de solicitud (METODO RUTA HTTP/1.1) ---
        HttpRequest req;
        size_t finPrimeraLinea = solicitudCruda.find("\r\n");
        std::string primeraLinea = (finPrimeraLinea != std::string::npos)
            ? solicitudCruda.substr(0, finPrimeraLinea) : solicitudCruda;

        std::istringstream lineaStream(primeraLinea);
        std::string rutaConQuery;
        lineaStream >> req.metodo >> rutaConQuery;

        size_t posInterrogacion = rutaConQuery.find('?');
        req.ruta = rutaConQuery.substr(0, posInterrogacion);
        if (posInterrogacion != std::string::npos) {
            std::string queryString = rutaConQuery.substr(posInterrogacion + 1);
            std::istringstream qs(queryString);
            std::string par;
            while (std::getline(qs, par, '&')) {
                size_t posIgual = par.find('=');
                if (posIgual != std::string::npos) {
                    req.query[par.substr(0, posIgual)] = par.substr(posIgual + 1);
                }
            }
        }

        if (finEncabezados != std::string::npos) {
            req.cuerpo = solicitudCruda.substr(finEncabezados + 4);
        }

        // Manejo de preflight CORS
        HttpResponse resp;
        if (req.metodo == "OPTIONS") {
            resp.codigo = 204;
            resp.cuerpo = "";
        } else {
            resp = manejarSolicitud(req);
        }

        std::ostringstream out;
        out << "HTTP/1.1 " << resp.codigo << " OK\r\n";
        out << "Content-Type: " << resp.tipoContenido << "\r\n";
        out << "Content-Length: " << resp.cuerpo.size() << "\r\n";
        out << "Access-Control-Allow-Origin: *\r\n";
        out << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        out << "Access-Control-Allow-Headers: Content-Type\r\n";
        out << "Connection: close\r\n";
        out << "\r\n";
        out << resp.cuerpo;

        std::string respuestaFinal = out.str();
#if defined(_WIN32)
        send(clienteFd, respuestaFinal.c_str(), static_cast<int>(respuestaFinal.size()), 0);
#else
        send(clienteFd, respuestaFinal.c_str(), respuestaFinal.size(), 0);
#endif
        CERRAR_SOCKET(clienteFd);
    }

    CERRAR_SOCKET(servidorFd);
}
