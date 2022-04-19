/*
    Resources:
    https://moodle.up.pt/pluginfile.php/118136/mod_resource/content/1/rcom.lecture3.data_link_layer.pdf
    https://stackoverflow.com/questions/52187/virtual-serial-port-for-linux
*/

#include "linklayer.h"
#include "stdlib.h"

#define FLAG 0x7e
#define A_T 0x03
#define A_R 0x01
#define SET 0x03
#define DISC 0x0b
#define UA 0x07
#define BBC1_T A_T ^ SET
#define BCC1_R A_T ^ UA

int fd = 0;
struct termios oldtio,newtio;

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters) {

    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {
        perror(connectionParameters.serialPort); 
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;         // inter-character timer unused
    newtio.c_cc[VMIN] = 5;          // blocking read until 5 chars received

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        return -1;
    }
    
    return 1;
}

// Sends data in buf with size bufSize
int llwrite(char* buf, int bufSize, linkLayer *connectionParameters) {
    switch (connectionParameters->role) {
        case TRANSMITTER:
            printf("Reciever in llread\n");
            break;
        case RECEIVER:
            perror("Error: Receiver in llwrite");
            return -1;
        default:
            perror("Error: Undifined role in llwrite");
            return -1;
    }
    size_t input_size = bufSize + 5;
    char *input = (char*)calloc(input_size, sizeof(char));

    input[0] = FLAG;
    input[1] = A_T;
    input[2] = SET;
    input[3] = BBC1_T;
    
    char BCC2 = 0;                                          // Creation of BCC2
    for (size_t i = 0; i < bufSize; i++) BCC2 ^= buf[i];

    strcat(input, buf);
    input[input_size - 1] = BCC2; 

    // Byte Stuffing    
    size_t stuffed_size = MAX_PAYLOAD_SIZE << 1;
    char *stuffed = (char*)calloc(stuffed_size, sizeof(char));

    size_t j = 0;
    for ( size_t i = 1; i < input_size; i++ ) {
        if ((input[i] == 0x7e) || (input[i] == 0x7d)) {
            stuffed[i + j++] = 0x7d;
            stuffed[i + j] = input[i] ^ 0x20;
        }
        else stuffed[i + j] = input[i];
    }
    stuffed[0] = FLAG;
    stuffed[i + j] = FLAG;
    
    stuffed_size = i + j + 1;
    stuffed = (char*)realloc(stuffed, stuffed_size);

    // State machine
    unsigned int END_STATE = 2;
    unsigned int state = 0;
    int tries = 0;
    do {
        if(connectionParameters->numTries < tries++) {
            perror("Exceeded the max number of tries");
            return -1;
        }
    } while(state != END_STATE);
  
    free(input);
    free(stuffed);
    return 1;
}
// Receive data in packet
int llread(char* packet) {
    switch (connectionParameters->role) {
        case RECEIVER:
            printf("Reciever in llread\n");
            break;
        case TRANSMITTER:
            perror("Error: Receiver in llread");
            return -1;
        default:
            perror("Error: Undifined role in llwrite");
            return -1;
    }
    size_t output_size = 5;
    char *output = (char*)calloc(output_size, sizeof(char));
    
    output[0] = FLAG;
    output[1] = A_T;
    output[2] = UA;
    output[3] = BCC1_R;
    output[4] = FLAG;
    
    return 0;
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics) {
    if(showStatistics) {
        printf("Statistics: \n");
    }
    
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 1;
}
/*
    Not implemented
*/
int send_message() {
    return FLAG;
}