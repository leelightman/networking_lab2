/* fpont 12/99 */
/* pont.net    */
/* udpServer.c */

/* Converted to echo client/server with select() (timeout option). See udpClient.c */
/* 3/30/05 John Schultz */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <sys/stat.h>

#define MAX_MSG 100
#define PKT_SIZE 1000
#define PAYLOAD_SIZE 800

/* function prototypes */
void sendFileToClient(char *fileName, int sd, int flags, struct sockaddr *cliAddr, int cliLen);
int getFileSize(char *filePath);
int lostPacket(float pro);

int main(int argc, char *argv[]) {
  
  int sd, rc, n, cliLen, flags, portNum;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];
  float probability;

  if(argc!=3){
    printf("Usage: %s <portNumber> <pl>\n", argv[0]);
    exit(1);
  }

  portNum = atoi(argv[1]);
  probability = atof(argv[2]);
printf("%d %d %d %d %d \n.", lostPacket(probability), lostPacket(probability), lostPacket(probability), lostPacket(probability), lostPacket(probability));
  /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(portNum);
  rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) {
    printf("%s: cannot bind port number %d \n", 
	   argv[0], portNum);
    exit(1);
  }

  printf("%s: waiting for data on port UDP %u\n", 
	   argv[0],portNum);

/* BEGIN jcs 3/30/05 */

  flags = 0;

/* END jcs 3/30/05 */

  /* server infinite loop */
  while(1) {
    /*init srand*/
    srand(time(NULL));
    
    /* init buffer */
    memset(msg,0x0,MAX_MSG);

    /* receive message */
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msg, MAX_MSG, flags,
		 (struct sockaddr *) &cliAddr, &cliLen);

    if(n<0) {
      printf("%s: cannot receive data \n",argv[0]);
      continue;
    }
      
    /* print received message */
    printf("%s: from %s:UDP%u : %s \n", 
	   argv[0],inet_ntoa(cliAddr.sin_addr),
	   ntohs(cliAddr.sin_port),msg);

/* BEGIN jcs 3/30/05 */

    sleep(1);

    sendFileToClient(msg, sd, flags, (struct sockaddr *)&cliAddr, cliLen);

/* END jcs 3/30/05 */
    
  }/* end of server infinite loop */

return 0;
}

void sendFileToClient(char *fileName, int sd, int flags, struct sockaddr *cliAddr, int cliLen){
	FILE *fileId;
	char packet[PKT_SIZE];
	int fileLength, pktMaxNum, seqNum, headerLength, n, i, error;
	char *header = "%d %d\r\n\r\n";
	//open file
	fileId = fopen(fileName,"r"); // read mode
	if(fileId == NULL){
		printf("Cannot open file \"%s\" on server.\n", fileName);
		return;
	}
	// get file size
	fileLength = getFileSize(fileName);
	if(fileLength<0){printf("Error occurs while getting file size on server.\n");return;}
	// calculate the total number of packets
	pktMaxNum = fileLength % PAYLOAD_SIZE == 0 ? (fileLength/PAYLOAD_SIZE):(fileLength/PAYLOAD_SIZE+1);
	seqNum = 1;
	printf("Sending data to client...\n");
	for(i=0; i<pktMaxNum; i++){
		//make header
		headerLength = sprintf(packet, header, seqNum, pktMaxNum);
		if(headerLength > PKT_SIZE - PAYLOAD_SIZE){printf("Server: Header size cannot over %d.\n", PKT_SIZE - PAYLOAD_SIZE);return;}
		//read file content to packet
		n = fread(packet+headerLength, sizeof(char), PAYLOAD_SIZE, fileId);
		//send packet
		error = sendto(sd,packet,n+headerLength,flags,cliAddr,cliLen);
		if(error<0){printf("Cannot send packet %d to client.\n", seqNum);return;}else{printf("Packet %d is sent.\n", seqNum);}
		seqNum++;
	}
	fclose(fileId);
	printf("Data successfully sent.\n");
}

int getFileSize(char *filePath){
	struct stat fst;
	bzero(&fst,sizeof(fst));
        if (stat(filePath,&fst) != 0) {return -1;}
	return fst.st_size;
}

/*return 1 if loss packet; return 0 if not*/
int lostPacket(float pro){
	float rnd;
	rnd = (float)rand() / (float)RAND_MAX;
	return rnd < pro ? 1 : 0;
}
