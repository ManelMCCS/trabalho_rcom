/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "linklayer.h"
#include "funcoes.h"

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//character value definitions

#define MAX_SIZE 256

int fd;

linkLayer linkLayerSettings;
struct termios oldtio, newtio;

//struct linkLayer
//{
//  char port[20];                 /*Dispositivo /dev/ttySx, x = 0, 1*/
//  int baudRate;                  /*Velocidade de transmissão*/
//  unsigned int sequenceNumber = 0;   /*Número de sequência da trama:0, 1*/
//  unsigned int timeout = 3;          /*Valor do temporizador: 1 s*/
//  unsigned int numTransmissions = 3; /*Número de tentativas em caso de falha*/
//  char frame[MAX_SIZE];          /*Trama*/
//}

volatile int STOP = FALSE;

void atende()
{
    alarmeFlag = 1;
    printf("alarme\n");
    alarmeCount++;
}

int llopen(linkLayer connectionParameters) {
    //int res;
    struct termios oldtio, newtio;
    //char buf[255];
    int sum = 0, speed = 0;

    strcpy(linkLayerSettings.serialPort, connectionParameters.serialPort);
    linkLayerSettings.role = connectionParameters.role;
    if (connectionParameters.baudRate)
        linkLayerSettings.baudRate = connectionParameters.baudRate;
    else
        linkLayerSettings.baudRate = BAUDRATE_DEFAULT;
    if (connectionParameters.numTries)
        linkLayerSettings.numTries = connectionParameters.numTries;
    else
        linkLayerSettings.numTries = MAX_RETRANSMISSIONS_DEFAULT;
    if (connectionParameters.timeOut)
        linkLayerSettings.timeOut = connectionParameters.timeOut;
    else
        linkLayerSettings.timeOut = TIMEOUT_DEFAULT;

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */

    fd = open(linkLayerSettings.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(linkLayerSettings.serialPort);
        return -1;
    }

    if (tcgetattr(fd, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = linkLayerSettings.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    printf("New termios structure set\n");

    (void)signal(SIGALRM, atende);

    if (linkLayerSettings.role == 0)
    {
        alarmeCount = 0;
        alarmeFlag = 0;
        sendSET();
        tcflush(fd, TCIOFLUSH);
        alarm(linkLayerSettings.timeOut);
        char buffer[255] = {};
        receiveUA(buffer, linkLayerSettings.numTries, linkLayerSettings.timeOut);
        tcflush(fd, TCIOFLUSH);

        if (buffer[0] == 0 || buffer[3] != buffer[1] ^ buffer[2])
        {
            printf("Received: %x\t%x\t%x\t%x\t%x\n\n\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
            return -1;
        }
        else
        {
            printf("Received: %x\t%x\t%x\t%x\t%x\n\n\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
            return 1;
        }
    }
    else if (linkLayerSettings.role == 1)
    {
        receiveSET();
        tcflush(fd, TCIOFLUSH);
        sendUA();
        tcflush(fd, TCIOFLUSH);
    }
}

int llclose(int showStatistics)
{
    if (linkLayerSettings.role == 0)
    {
        alarmeCount = 0;
        alarmeFlag = 0;
        sendDISC(fd);
        tcflush(fd, TCIOFLUSH);
        alarm(linkLayerSettings.timeOut);

        char buffer[255] = {};

        receiveDISC(buffer, linkLayerSettings.numTries, linkLayerSettings.timeOut);
        tcflush(fd, TCIOFLUSH);
        printf("Received: %x\t%x\t%x\t%x\t%x\n\n\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

        sendUA(fd);
        tcflush(fd, TCIOFLUSH);
    }
    else if (linkLayerSettings.role == 1)
    {
        receiveDisc();
        tcflush(fd, TCIOFLUSH);
        sendDISC();
        tcflush(fd, TCIOFLUSH);
        receiveUa();
        tcflush(fd, TCIOFLUSH);
    }

    if(showStatistics){
        //todo
    }

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }

    sleep(1);
    close(fd);
    return 1;
}

int llwrite(char* buf, int bufSize)
{/*
    char sbuf[256];

    sbuf[0] = (char)Flag;
    sbuf[1] = (char)ACommands;
    sbuf[2] = 0b00000000 | (1 << 1);
    sbuf[3] = sbuf[1] & sbuf[2];

    int i = 0;
    for (i; i < bufSize; i++)
    {
        sbuf[i + 4] = buf[i];
        sbuf[bufSize + 4] = sbuf[length + 4] & buffer[i];
    }
    sbuf[length + 5] = (char)Flag;

    int res = write(fd, sbuf, len);

    //printf("String inputted: %s\n", sbuf);
    printf("%d bytes written\n", res);

    return res;*/
}

int llread(char* packet){
    int j=0,r=0,aux=0;
    char c1,buffer[2];
    short state = 0;
    

  while (state < 5)
  {
    r = read(fd, &c1, 1);
    switch (state)
    {
    case 0:
      if (c1 == (char)Flag)
      {
        state = 1;
      }
      else
        state = 0;
      break;
    case 1:
      if (c1 == (char)Flag)
        state = 1;
      else if (c1 == (char)ACommands)
      {
        state = 2;
        buffer[0]=c1;
      }
      else
        state = 0;
      break;
    case 2:
      if (c1 == (char)Flag)
        state = 1;
      else if (c1 == (char)S0)
      {
        aux=0;
        state = 3;
      }
      else if(c1==(char)S1){
        aux=1;
        state=3;
        buffer[1]=c1;
      }
      else
        state = 0;
      break;
    case 3:
      if (c1 == (char)Flag)
        state = 1;
      else if (c1 == (buffer[0] & buffer[1]))
      {
        state = 4;
      }
      else
        senREJ(aux);
        state=100;
        break;
    case 4:
        packet[j]=c1;
        if(c1==(buffer[0] & buffer[1])) state=5;
        j++;
        break;
    case 5:
      if(c1==(char)Flag){
          sendRR(aux);
          state=100;
      }
      break;
    default:
      state = 0;
      break;
    }
  }

    if(state==100) return 0;
    return 1;
}