# Contribuir a DOOMSat

¡Gracias por tu interés en contribuir a DOOMSat! Este proyecto de código abierto busca optimizar la comunicación con un satélite. Si bien en la realidad no se realiza (no disponemos de un satélite), modificamos el entorno para simular lo más fielmente posible una comunicación con uno.

## 🛠️ Requisitos previos
Antes de contribuir, asegúrate de tener instalados:

- **Java 17 o superior** con Gradle (`./gradlew build`).
- **Qemu people con aarch64**
- **Conocimientos en C y, opcionalmente, en JavaFX**.

## 📌 Cómo contribuir

### 1️⃣ Forkea el repositorio
Haz un fork del proyecto en GitHub y clona tu copia localmente:

```sh
git clone https://github.com/JorgeOteroPailos/hackaton
cd competencIApp
```
### 2️⃣ Crea una rama para tu contribución
Utiliza un nombre descriptivo para tu rama:

```sh
Copy
Edit
git checkout -b feature-nombre-mejora
```
### 3️⃣ Realiza los cambios y asegúrate de que el código funcione
Antes de hacer un commit, asegúrate de que la aplicación sigue funcionando y de que tus cambios no rompen ninguna funcionalidad:

sh
Copy
Edit
# Prueba el backend
- `gcc` o `clang`
- `make` (si hay Makefile)
- **zlib** (headers + runtime), p. ej. `zlib1g-dev` / `zlib-devel`
- Cross-compile AArch64 (si aplica):
  - `aarch64-linux-gnu-gcc` (o toolchain equivalente)
- QEMU (si vais a emular):
  - `qemu-user` / `qemu-aarch64` (o `qemu-system-aarch64` según setup)


# Prueba la interfaz en JavaFX
./gradlew run
### 4️⃣ Sigue las normas de estilo
Java: Usa convenciones estándar y formatea el código con ./gradlew format.
Commits: Usa mensajes claros y estructurados, por ejemplo:
sh
Copy
Edit
git commit -m "feat: agregada funcionalidad de consulta de competencias"
### 5️⃣ Envía un Pull Request (PR)
Sube tus cambios a tu repositorio y crea un PR hacia la rama main:

sh
Copy
Edit
git push origin feature-nombre-mejora
En la descripción del PR, explica los cambios realizados y menciona si hay problemas o puntos a mejorar.
