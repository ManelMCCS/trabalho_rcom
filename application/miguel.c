#include "linklayer.h"

// Define Flags, Header and Trailer
#define FLAG 0x7e
#define A_T 0x03
#define A_R 0x01
#define SET 0x03
#define DISC 0x0b
#define UA 0x07
#define BBC1_T A_T ^ SET // Será usado o mesmo Address no llopen, tanto para Transmitter como Receiver
#define BCC1_R A_T ^ UA

#define HEADER_SIZE 4
#define SUP_SIZE 5

/***********************
 * Struct Linklayer received values:
 * ll.serialPort="/dev/ttySx"
 * ll.role=0 se tx, 1 se rx
 * ll.baudRate=9600
 * ll.numTries=3
 * ll.timeOut=3
 * ****************************/

// needs to be global variable, but honestly tenho de ver outra maneira que a stora disse que dava

struct statistacs{
  int DataWritten;
};

struct termios oldtio, newtio;
linkLayer ll;
int fd;
int state = 0;
int S;
int R;

void escrita(){
  printf("TIMEDOUT maninho\n");
  state=0;
}

//control and expected are very well defined already by the asking fucntion
//works only for answers
//returns control variable
char wait_for_answer(){

  char input[HEADER_SIZE+1];
  int res=0;

  (void)signal(SIGALRM, escrita);

	if(!state){
	state=1;
	alarm(0);
	alarm(ll.timeOut);
	}

	while (state <3)
	{
	  switch (state)
	  {

	  case 0:
		return 0;

	  // receber informaçao
	  case 1:
		printf("À espera da resposta\n");
		while ((!read(fd, &input[0], 1)) && state)
		{
		}
		if (input[0] == FLAG)
		{
			printf("Recebemos primeira FLAG\n");
		  state = 2;
		  alarm(0); // paramos o timer, porque de facto temos uma receçao de dados
		}
		else
		{
		  state = 0;
		}

		break;

	  case 2:
		while (!res){
			res=read(fd, &input[1], 3);
			printf("%d bytes Read\n", res);
		}   //might not work por causa do endereço, ams hard doubt
		printf("1->%d\n2->%d\n3->%d\n", input[1],input[2],input[3]);
		if(input[3]!=(input[1]^input[2])){
		  printf("ERRO, bcc1 diferente, retransmite\n");
		  state=0;
		  break;
		}
		printf("BCC1 is good\n");
		state++;
		break;
		
	  }
	}


	printf("Recebeste um Header quite successfully\n");
	return input[2];
}


// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess
int llopen(linkLayer connectionParameters)
{
	char aux;


	sprintf(ll.serialPort, "%s", connectionParameters.serialPort);
	ll.role=connectionParameters.role;
	ll.baudRate=connectionParameters.baudRate;
  	ll.numTries=connectionParameters.numTries;
	ll.timeOut=connectionParameters.timeOut;



  /*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
  */

fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    if (fd <0) 
    {
      perror(connectionParameters.serialPort); 
      exit(-1); 
    }

    if ( tcgetattr(fd,&oldtio) == -1) 
    { 
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;  
    newtio.c_cc[VMIN]     = 1;  

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) 
    {
      perror("tcsetattr");
      exit(-1);
    }

   printf("New termios structure set\n");
  // Comunicaçao aberta

  int k = 0, res = 0;
  char input[5];

  // activates timeOut interrupt

  switch (connectionParameters.role)

  {

  case TRANSMITTER:

	input[0] = FLAG;
	input[1] = A_T;
	input[2] = SET;
	input[3] = BBC1_T;
	input[4] = FLAG;


	while (state != 2)
	{
	
	  switch (state)
	  {

	  // escrever informaçao e verificar
	  case 0:
		printf("Estamos em state=0 pela %d vez\n", k);

		if (k == connectionParameters.numTries)
		{
		  printf("Nao recebeste dados nenhuns amigo, problems de envio\n");
		  return -1;
		}

		k++;
		res = write(fd, input, 5);
		printf("%d Bytes written\n", res);
		aux=wait_for_answer();
		if(aux==UA){
			printf("Recebemos UA llopen\n");
			state=1;
			break;
		}
		break;

	  // receber informaçao
		case 1:
			while(!read(fd, &aux, 1)){}

			if(aux!=FLAG){
				printf("Nao recebemos last FLAG, trying again\n");
				state=0;
				break;
			}

			state++;
			printf("llopen Transmitter done successfully (palmas)\n");
			return 1;

		break;
	  
	}

	}

	return 1;
	
	
//Ver return desta function quando ha erro, should I even count the tries??? 
//HELPPPPPPPPP
  case RECEIVER:

	input[0] = FLAG;
	input[1] = A_T;
	input[2] = UA;
	input[3] = BCC1_R;
	input[4] = FLAG;

	printf("A começar RECEIVER\n");

	// control field definition
	while (state != 2)
	{
	  // Receiver
	  switch (state)
	  {
		case 0:
			state=1;
			aux=wait_for_answer();
			if(aux==SET){
				printf("Header well received\n");
				state=1;
				break;
			}

			printf("Header badly received\n");
			state=0;//ou return 0, who knows honestly
			break;
			
		case 1:
			while(!read(fd, &aux, 1)){}

			if(aux!=FLAG){
				printf("Nao recebemos last FLAG\n");
				state=0;	//ou return, who knows
				break;
			}

			printf("%ld bytes Written back\n",write(fd, input, SUP_SIZE));
			state++;
			printf("llopen RECEIVER done successfully (palmas)\n");
			return 1;

		break;


	  }
}
}
}


int llwrite(char *buf, int bufSize)
{

  int i = 0, k = 0, inputSize=bufSize+5, res;
  int stuffedSize;
  char *input = malloc(sizeof(char) * (inputSize));
  char stuffed[MAX_PAYLOAD_SIZE*2];
  char bcc2 = 0;
  char help;



  input[0] = FLAG;
  input[1] = A_T;
  input[2] = SET;   //atualizar S se receçao da resposta foi bem sucedida
  input[3] = input[1]^input[2];
	printf("1->%d\n2->%d\n3->%d\n", input[1],input[2],input[3]);



  // BCC2 creation
  // to be honest n sei de onde vem esta ideia do XOR, so we must ask teacher
  for (i = 0; i < bufSize; i++)
  {
	bcc2 ^= buf[i];
	input[i+HEADER_SIZE]=buf[i];
  }

	input[bufSize+HEADER_SIZE]=bcc2;

		for(i=0;i<10;i++){
		printf("input[%d]->%d\n", i, input[i]);
	}



  // Byte Stuffing
  // n tenho de ir verificar bit a bit, porque a leitura é feita byte a byte
  stuffed[0] = FLAG;
  k=0;
  for (i = 1; i < inputSize; i++)
  {
	if ((input[i] == 0x7e) || (input[i] == 0x7d))
	{
	  stuffed[i + k] = 0x7d;
	  k++;
	  stuffed[i + k] = input[i] ^ 0x20;
	}else
	{
	  stuffed[i + k] = input[i];
	}
  }
	
	stuffedSize = i + k;
	stuffed[stuffedSize]=FLAG;

	
	for(i=4;i<14;i++){
	printf("stuffed[%d]->%d\n", i, stuffed[i]);
	}



  state=0;
  k=0;
  while(state<2){

	switch (state){
	  
	  case 0:

		if (k > ll.numTries)
		{
		  printf("Nao recebeste dados nenhuns amigo, problems de envio\n");
		  return -1;
		  break;
		}

		k++;

		res=write(fd, stuffed, stuffedSize);
		printf("Escrevemos %d BYTES em llwrite\n", res);
		help=wait_for_answer();
		if(!help){
			printf("TimedOut, ou mal recebida, sending again\n");
			state=0;
			break;
		}

		state=1;
	  break;

	  case 1:

	  	//add code to verify rejection or not
		while(!read(fd, &help, 1)){}

			if(help!=FLAG){
				printf("Nao recebemos last FLAG\n");
				state=0;	//ou return, who knows
				break;
			}

			state++;
			printf("llwrite done successfully (palmas)(yet again)\n");
			return 1;


	  break;
		}

	  }
	

  }
  


///////////////////////////////////////
//////////////////////////////////////

// Receive data in packet, which has already  MAXSIZE
int llread(char *packet)
{
  int i, fd, length = 0, packetSize=0, res;
  char input[MAX_PAYLOAD_SIZE * 2];
  char output[5];
  char bcc2=0;
  char aux, help;



  output[0] = FLAG;
  output[1] = A_T;
  output[2] = UA;
  output[3] = output[1]^output[2];
  output[4] = FLAG;



	state=1;
	aux=wait_for_answer();

if(!aux){
	printf("Header not well received, wait transmission\n");
	return 0;
}

//como ja recebemos e verificamos, podemos escrever artificialmente os valores de input
	input[0]=FLAG;
	input[1]=A_T;
	input[2]=aux;
	input[3]=input[1]^input[2];


	int j=0;


//como ja lemos anteriormente o HEADER, ja so temos DATA e BCC2 ate FLAG
//FLAG also fica lida, need to tirar BCC2 de packet
printf("Inside read DATA while\n");
while(j<MAX_PAYLOAD_SIZE){
	res=read(fd, &help, 1);
	printf("%d Bytes of DATA read\n", res);
	if(j<10){
		printf("help[%d]=%d\n", j, help);
	}
	if(help==0x7d){
		(void) read(fd, &help, 1);
		help^=0x20;
	}
	if(help==FLAG)
		break;
	bcc2^=help;
	packet[j]=help;
	j++;
}


//debugging code
printf("LEFT READ DATA while\nbcc2=%d\n", bcc2);
for(j=0;j<6;j++){
	printf("packet[%d]->%d\n", j, packet[j]);
}



	//verificamos if input[i-2]==bcc2
  if(bcc2){
	printf("Error in received data, trying again\n");
	return 0;
  }
  printf("BCC2 well received in read\n");
  packet[i-2]='\0';


res=write(fd, output, 5);
printf("%d bytes written back in llread\n", res);

return packetSize;

}



// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close
int llclose(int showStatistics){

}