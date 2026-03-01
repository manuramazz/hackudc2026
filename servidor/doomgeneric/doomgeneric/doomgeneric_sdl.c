//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <zlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdbool.h>
#include <SDL.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_PAYLOAD 1200  //MTU para paquetes UDP
static uint32_t globalFrameId = 0;

struct PacketHeader {
    uint32_t frameId;
    uint16_t chunkIndex;
    uint16_t totalChunks;
} __attribute__((packed)); // Evitar padding

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture;

#define KEYQUEUE_SIZE 16

#ifndef HARD_IP
#define HARD_IP "10.64.151.154"
#endif

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static uint8_t* prevFrameData = NULL;

static boolean enviarFrames = false;

static uint64_t bytesSentTotal = 0;
static uint64_t lastPrintTime = 0;

// Variables para UDP y TCP
//char HARD_IP[INET_ADDRSTRLEN]; // IP del cliente para UDP
int udpSock;                  
struct sockaddr_in clientAddr; 
socklen_t clientAddrLen;  
int clientSock = -1;
int tcpSock = -1;


static void addKeyToQueue(int pressed, unsigned int keyCode){
  //unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | keyCode;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

/*
static void handleKeyInput(){
  SDL_Event e;
  while (SDL_PollEvent(&e)){
    if (e.type == SDL_QUIT){
      puts("Quit requested");
      atexit(SDL_Quit);
      exit(1);
    }
    if (e.type == SDL_KEYDOWN) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyPress:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(1, e.key.keysym.sym);
    } else if (e.type == SDL_KEYUP) {
      //KeySym sym = XKeycodeToKeysym(s_Display, e.xkey.keycode, 0);
      //printf("KeyRelease:%d sym:%d\n", e.xkey.keycode, sym);
      addKeyToQueue(0, e.key.keysym.sym);
    }
  }
}
*/

static void handleKeyInput(void)
{
    if (tcpSock < 0) return;

    // Buffer de ensamblado: TCP puede entregar 1, 2 o 3 bytes en lecturas distintas
    static uint8_t inbuf[3];
    static size_t have = 0;

    // Lee todo lo disponible sin bloquear, hasta completar mensajes de 3 bytes
    for (;;) {
        ssize_t n = recv(tcpSock, inbuf + have, 3 - have, MSG_DONTWAIT);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No hay más datos ahora mismo
                return;
            }
            perror("recv(tcpSock)");
            // Si quieres, podrías cerrar aquí, pero mejor solo loguear
            return;
        }
        bytesSentTotal += n;
        if (n == 0) {
            // Conexión cerrada por el peer
            printf("TCP cerrado por el cliente\n");
            close(tcpSock);
            tcpSock = -1;
            have = 0;
            return;
        }

        have += (size_t)n;

        // Si aún no tenemos un paquete completo, vuelve a intentar en el próximo frame
        if (have < 3) {
            return;
        }

        // Tenemos 3 bytes: [keycode][pressed][0]
        uint8_t keycode = inbuf[0];
        uint8_t pressed = inbuf[1];
        uint8_t term    = inbuf[2];

        // Reset para el siguiente mensaje
        have = 0;

        // Validación del paquete
        if (term != 0) {
            // Desincronización: descarta y sigue (o resync si quieres)
            printf("Paquete inválido: term=%u (esperado 0)\n", (unsigned)term);
            continue;
        }

        if (pressed == 0 || pressed == 1) {
            addKeyToQueue((int)pressed, (unsigned int)keycode);
            // Debug opcional:
            //printf("Input TCP: key=%u pressed=%u\n", keycode, pressed);
        } else {
            printf("Paquete inválido: pressed=%u\n", (unsigned)pressed);
        }

        // Sigue el bucle por si llegaron más bytes (varios paquetes juntos)
    }
}

size_t compressFrameASCII(uint32_t* frameData, uint8_t** compressedData)
{
    int width = DOOMGENERIC_RESX;
    int height = DOOMGENERIC_RESY;

    // Cada pixel: 1 byte combinación intensidad + color
    *compressedData = malloc(width * height);
    if (*compressedData == NULL) { exit(1); }

    for (int i = 0; i < width * height; i++) {
        uint32_t pixel = frameData[i];

        // Separar RGB
        uint8_t r = (pixel >> 16) & 0xFF;
        uint8_t g = (pixel >> 8) & 0xFF;
        uint8_t b = pixel & 0xFF;
        uint8_t r4 = r >> 4;
        uint8_t g4 = g >> 4; 
        uint8_t b4 = b >> 4; 

        // Combina usando 4 bits
        uint8_t color = (r4 + g4 + b4) / 3;

        // Intensidad aproximada: 0-15 (4 bits)
        uint8_t intensity = (r + g + b) / 48;  // 0-15
        if (intensity > 15) intensity = 15;

        // Color: usa 4 bits para rojo+verde+azul de forma simplificada
        color = ((r & 0xF0) >> 4) | (g & 0xF0) | ((b & 0xF0) >> 4);

        uint8_t byte = (intensity << 4) | (color & 0x0F);
        (*compressedData)[i] = byte;       // Intensidad + color
    }

    return height * width;
}

size_t compressFrameASCII_2x2(uint32_t* frameData, uint8_t** compressedData)
{
    int width = DOOMGENERIC_RESX;
    int height = DOOMGENERIC_RESY;
    int newWidth = width / 2;
    int newHeight = height / 2;

    *compressedData = malloc(newWidth * newHeight);
    if (*compressedData == NULL) { exit(1); }

    int outIndex = 0;

    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 2) {
            uint32_t sumR = 0, sumG = 0, sumB = 0;

            for (int dy = 0; dy < 2; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    uint32_t p = frameData[(y+dy)*width + (x+dx)];
                    sumR += (p >> 16) & 0xFF;
                    sumG += (p >> 8) & 0xFF;
                    sumB += p & 0xFF;
                }
            }

            // Promedios promediados (div 4)
            uint8_t r = sumR >> 2;
            uint8_t g = sumG >> 2;
            uint8_t b = sumB >> 2;

            // 1. LUMINANCIA PERCIBIDA (Más brillante para el ojo)
            uint32_t lum = (r * 299 + g * 587 + b * 114) / 1000;
            
            // 2. ACLARADO EXTRA (Gamma boost manual)
            // Esto "empuja" los colores oscuros hacia arriba
            if (lum < 128) lum += (128 - lum) / 2; 

            // 3. REDUCCIÓN CON REDONDEO (En lugar de truncar con >> 6)
            uint8_t i2 = (lum + 31) / 64; if (i2 > 3) i2 = 3;
            uint8_t r2 = (r + 31) / 64;   if (r2 > 3) r2 = 3;
            uint8_t g2 = (g + 31) / 64;   if (g2 > 3) g2 = 3;
            uint8_t b2 = (b + 31) / 64;   if (b2 > 3) b2 = 3;

            (*compressedData)[outIndex++] = (i2 << 6) | (r2 << 4) | (g2 << 2) | b2;
        }
    }
    return newWidth * newHeight;
}

size_t deltaDataFunc(uint8_t* compressedData, uint8_t** deltaOut) {
  size_t deltaSize = 0;
  uint8_t* delta = malloc(sizeof(uint8_t) * DOOMGENERIC_RESX * DOOMGENERIC_RESY*3);
  if (!delta) exit(1);

  for (size_t i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++) {
        if (compressedData[i] != prevFrameData[i]) {
            uint16_t idx = (uint16_t)i;              // si frameSize > 65535, usa uint32_t
            delta[deltaSize++] = idx & 0xFF;        // byte bajo del índice
            delta[deltaSize++] = (idx >> 8) & 0xFF;
            delta[deltaSize++] = compressedData[i];
      }
      if(i<10) {
        printf("Pixel %d: current=%02X prev=%02X\n", i, compressedData[i], prevFrameData[i]);
      }
  }
  
  *deltaOut = delta;
  printf("Delta size: %zu bytes\n", deltaSize);
  if(deltaSize > 0) {
    printf("DELTA MODIFICADO ejemplo: índice %d valor %02X\n", delta[0], delta[1]);
  } 
  return deltaSize;
}

size_t compressRLE(uint8_t* data, size_t dataSize, uint8_t** out) {
    *out = malloc(dataSize * 2);
    if (!*out) exit(1);

    size_t outIndex = 0;
    for (uint32_t i = 0; i < dataSize;) {
        uint8_t value = data[i];
        size_t runLength = 1;

        while (i + runLength < dataSize && data[i + runLength] == value && runLength < 255) {
            runLength++;
        }

        (*out)[outIndex++] = value;
        (*out)[outIndex++] = (uint8_t)runLength;

        i += runLength;
    }

    return outIndex; // tamaño final comprimido
}

size_t compressFrame(uint32_t* frameData, uint8_t** compressedData) {
    //printf("Comprimiendo frame... DG_ScreenBuffer=%p\n", (void*)frameData);

    // Comprimir a ASCII 2x2
    uint8_t* asciiData = NULL;
    size_t asciiSize = compressFrameASCII_2x2(frameData, &asciiData);
    //printf("Frame comprimido a ASCII de %zu bytes\n", asciiSize);

    // Comprimir con zlib
    uLongf zlibMaxSize = compressBound(asciiSize);
    uint8_t* zlibBuffer = malloc(zlibMaxSize);
    if (!zlibBuffer) { free(asciiData); exit(1); }

    int res = compress2(zlibBuffer, &zlibMaxSize, asciiData, asciiSize, 8);
    if (res != Z_OK) {
        fprintf(stderr, "Error comprimiendo frame con zlib: %d\n", res);
        free(asciiData);
        free(zlibBuffer);
        exit(1);
    }

    free(asciiData);
    *compressedData = zlibBuffer;
    //printf("Frame comprimido con zlib a %lu bytes\n", zlibMaxSize);

    return (size_t)zlibMaxSize;
}

int initTCP(int port) {
    struct sockaddr_in serverAddr;

    // 1. Crear el socket
    tcpSock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSock < 0) { 
        perror("Error creando socket TCP"); 
        exit(1); 
    }

    // 2. Configurar la dirección del servidor (Java)
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // Convertir la IP de texto a binario
    if (inet_pton(AF_INET, HARD_IP, &serverAddr.sin_addr) <= 0) {
        fprintf(stderr, "Dirección IP inválida: %s\n", HARD_IP);
        return -1;
    }

    printf("Intentando conectar al servidor Java en %s:%d...\n", HARD_IP, port);

    // 3. Connect (Bloqueante hasta que conecte o falle)
    if (connect(tcpSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error en connect TCP (¿Está Java encendido?)");
        close(tcpSock);
        tcpSock = -1;
        return -1;
    }

    // 4. Modo No Bloqueante
    int flags = fcntl(tcpSock, F_GETFL, 0);
    fcntl(tcpSock, F_SETFL, flags | O_NONBLOCK);

    // En el modo cliente, el socket que usas para enviar es el mismo tcpSock
    clientSock = tcpSock; 

    printf("¡Conectado exitosamente al servidor Java!\n");
    return 0;
}

void initUDP() {
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) { perror("socket"); exit(1); }

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(5001); // puerto del cliente
    inet_pton(AF_INET, HARD_IP, &clientAddr.sin_addr);
    printf("Iniciado servicio UDP en puerto 5001 con cliente %s...\n", HARD_IP);
    clientAddrLen = sizeof(clientAddr);
}


void sendFrameUDP(uint8_t* data, size_t dataSize) {
    
    ssize_t sentBytes = sendto(udpSock, data, dataSize, 0,
                               (struct sockaddr*)&clientAddr, clientAddrLen);
    if (sentBytes < 0) {
        perror("Error enviando frame UDP");
    } else if ((size_t)sentBytes != dataSize) {
        fprintf(stderr, "Advertencia: se enviaron %zd de %zu bytes\n", sentBytes, dataSize);
    }else{
      // Printear uno de cada 10 frames
       static int frameCount = 0;
       frameCount++;
        //printf("Frame de %zu bytes enviado por UDP\n", sentBytes);
        if(frameCount == 0) {
            
        }
    }
}


void sendFrameUDPFragmentado(uint8_t* data, size_t dataSize) {
    // Calcular número de paquetes
    uint16_t totalChunks = (dataSize + MAX_PAYLOAD - 1) / MAX_PAYLOAD;
    //printf("Enviando frame %u en %u chunks, total bytes=%zu\n", globalFrameId, totalChunks, dataSize);

    for (uint16_t chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
        size_t offset = chunkIndex * MAX_PAYLOAD;
        size_t chunkSize = (dataSize - offset > MAX_PAYLOAD) ? MAX_PAYLOAD : (dataSize - offset);

        // Crear buffer con header + payload
        uint8_t* packet = malloc(sizeof(struct PacketHeader) + chunkSize);
        if (!packet) { perror("malloc packet"); exit(1); }

        struct PacketHeader* hdr = (struct PacketHeader*)packet;
        hdr->frameId = htonl(globalFrameId);
        hdr->chunkIndex = htons(chunkIndex);
        hdr->totalChunks = htons(totalChunks);

        memcpy(packet + sizeof(struct PacketHeader), data + offset, chunkSize);

        ssize_t sentBytes = sendto(udpSock, packet, sizeof(struct PacketHeader) + chunkSize, 0,
                                   (struct sockaddr*)&clientAddr, clientAddrLen);
        if (sentBytes < 0) {
            perror("Error enviando chunk UDP");
        } else if ((size_t)sentBytes != sizeof(struct PacketHeader) + chunkSize) {
            fprintf(stderr, "Advertencia: se enviaron %zd de %zu bytes\n", sentBytes, sizeof(struct PacketHeader) + chunkSize);
        } else {
            //printf("Chunk %u/%u enviado, tamaño %zu bytes\n", chunkIndex, totalChunks, sizeof(struct PacketHeader) + chunkSize);
            bytesSentTotal += sizeof(struct PacketHeader) + chunkSize;
            //printf("bytes enviados: %u", bytesSentTotal);
        }

        free(packet);
    }

    // Incrementar frameId para el siguiente frame
    globalFrameId++;
}



void DG_Init(){
  
  //Reserva de memoria para el buffer de pantalla
  // Solo reservar memoria para el buffer de pantalla
    DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * sizeof(uint32_t));
    printf("Resolución: %dx%d\n", DOOMGENERIC_RESX, DOOMGENERIC_RESY);
    if (DG_ScreenBuffer == NULL) {
        fprintf(stderr, "Error reservando memoria para DG_ScreenBuffer\n");
        exit(1);
    }else {
    //("DG_ScreenBuffer=%p\n", (void*)DG_ScreenBuffer);
}
}

size_t compressFrameDefault(uint32_t* frameData, uint8_t** compressedData) {
    *compressedData = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY / 2000);
    if (*compressedData == NULL) { exit(1); }
    //Copiar el frame sin comprimir (solo para pruebas)
    for (long int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY / 2000; i++) {
        (*compressedData)[i] = (uint8_t)(frameData[i] & 0xFF); // Solo el canal azul como ejemplo
    }
    return DOOMGENERIC_RESX * DOOMGENERIC_RESY / 2000;
}

static boolean udpIniciado = false;
static boolean tcpIniciado = false;
void DG_DrawFrame()
{
  if (!enviarFrames) {
    return; // No enviar frames hasta el primer tick
  }
  if(!tcpIniciado) {
    initTCP(5555);
    tcpIniciado = true;
  }
  if(!udpIniciado) {
    initUDP();
    udpIniciado = true;
  }
  
  // Comprimir frame
  uint8_t* compressedFrame;
  size_t size = compressFrame(DG_ScreenBuffer, &compressedFrame);
  //size_t size = compressFrameDefault(DG_ScreenBuffer, &compressedFrame);
  //printf("Frame comprimido a %zu bytes\n", size);
  //printf("\n");
  // Enviar por UDP al cliente
  sendFrameUDPFragmentado(compressedFrame, size);

  free(compressedFrame);

  // Procesar inputs que llegan del cliente
  handleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
  SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
  return SDL_GetTicks();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}

void DG_SetWindowTitle(const char * title)
{
  if (window != NULL){
    SDL_SetWindowTitle(window, title);
  }
}

int main(int argc, char **argv)
{
    if(!tcpIniciado) {
        initTCP(5555);
        tcpIniciado=true;
    }
    if(!udpIniciado) {
        initUDP();
    }
    
    doomgeneric_Create(argc, argv);
    uint8_t FPS = 15;
    for (int i = 0; ; i++)
    {
        uint32_t frameStartTime = DG_GetTicksMs();
        enviarFrames = true;
        doomgeneric_Tick();

        uint32_t frameTime = DG_GetTicksMs() - frameStartTime;
        uint32_t sleepTime = (1000 / FPS) - frameTime;
        if (sleepTime > 0) {
            DG_SleepMs(sleepTime);
        }
        uint32_t now = DG_GetTicksMs();

        if (now - lastPrintTime >= 1000) {
            double mbps = (bytesSentTotal * 8.0) / 1000.0;
            //printf("sexo: %.2f\n", bytesSentTotal);
            printf("Bandwidth: %.2f Kbps\n", mbps);
            bytesSentTotal = 0;
            lastPrintTime = now;
        }
        //printf("Tick %d: DG_ScreenBuffer=%p\n espera=%d ms\n", i, (void*)DG_ScreenBuffer, sleepTime);
    }
    

    return 0;
}