# DOOMSat — DOOM en órbita con downlink ridículo

> ¿Es posible jugar a DOOM en un satélite a miles de kilómetros de la Tierra?  
> Este proyecto dice: **sí, si conviertes cada bit en oro**.

## Idea en una frase
Ejecutamos el motor de DOOM en una plataforma **AArch64 “space-grade”** y hacemos la partida **jugable desde tierra** enviando el vídeo por un **downlink muy limitado** (frames comprimidos) y subiendo los controles por un **uplink fiable**.

---

## Qué resuelve este repo (y cómo puntúa en el reto)

### 1) Eficiencia del enlace (Downlink) — 40%
- El frame se **reduce a media resolución**.
- Cada píxel se **cuantiza** a **8 bits**: **2 bits de intensidad + 6 bits de color**.
- Se aplica **compresión con `zlib`**.
- Resultado típico en nuestras pruebas: **~4 KB por frame**.
- Envío por **UDP** para priorizar velocidad; si se pierde un frame, **se salta**.

### 2) Misión: definición y asunciones — 30%
Para pruebas asumimos un escenario tipo **LEO (órbita baja)**:
- **Downlink:** 500 kb/s
- **Latencia (RTT aprox):** 100 ms
- **Pérdida:** 5% de tramas
Esto se **simula** con un **proxy/emulador de enlace** entre “satélite” (servidor) y estación de tierra (cliente).

### 3) Resiliencia y latencia — 20%
- **Vídeo (frames):** UDP (aceptamos pérdida ocasional).
- **Controles:** **TCP** (lo único que *no puede* perderse).
- El sistema tolera microcortes: si no llega un frame a tiempo, se mantiene el último frame válido y se continúa.

### 4) Viabilidad técnica — 10%
- **Servidor en C**, preparado para **AArch64** (cross-compile) y ejecución/emulación con **QEMU**.
- Comunicación por **sockets** (UDP/TCP).

### Bonus PL — 10% (si aplica)
Punto de extensión para FPGA:
- Acelerar **cuantización** y/o **empaquetado 2+6 bits**
- Acelerar **preprocesado** antes de `zlib`
- Integrar sensores internos (temperatura/IMU) como telemetría embebida en el stream

---

## Arquitectura

<img width="881" height="281" alt="DOOMSat2" src="https://github.com/user-attachments/assets/af540a33-64c8-4f84-8db2-46629c20ed23" />


- **Servidor (C)**: captura frame del motor, preprocesa, comprime y envía.
- **Cliente (Java)**: recibe datagramas, descomprime y renderiza; envía controles.

---

## Pipeline de compresión (Downlink)

1. **Downscale**: resolución /2  
2. **Cuantización por píxel**:
   - 2 bits → intensidad (luma / “brillo”)
   - 6 bits → color (paleta / crominancia simplificada)
3. **`zlib`**: compresión sin pérdidas sobre el buffer cuantizado
4. **UDP**: envío del frame (~4 KB típico)

---

## Protocolo de controles (Uplink)
- Canal **TCP** para garantizar entrega y orden.
- Mensajes pequeños (teclas/acciones) para minimizar latencia percibida.
- El cliente envía eventos; el servidor los inyecta en el motor.

---

## Simulación del enlace (Proxy)
Para reproducir condiciones satelitales, el proxy actúa como intermediario:
- limita ancho de banda,
- añade latencia,
- introduce pérdida..

---

## Dependencias

### Comunes
- Linux/macOS recomendado (Windows posible con WSL2)
- `git`

### Servidor (C)
- `gcc` o `clang`
- `make` (si hay Makefile)
- **zlib** (headers + runtime), p. ej. `zlib1g-dev` / `zlib-devel`
- Cross-compile AArch64 (si aplica):
  - `aarch64-linux-gnu-gcc` (o toolchain equivalente)
- QEMU (si vais a emular):
  - `qemu-user` / `qemu-aarch64` (o `qemu-system-aarch64` según setup)

### Cliente (Java)
- JDK **17+** (o la versión que uséis)
- Gradle o Maven (según el proyecto)

### Proxy / Emulación
- Proxy incluido en el repo **o**:
- `tc` + `netem` (Linux) para latencia/pérdida/limitación

### DOOM
- Necesitas un WAD compatible (por ejemplo `doom1.wad` shareware).  
  **No** se incluye en el repo.

---

## Cómo usar

> Los comandos exactos pueden variar según vuestra estructura de carpetas; la idea es que esto sea el “happy path”.


### 1) Build y ejecución del servidor (AArch64 cross-compile)
```bash
cd servidor/doomgeneric/doomgeneric

make CC=aarch64-linux-gnu-gcc   CFLAGS="--sysroot=$SYSROOT -isystem $SYSROOT/usr/include -isystem $SYSROOT/usr/include/aarch64-linux-gnu -isystem $SYSROOT/usr/include/SDL2 -ggdb3 -Os -Wall -DNORMALUNIX -DLINUX -DSNDSERV -D_DEFAULT_SOURCE"   LDFLAGS="--sysroot=$SYSROOT -Wl,--gc-sections"   LIBS="-L$SYSROOT/usr/lib/aarch64-linux-gnu -lSDL2 -lz -lm -lc"

qemu-aarch64 -L /opt/arm64-rootfs ./doomgeneric -iwad ../Doom1.WAD -warp 1 1
```
### 2) Ejecutar el cliente (Java)
Desde la terminal en el directorio del proyecto
```bash
.\gradlew build
.\gradlew run
```
### 3) Ejecutar el proxy (solo en caso de querer simularse condiciones de prueba)
```bash
pene
```

## Rendimiento esperado (orden de magnitud)

~4 KB/frame → con 500 kb/s (≈ 62.5 KB/s) el techo teórico es ~15 fps sin contar overhead/retransmisiones.

En la práctica: dependiendo de las condiciones reales, podría subirse o bajarse ligeramente.

## Limitaciones conocidas

UDP puede perder frames: preferimos fluidez sobre perfección.

Latencia variable: los controles van por TCP para evitar pérdidas, pero la interacción puede sentirse “pesada” si el RTT sube.

## Roadmap / ideas

Delta entre frames (solo cambios): no funciona y consume más ancho de banda, pero podría implementarse para partes concretas del frame (por ejemplo, el HUD apenas cambia)

“Keyframes” periódicos para recuperarse mejor de pérdidas.

Aceleración en FPGA del empaquetado 2+6 bits y/o preprocesado.

Telemetría (sensores) embebida en el stream.

## Sobre el desarrollo del proyecto

### Inspiration

DOOMSat nace a partir de una pregunta aparentemente absurda pero técnicamente muy seria, planteada en el Hackudc2026:  
**¿es posible ejecutar un videojuego interactivo en un satélite real, con las limitaciones extremas de un enlace espacial?**

Inspirados de igual manera por problemas reales de ingeniería aeroespacial —downlinks extremadamente limitados, latencias no despreciables y pérdida de paquetes— decidimos abordar el problema desde un ángulo distinto: **convertir DOOM en una carga útil experimental**, usando el juego como banco de pruebas para técnicas de compresión, transmisión y tolerancia a fallos propias de sistemas espaciales.

El objetivo no era solo “hacer funcionar DOOM”, sino demostrar que, con un diseño cuidadoso, **es posible transmitir información visual interactiva a través de un enlace ridículamente estrecho**, como los que se encuentran en microsatélites académicos.

---

### What it does

DOOMSat ejecuta el motor de DOOM en una plataforma **AArch64 compatible con entornos space-grade**, y permite jugarlo desde tierra en tiempo casi real.

Para ello:
- El **servidor (satélite)** captura los frames del motor de DOOM, los **reduce, cuantiza y comprime agresivamente**, y los envía a tierra a través de un **downlink UDP**.
- El **cliente en tierra** recibe los frames, los reconstruye y los renderiza, enviando los **controles del jugador por un uplink TCP fiable**.

El sistema está diseñado para funcionar bajo condiciones realistas de órbita baja (LEO), aceptando pérdidas de paquetes, latencia y ancho de banda muy limitado, sin que el juego deje de ser jugable.

Más allá del juego, DOOMSat demuestra un enfoque generalizable para:
- transmisión de vídeo científico,
- teleoperación,
- visualización remota,
- y experimentación con enlaces espaciales degradados.

---

### How it works (high level)

Internamente, DOOMSat aplica un pipeline optimizado para minimizar cada bit transmitido:

1. **Reducción de resolución** del frame original.  
2. **Cuantización por píxel**:
   - 2 bits de intensidad (luma).
   - 6 bits de color (crominancia simplificada).
3. **Compresión sin pérdidas con zlib** sobre el buffer cuantizado.  
4. **Envío por UDP**, priorizando frescura del frame frente a fiabilidad.  
5. **Controles por TCP**, garantizando orden y entrega.

El resultado típico es un frame de aproximadamente **4 KB**, lo que permite alcanzar del orden de **15–16 FPS** bajo un downlink de ~500 kbps, una cifra realista para microsatélites académicos.

---

### Challenges we ran into

El principal reto del proyecto fue **pensar como ingenieros espaciales**, no como desarrolladores de software convencional.

Algunos de los desafíos más relevantes fueron:

- **Downlink extremadamente limitado**, que obligó a exprimir cada bit del frame.
- **Elección de protocolos**: pérdida aceptable en vídeo (UDP), fiabilidad obligatoria en controles (TCP).
- **Latencia y jitter**, manteniendo jugabilidad incluso con frames perdidos.
- **Arquitectura heterogénea**: servidor en C (AArch64) y cliente en Java.
- **Cross-compiling y emulación** mediante QEMU para validar ejecución ARM.

Además, fue necesario definir **suposiciones realistas de misión** (RTT, pérdida, ancho de banda) y construir un **proxy/emulador de enlace** para pruebas controladas.

---

### Accomplishments that we’re proud of

Estamos especialmente orgullosos de haber logrado:

- Un sistema **jugable** bajo restricciones espaciales severas.
- Un pipeline de vídeo **simple pero muy eficiente** (~4 KB por frame).
- Una arquitectura clara y extensible.
- Un proyecto técnicamente honesto, basado en supuestos plausibles para LEO.

DOOMSat demuestra cómo un problema lúdico puede convertirse en una **demostración seria de ingeniería de sistemas**.

---

### What we learned

El desarrollo de DOOMSat nos permitió aprender sobre:

- Transmisión de datos en enlaces degradados.
- Compresión, cuantización y trade-offs calidad/ancho de banda.
- Programación de bajo nivel en C y comunicación por sockets.
- Cross-compiling y emulación de arquitecturas no convencionales.
- Diseño de sistemas tolerantes a fallos.

También reforzamos habilidades transversales como planificación, priorización de requisitos y toma de decisiones técnicas bajo restricciones duras.

---

### What’s next for DOOMSat

Aunque el sistema es funcional, existen múltiples líneas claras de mejora:

- Segmentación de frames a nivel de aplicación para evitar fragmentación IP.
- **Keyframes periódicos** para mejorar la recuperación tras pérdidas.
- Compresión diferencial parcial (por ejemplo, HUD vs escena).
- Aceleración por **FPGA** del empaquetado 2+6 bits y/o preprocesado.
- Inserción de **telemetría** embebida en el stream.
- Parametrización dinámica del enlace desde la propia aplicación.

DOOMSat se concibe como una **plataforma experimental abierta**, no como un producto cerrado.
