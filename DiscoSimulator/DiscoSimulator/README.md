# Simulador de Rendimiento de Discos (C++) — HDD vs SSD SATA vs NVMe M.2

Proyecto universitario de Arquitectura de Computadoras. Arquitectura
profesional en capas, con **dos modos de uso**:

- **Modo consola**: ejecutable offline con menu interactivo (exporta CSV/JS).
- **Modo servidor**: backend en C++ que expone una API HTTP/JSON y sirve
  el frontend web, todo conectado en tiempo real en `http://localhost:8080`.

## Estructura del proyecto

```
backend/
  Disco.hpp/.cpp          -> Clase base abstracta (POO)
  HDD.hpp/.cpp             -> Subclase: penaliza 10% en acceso fragmentado
  SSDSata.hpp/.cpp         -> Subclase
  NVMe.hpp/.cpp            -> Subclase
  DiscoFactory.hpp/.cpp    -> Patron Factory Method (crea los discos)
  Simulador.hpp/.cpp       -> Logica de negocio y el historial de sesion
  Exceptions.hpp           -> Jerarquia de excepciones propias
  Logger.hpp/.cpp          -> Logging con marca de tiempo (simulador.log)
  HttpServer.hpp/.cpp      -> Servidor HTTP minimalista (sockets crudos,
                               multiplataforma Windows/Linux/Mac)
  JsonUtil.hpp/.cpp        -> Utilidades minimas para leer/escribir JSON
frontend/
  ConsoleUI.hpp/.cpp       -> Interfaz de consola (colores, menus, validacion)
  Colores.hpp              -> Codigos ANSI de color
web/
  index.html               -> Frontend conectado EN VIVO al backend via fetch()
tests/
  test_disco.cpp           -> Pruebas unitarias (sin dependencias externas)
main.cpp                   -> Punto de entrada del modo consola
main_servidor.cpp          -> Punto de entrada del modo servidor (API + web)
CMakeLists.txt             -> Build multiplataforma (recomendado en Windows)
Makefile                   -> Build alternativo con g++ directo
```

## Compilar

### Opcion A: CMake 

```bash
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

Genera `build/simulador_discos` y `build/servidor_discos` (o dentro de
`build/Debug/` si usas Visual Studio).

### Opcion B: Makefile (Linux/Mac/MSYS2)

```bash
make            # compila ambos ejecutables
make test       # corre las 12 pruebas unitarias
```

### Opcion C: g++ directo

```bash
# Modo consola
g++ -std=c++17 -O2 -o simulador_discos.exe main.cpp backend/Disco.cpp backend/HDD.cpp backend/SSDSata.cpp backend/NVMe.cpp backend/DiscoFactory.cpp backend/Simulador.cpp backend/Logger.cpp frontend/ConsoleUI.cpp

# Modo servidor (en Windows agregar -lws2_32 al final)
g++ -std=c++17 -O2 -o servidor_discos.exe main_servidor.cpp backend/Disco.cpp backend/HDD.cpp backend/SSDSata.cpp backend/NVMe.cpp backend/DiscoFactory.cpp backend/Simulador.cpp backend/Logger.cpp backend/HttpServer.cpp backend/JsonUtil.cpp -lws2_32
```

## Modo servidor: backend y frontend conectados en tiempo real

Este es el modo recomendado para ver "todo conectado":

```bash
./servidor_discos          # usa el puerto 8080 por defecto
./servidor_discos 5000     # o especifica otro puerto
```

Veras en consola:
```
Servidor backend escuchando en http://localhost:8080
```

Abre esa direccion en tu navegador. La pagina:
- Envia formularios al backend (`POST /api/simular/archivo-unico`,
  `POST /api/simular/fragmentado`) y grafica la respuesta al instante.
- Consulta `GET /api/historial` para mostrar todo lo simulado en la sesion.
- Consulta `GET /api/discos` para las descripciones tecnicas.
- Puede exportar CSV/JS a disco con `POST /api/exportar`.

Todo corre desde el mismo origen (`http://localhost:8080`), por lo que no
hay problemas de CORS. Para detener el servidor, `Ctrl+C` en la terminal.

### Endpoints de la API

| Metodo | Ruta                              | Body (JSON)                                 | Respuesta                          |
|--------|------------------------------------|----------------------------------------------|-------------------------------------|
| GET    | /api/discos                       | -                                              | Info tecnica de cada tecnologia     |
| POST   | /api/simular/archivo-unico        | `{"tamanoMB": 40000}`                        | Resultados + factor de aceleracion  |
| POST   | /api/simular/fragmentado          | `{"tamanoPorArchivoMB":1,"cantidadArchivos":15000}` | Resultados + factor de aceleracion |
| GET    | /api/historial                    | -                                              | Todas las simulaciones de la sesion |
| POST   | /api/exportar                     | -                                              | Genera resultados.csv y web/datos.js |

## Modo consola (offline)

```bash
./simulador_discos
```

Menu con las mismas simulaciones, pero interactuando por texto y
exportando manualmente a archivos. Util si no quieres levantar un
servidor o para la sustentacion en clase sin depender del navegador.

## Fundamento matematico

```
Tiempo Total (s) = Latencia (s) + Tamaño del Archivo (MB) / Velocidad (MB/s)
```

En el escenario fragmentado, la latencia se paga una vez por cada archivo
accedido (Σ), mientras que la transferencia se acumula sobre el tamaño
total. El HDD ademas incorpora una penalizacion mecanica adicional del
10%, modelando el reposicionamiento fisico repetido del cabezal.

## Decisiones de diseño 

- **Arquitectura cliente-servidor real**: el backend en C++ (`HttpServer`)
  expone una API JSON consumida dinamicamente por el frontend web via
  `fetch()`, en lugar de depender solo de archivos estaticos exportados.
- **Servidor HTTP propio sin dependencias externas**: implementado sobre
  sockets BSD/Winsock con codigo condicional por plataforma
  (`#ifdef _WIN32`), suficiente para fines academicos y demostrar
  comprension de la capa de transporte.
- **Patron Factory Method** (`DiscoFactory`): centraliza la creacion de
  discos (principio Abierto/Cerrado de SOLID).
- **Jerarquia de excepciones propia**: distingue errores de entrada de
  errores de archivo, con codigos HTTP apropiados (400 vs 500) en la API.
- **Logging**: toda simulacion, exportacion y error queda registrado con
  marca de tiempo en `simulador.log`.
- **Pruebas unitarias**: 12 casos (`make test`) que verifican formulas,
  polimorfismo, la fabrica y el manejo de errores.
- **Separacion estricta backend/frontend**: `Simulador` no sabe nada de
  HTTP ni de consola; se reutiliza identico en ambos modos de ejecucion.
### Resumen de Componentes y Capas
| Capa | Componente | Descripción / Responsabilidad |
| :--- | :--- | :--- |
| **Núcleo / Hardware** | `Disco.hpp` (Base)<br>`HDD`, `SSDSata`, `NVMe` | Define la interfaz polimórfica y aplica las ecuaciones de rendimiento específicas de cada tecnología. |
| **Lógica de Negocio** | `Simulador.hpp`<br>`DiscoFactory.hpp` | Controla el flujo de las simulaciones, mantiene el historial de la sesión y centraliza la creación de discos mediante patrones de diseño. |
| **Infraestructura** | `HttpServer.hpp`<br>`JsonUtil.hpp`<br>`Logger.hpp`<br>`Exceptions.hpp` | Levanta sockets de red nativos (multiplataforma), procesa payloads en texto plano, gestiona logs con marcas de tiempo y propaga errores tipificados. |
| **Interfaces (UI)** | `ConsoleUI.hpp` (CLI)<br>`index.html` (Web) | Canales de interacción con el usuario. Consumen la lógica del simulador mediante menús ANSI o peticiones asíncronas (`fetch`). |

### Matriz de Decisiones Técnicas de Arquitectura
| Criterio de Diseño | Solución Implementada | Justificación Computacional / Académica |
| :--- | :--- | :--- |
| **Portabilidad de Red** | Código condicional (`#ifdef _WIN32`) | Soporta Winsock en Windows (`-lws2_32`) y sockets POSIX (BSD) en Linux/Mac de forma transparente. |
| **Extensibilidad** | Patrón *Factory Method* | Cumple el Principio Abierto/Cerrado (SOLID). Agregar nuevos discos no altera la lógica del simulador. |
| **Desacoplamiento** | Separación Backend/Frontend | El dominio de la aplicación (`Simulador`) procesa datos puros, permitiendo intercambiar la consola y la web sin efectos secundarios. |
| **Consistencia** | Jerarquía de excepciones propia | Permite traducir errores del núcleo directamente en respuestas formateadas de consola o códigos de estado HTTP semánticos. |