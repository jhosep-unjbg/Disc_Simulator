cmake_minimum_required(VERSION 3.15)
project(DiscoSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BACKEND_SOURCES
    backend/Disco.cpp
    backend/HDD.cpp
    backend/SSDSata.cpp
    backend/NVMe.cpp
    backend/DiscoFactory.cpp
    backend/Simulador.cpp
    backend/Logger.cpp
)

set(BACKEND_SERVIDOR_SOURCES
    ${BACKEND_SOURCES}
    backend/HttpServer.cpp
    backend/JsonUtil.cpp
)

set(FRONTEND_SOURCES
    frontend/ConsoleUI.cpp
)

# --- Ejecutable de consola (offline, exporta CSV/JS) ---
add_executable(simulador_discos main.cpp ${BACKEND_SOURCES} ${FRONTEND_SOURCES})
target_include_directories(simulador_discos PRIVATE ${CMAKE_SOURCE_DIR})

# --- Ejecutable servidor (backend + frontend web conectados en vivo) ---
add_executable(servidor_discos main_servidor.cpp ${BACKEND_SERVIDOR_SOURCES})
target_include_directories(servidor_discos PRIVATE ${CMAKE_SOURCE_DIR})
if (WIN32)
    target_link_libraries(servidor_discos PRIVATE ws2_32)
endif()

foreach(objetivo simulador_discos servidor_discos)
    if (MSVC)
        target_compile_options(${objetivo} PRIVATE /W4)
    else()
        target_compile_options(${objetivo} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# --- Pruebas unitarias ---
enable_testing()
add_executable(tests_simulador tests/test_disco.cpp ${BACKEND_SOURCES})
target_include_directories(tests_simulador PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME PruebasSimulador COMMAND tests_simulador)
