/*
    Resources:
    https://moodle.up.pt/pluginfile.php/118136/mod_resource/content/1/rcom.lecture3.data_link_layer.pdf
    https://stackoverflow.com/questions/52187/virtual-serial-port-for-linux
*/

#include "linklayer.h"
#include "stdlib.h"

#define DEBUG_MODE 1

#define FLAG 0x7e
#define A_T 0x03
#define A_R 0x01
#define C_T 0x03
#define DISC 0x0b
#define UA 0x07
#define BBC1_T A_T ^ C_T
#define BCC1_R A_T ^ UA

int fd = 0;
struct termios oldtio, newtio;
linkLayer parameters;

// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters)
{

    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* C_T input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; // inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // blocking read until 5 chars received

    /*
      VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
      leitura do(s) pr�ximo(s) caracter(es)
    */
    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcC_Tattr");
        return -1;
    }
    parameters.role = connectionParameters.role;

    return 1;
}

// Sends data in buf with size bufSize
int llwrite(char *buf, int bufSize)
{
    switch (parameters.role)
    {
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
    char *input = (char *)calloc(input_size, sizeof(char));

    // Creation of the header

    input[0] = FLAG;
    input[1] = A_T;
    input[2] = C_T;
    input[3] = BBC1_T;

    char BCC2 = 0; // Creation of BCC2
    for (size_t i = 0; i < bufSize; i++)
        BCC2 ^= buf[i];

    strcat(input, buf);
    input[input_size - 1] = BCC2;
    // End of creation of the header

    // Byte Stuffing
    size_t stuffed_size; // 4 primeiras casas; bbbb; faz stuf do bcc2; e poe flag
    char *stuffed = stuffing(input, input_size, &stuffed_size);

    // End of byte stuffing;

    // State machine
    unsigned int END_STATE = 2;
    unsigned int state = 0;
    int tries = 0;
    do
    {
        if (parameters.numTries < tries++)
        {
            perror("Exceeded the max number of tries");
            return -1;
        }
    } while (state != END_STATE);

    free(input);
    // free(stuffed);
    return 1;
}
// Receive data in packet
int llread(char *packet)
{
    switch (parameters.role)
    {
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
    // Creation of the header

    size_t output_size = 5;
    char *output = (char *)calloc(output_size, sizeof(char));

    output[0] = FLAG;
    output[1] = A_T;
    output[2] = UA;
    output[3] = BCC1_R;
    output[4] = FLAG;

    // Destuffing

    return 0;
}
// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics)
{
    if (showStatistics)
    {
        printf("Statistics: \n");
    }

    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return 1;
}
/*
    Not implemented
*/
int send_message()
{
    return FLAG;
}

char *stuffing(char *input, size_t input_size, size_t *output_size)
{
    size_t stuffed_size = 4;
    char *stuffed = (char *)calloc(MAX_PAYLOAD_SIZE << 1, sizeof(char));
    for (size_t i = 0; i < stuffed_size; i++)
    {
        stuffed[i] = input[i];
    }
    PRINT_V(input, input_size);
    input_size--;

    for (size_t i = stuffed_size; i < input_size; i++)
    {
        if (DEBUG_MODE)
            printf("input: %x ->", input[i]);
        switch (input[i])
        {
        case 0x7e:
            if (DEBUG_MODE)
                printf("Caso 1 de byte stuffing \n");
            stuffed[stuffed_size++] = 0x7d; // pos atual
            stuffed[stuffed_size++] = 0x5e; // pos seguinte
            break;
        case 0x7d:
            if (DEBUG_MODE)
                printf("Caso 2 de byte stuffing \n");
            stuffed[stuffed_size++] = 0x7d;
            stuffed[stuffed_size++] = 0x5d;
            break;
        default:
            if (DEBUG_MODE)
                printf("stuffed = input \n");
            stuffed[stuffed_size++] = input[i];
            break;
        }
    }

    stuffed[stuffed_size] = stuffed[0];

    *output_size = ++stuffed_size;
    stuffed = (char *)realloc(stuffed, stuffed_size);
    return stuffed;
}
char *destuffing(char *packet, size_t packet_size, size_t *output_size)
{

    size_t destuffed_size = 4;
    char *destuffed = (char *)calloc(packet_size, sizeof(char));
    for (size_t i = 0; i < destuffed_size; i++)
    {
        destuffed[i] = packet[i];
    }

    packet_size--;
    for (size_t i = destuffed_size; i < packet_size; i++)
    {
        if (DEBUG_MODE)
            printf("packet[%i]: %x, packet[%i]: %x ->", (int)i, packet[i], (int)(i + 1), packet[i + 1]);
        if (packet[i] == 0x7d)
        {
            switch (packet[i + 1])
            {
            case 0x5e:
                if (DEBUG_MODE)
                    printf("Caso 1 de byte destuffing \n");
                destuffed[destuffed_size] = 0x7e;
                i++;
                break;
            case 0x5d:
                if (DEBUG_MODE)
                    printf("Caso 2 de byte destuffing \n");
                destuffed[destuffed_size] = 0x7d;
                i++;
                break;
            default:
                if (DEBUG_MODE)
                    printf("stuffed = input \n");
                destuffed[destuffed_size] = packet[i];
                break;
            }
        }
        else
        {
            if (DEBUG_MODE)
                printf("stuffed = input \n");
            destuffed[destuffed_size] = packet[i];
        }
        destuffed_size++;
    }
    if (DEBUG_MODE)
        printf("\n");

    destuffed[destuffed_size++] = destuffed[0];

    *output_size = destuffed_size;
    destuffed = (char *)realloc(destuffed, destuffed_size);

    return destuffed;
}