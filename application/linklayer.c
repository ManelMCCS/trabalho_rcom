#include "linklayer.h"

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters) {
    char serialPort[50];
    strcpy(serialPort, connectionParameters.serialPort);  // Dispositivo /dev/ttySx, x = 0, 1*/
    int role = connectionParameters.role; //defines the role of the program: 0==Transmitter, 1=Receiver
    int baudRate = connectionParameters.baudRate; // Número de sequência da trama:0, 1
    int numTries = connectionParameters.numTries;
    int timeOut = connectionParameters.timeOut;
    struct termios oldtio,newtio;

    int fd = open(serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    if(role == TRANSMITTER) {
        
    }
    
    return 1;
}
// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize) {
    return 1;
}
// Receive data in packet
int llread(char* packet) {
    return 1;
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics) {
    return 1;
}