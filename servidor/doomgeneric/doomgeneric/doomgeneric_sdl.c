//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <SDL.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(unsigned int key){
  switch (key)
    {
    case SDLK_RETURN:
      key = KEY_ENTER;
      break;
    case SDLK_ESCAPE:
      key = KEY_ESCAPE;
      break;
    case SDLK_LEFT:
      key = KEY_LEFTARROW;
      break;
    case SDLK_RIGHT:
      key = KEY_RIGHTARROW;
      break;
    case SDLK_UP:
      key = KEY_UPARROW;
      break;
    case SDLK_DOWN:
      key = KEY_DOWNARROW;
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      key = KEY_FIRE;
      break;
    case SDLK_SPACE:
      key = KEY_USE;
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      key = KEY_RSHIFT;
      break;
    case SDLK_LALT:
    case SDLK_RALT:
      key = KEY_LALT;
      break;
    case SDLK_F2:
      key = KEY_F2;
      break;
    case SDLK_F3:
      key = KEY_F3;
      break;
    case SDLK_F4:
      key = KEY_F4;
      break;
    case SDLK_F5:
      key = KEY_F5;
      break;
    case SDLK_F6:
      key = KEY_F6;
      break;
    case SDLK_F7:
      key = KEY_F7;
      break;
    case SDLK_F8:
      key = KEY_F8;
      break;
    case SDLK_F9:
      key = KEY_F9;
      break;
    case SDLK_F10:
      key = KEY_F10;
      break;
    case SDLK_F11:
      key = KEY_F11;
      break;
    case SDLK_EQUALS:
    case SDLK_PLUS:
      key = KEY_EQUALS;
      break;
    case SDLK_MINUS:
      key = KEY_MINUS;
      break;
    default:
      key = tolower(key);
      break;
    }

  return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode){
  unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

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

size_t compressFrame(uint32_t* frameData, uint8_t** compressedData) {
    // Implementa aquí tu algoritmo de compresión (por ejemplo, RLE)
    // Para este ejemplo, simplemente vamos a copiar los datos sin comprimir
    size_t dataSize = DOOMGENERIC_RESX * DOOMGENERIC_RESY * sizeof(uint32_t);
    *compressedData = malloc(dataSize);
    if (*compressedData == NULL) {
        fprintf(stderr, "Error reservando memoria para compressedData\n");
        exit(1);
    }
    memcpy(*compressedData, frameData, dataSize);
    return dataSize;
}

// Variables para UDP
extern int udpSock;                  
extern struct sockaddr_in clientAddr; 
extern socklen_t clientAddrLen;  
char ip_client[17];     

void initUDP() {
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) { perror("socket"); exit(1); }

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(5000); // puerto del cliente
    inet_pton(AF_INET, ip_client, &clientAddr.sin_addr);

    clientAddrLen = sizeof(clientAddr);
}

void sendFrameUDP(uint8_t* data, size_t dataSize) {
    ssize_t sentBytes = sendto(udpSock, data, dataSize, 0,
                               (struct sockaddr*)&clientAddr, clientAddrLen);
    if (sentBytes < 0) {
        perror("Error enviando frame UDP");
    } else if ((size_t)sentBytes != dataSize) {
        fprintf(stderr, "Advertencia: se enviaron %zd de %zu bytes\n", sentBytes, dataSize);
    }
}

void handleRemoteInput() {
    
}


void DG_Init(){
  
  //Reserva de memoria para el buffer de pantalla
  // Solo reservar memoria para el buffer de pantalla
    DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * sizeof(uint32_t));

    if (DG_ScreenBuffer == NULL) {
        fprintf(stderr, "Error reservando memoria para DG_ScreenBuffer\n");
        exit(1);
    }
}

void DG_DrawFrame()
{
  // Comprimir frame
  uint8_t* compressedFrame;
  size_t size = compressFrame(DG_ScreenBuffer, &compressedFrame);

  // Enviar por UDP al cliente
  sendFrameUDP(compressedFrame, size);

  free(compressedFrame);

  // Procesar inputs que llegan del cliente
  handleRemoteInput();
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
    doomgeneric_Create(argc, argv);
    initUDP();
    size32_t FPS = 10;
    for (int i = 0; ; i++)
    {
        size32_t frameStartTime = DG_GetTicksMs();

        doomgeneric_Tick();

        size32_t frameTime = DG_GetTicksMs() - frameStartTime;
        size32_t sleepTime = (1000 / FPS) - frameTime;
        if (sleepTime > 0) {
            DG_SleepMs(sleepTime);
        }
    }
    

    return 0;
}