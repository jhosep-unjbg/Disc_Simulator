# Simulador de Rendimiento de Discos (C++) — HDD vs SSD SATA vs NVMe M.2

Proyecto universitario de Arquitectura de Computadoras. Arquitectura en capas:

```
backend/    -> Logica de negocio y calculo (POO, herencia, polimorfismo)
frontend/   -> Interfaz de consola (menu, validacion, tablas)
web/        -> Frontend grafico (HTML + Chart.js) que consume los datos
              exportados por el backend en C++
main.cpp    -> Punto de entrada, conecta backend con frontend
```

## Compilar

```bash
make
```

## Ejecutar (consola)

```bash
./simulador_discos
```

Menu disponible:
1. Simular archivo unico (secuencial masivo, ej. pelicula de 40 GB)
2. Simular archivos fragmentados (ej. 15 000 fotos de 1 MB) — expone el
   cuello de botella del HDD por el movimiento mecanico del cabezal
3. Ver descripcion tecnica de cada tecnologia (demuestra polimorfismo)
4. Exportar resultados a `resultados.csv` y `web/datos.js`
5. Salir

## Ver el reporte grafico (frontend web)

Tras exportar (opcion 4), abrir `web/index.html` en el navegador
(doble clic es suficiente, no requiere servidor). Muestra un grafico de
barras comparativo por escenario y una tabla con el factor de aceleracion
de cada tecnologia frente al HDD.

## Fundamento matematico

```
Tiempo Total (s) = Latencia (s) + Tamaño del Archivo (MB) / Velocidad (MB/s)
```

Para el escenario fragmentado, la latencia se paga una vez por cada
archivo accedido (Σ), mientras que la transferencia se acumula sobre el
tamaño total. Esto es lo que expone matematicamente por que el HDD sufre
mas con archivos pequeños y numerosos que con un archivo grande.

## Decisiones de diseño (nivel intermedio-avanzado)

- **Herencia + polimorfismo real**: `HDD` sobrescribe
  `calcularTiempoFragmentado` para modelar una penalizacion mecanica
  adicional del 10% por reposicionamiento repetido del cabezal, algo que
  `SSDSata` y `NVMe` no necesitan.
- **Separacion backend/frontend**: `Simulador` no sabe nada de consola ni
  de HTML; `ConsoleUI` no calcula nada, solo orquesta E/S.
- **Interoperabilidad backend (C++) -> frontend (web)** mediante
  generacion de un archivo `datos.js` consumido por Chart.js, evitando
  problemas de CORS al abrir el HTML directamente desde el disco.
