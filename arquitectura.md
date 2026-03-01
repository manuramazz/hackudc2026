# Arquitectura de DOOMSat

## Visión General
DOOMSat consiste en la simulación del envío de datos de la Tierra a un satélite y viceversa: una aplicación cliente-servidor diseñada para correr el videojuego Doom de manera remota con un proxy intermediario. Este es ejecutado en un servidor, pero el input es enviado de manera remota por el cliente. Todo con un proxy intermediario, de manera que en la máquina proxy podamos simular una disminución del ancho de banda o pérdida de paquetes que sucedería en el caso que intentamos simular.

## Componentes Principales

### 1. Servidor (C)
- Servidor que corre, emulando Aarch64, el videojuego Doom
- Se le envía por TCP el input para el videojuego. 
- Envía, por UDP, los frames del juego, comprimidos.

### 2. Proxy (Java)
- Recoge las llamadas TCP del cliente y las redirecciona al servidor.
- Recoge las llamadas UDP del servidor y las redirecciona al cliente.
- Simula las fallas que supondría la conexión entre ambos alejados extremos.

### 3. Cliente (Java - JavaFX)
- Envía los inputs al servidor (Pasando por el proxy).
- Recibe los frames comprimidos.
- Descomprime los data packets recibidos.
- Crea, con JavaFX, una representación visual del frame.

## Flujos de Datos
- TCP Cliente -> Proxy (Input)
- Proxy -> TCP Servidor (Input)
- UDP Servidor -> Proxy (Frame)
- Proxy -> UDP Cliente (Frame)


