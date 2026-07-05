#pragma once
#include <string>
#include <map>
#include <functional>

/// Representa una solicitud HTTP entrante ya parseada.
struct HttpRequest {
    std::string metodo;   // "GET", "POST", etc.
    std::string ruta;     // "/api/discos" (sin query string)
    std::string cuerpo;   // Cuerpo crudo (para POST con JSON)
    std::map<std::string, std::string> query; // Parametros ?clave=valor
};

/// Representa la respuesta que el manejador de ruta debe construir.
struct HttpResponse {
    int codigo = 200;
    std::string tipoContenido = "application/json";
    std::string cuerpo;
};

using ManejadorRuta = std::function<HttpResponse(const HttpRequest&)>;

/// Servidor HTTP minimalista construido sobre sockets crudos (BSD sockets
/// en Linux/Mac, Winsock en Windows), sin dependencias externas. Sirve dos
/// propositos:
///   1) Exponer una API JSON para que el frontend web consuma el backend
///      en C++ en tiempo real (conexion cliente-servidor real).
///   2) Servir los archivos estaticos del frontend (HTML/CSS/JS) para que
///      todo corra desde un unico origen (http://localhost:PUERTO), sin
///      problemas de CORS.
///
/// No es apto para produccion (sin TLS, sin concurrencia real, parser
/// HTTP simplificado), pero es suficiente y didactico para un proyecto
/// academico que demuestra la arquitectura backend/frontend conectada.
class HttpServer {
public:
    explicit HttpServer(int puerto);
    ~HttpServer();

    void agregarRuta(const std::string& metodo, const std::string& ruta, ManejadorRuta manejador);
    void servirArchivosEstaticos(const std::string& directorioBase);

    /// Bloqueante: entra en el ciclo de aceptar conexiones. Ctrl+C para salir.
    void iniciar();

private:
    int puerto;
    std::string directorioEstatico;
    std::map<std::string, ManejadorRuta> rutas; // clave = "METODO ruta"

    HttpResponse manejarSolicitud(const HttpRequest& req);
    static bool leerArchivo(const std::string& ruta, std::string& contenido);
    static std::string tipoMimePorExtension(const std::string& ruta);
};
