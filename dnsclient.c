#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_SIZE 512
#define SERV_PORT 53

char sendBuff[MAX_SIZE];
char receiveBuff[MAX_SIZE];
unsigned char domian_array[MAX_SIZE][MAX_SIZE] = {0};
unsigned short querytype_array[MAX_SIZE];

void processReceivedData();
void processPointerName(unsigned char location);
unsigned short getXID();

int main(int argc, unsigned char *argv[])
{
    struct sockaddr_in servaddr;

    int sockfd, n;

    // if(argc != 4) {
    //     fputs("usage: ./client domainname querytype(-A/-CNAME/-MX/-PTR) querymode(-I/-R)\n", stderr);
    //     exit(1);
    // }

    unsigned short QueryTypeInput = (0x0000);
    unsigned short RDInput = (0x0000);
    
    int i = 0;
    for(i = 1; i < (argc - 1); i += 2){
    	int j = 0;
    	for(j = 0; j < strlen(argv[i]); j++)
    	{
    		domian_array[(i - 1)/2][j] = argv[i][j];
    		printf("%d\n", argv[i][j]);
    	}

    	//strcpy(domian_array[(i - 1)/2], argv[i]);
    	if(strcmp("-A", argv[i + 1]) == 0) 
    		querytype_array[(i - 1)/2] = 0x0001;
	    else if(strcmp("-CNAME", argv[i + 1]) == 0) 
	    	querytype_array[(i - 1)/2] = 0x0005;
	    else if(strcmp("-MX", argv[i + 1]) == 0)
	    	querytype_array[(i - 1)/2] = 0x000f;
	    else if(strcmp("-PTR", argv[i + 1]) == 0)
	    	querytype_array[(i - 1)/2] = 0x000c;
	    else
    	{
        	printf("Unknown command \"%s\"\n", argv[2]);
        	exit(0);
    	}
    }


    printf("over\n");

	if(strcmp("-I", argv[argc - 1]) == 0) 
    	RDInput = 0x0000;
    else if(strcmp("-R", argv[argc - 1]) == 0) 
    	RDInput = 0x0100;
    else
    {
        printf("Unknown command \"%s\"\n", argv[3]);
        exit(0);
    }


 	unsigned short XID = getXID();
 	memcpy(&sendBuff[0], &XID, sizeof(unsigned short));

 	unsigned short flag = (0x0000);
 	flag = flag | (0x0000); // QR: 	   0000 0000 0000 0000 (16th bit)
 	flag = flag | (0x0000); // OPcode: 0000 0000 0000 0000 (12th-15th bit)
 	flag = flag | (0x0000); // AA: 	   0000 0000 0000 0000 (11th bit)
 	flag = flag | (0x0000); // TC:     0000 0000 0000 0000 (10th bit)
 	flag = flag | (RDInput); // RD:     0000 0001 0000 0000 (9th bit)
 	flag = flag | (0x0000); // RA:     0000 0000 0000 0000 (8th bit)
 	flag = flag | (0x0000); // Z: 	   0000 0000 0000 0000 (5th-7th bit)
 	flag = flag | (0x0000); // Rcode:  0000 0000 0000 0000 (1st-4th bit)
 	flag = htons(flag);
 	memcpy(&sendBuff[2], &flag, sizeof(unsigned short));

 	unsigned short qc = (argc - 2) / 2;
 	printf("%d\n", qc);
	unsigned short QCount = htons(qc);
	printf("%d\n", QCount);
 	memcpy(&sendBuff[4], &QCount, sizeof(unsigned short));

	unsigned short ACount = htons(0x0000);
 	memcpy(&sendBuff[6], &ACount, sizeof(unsigned short));

	unsigned short AuthCount = htons(0x0000);
 	memcpy(&sendBuff[8], &AuthCount, sizeof(unsigned short));

	unsigned short AddCount = htons(0x0000);
 	memcpy(&sendBuff[10], &AddCount, sizeof(unsigned short));

 	int totalLength = 12;
 	for(i = 0; i < (argc - 2)/2; i++)
 	{
 		if(querytype_array[i] == 0x0c)
 		{
 			char domain_rev[4][3];
 			char *domain;
	    	domain = strtok(domian_array[i], ".");
	    	
	    	strcpy(domain_rev[0], domain);

	    	domain = strtok(NULL, ".");
	    	strcpy(domain_rev[1], domain);

	    	domain = strtok(NULL, ".");
	    	strcpy(domain_rev[2], domain);

	    	domain = strtok(NULL, ".");
	    	strcpy(domain_rev[3], domain);


 			unsigned char ptrdomain[MAX_SIZE];
 			sprintf(ptrdomain, "%s.%s.%s.%s.in-addr.arpa", domain_rev[3], domain_rev[2], domain_rev[1], domain_rev[0]);

	    	domain = strtok(ptrdomain, ".");
	    	
	    	while(domain != NULL)
	    	{
	    		int length = strlen(domain);
	    		unsigned char domainLength = length;
	    		memcpy(&sendBuff[totalLength], &domainLength, sizeof(unsigned char));
	    		totalLength += 1;
	    		memcpy(&sendBuff[totalLength], domain, length);
	    		totalLength += length;
	    		domain = strtok(NULL, ".");
	   		}
 		}
 		else{
 			char *domain;
	    	domain = strtok(domian_array[i], ".");
	    	
	    	while(domain != NULL)
	    	{
	    		int length = strlen(domain);
	    		unsigned char domainLength = length;
	    		memcpy(&sendBuff[totalLength], &domainLength, sizeof(unsigned char));
	    		totalLength += 1;
	    		memcpy(&sendBuff[totalLength], domain, length);
	    		totalLength += length;
	    		domain = strtok(NULL, ".");
	   		}
 		}
 		

		unsigned char DomainEnd = (0x00);
	 	memcpy(&sendBuff[totalLength], &DomainEnd, sizeof(unsigned char));
	 	totalLength += 1;

	    unsigned short QueryType = htons(querytype_array[i]);
	 	memcpy(&sendBuff[totalLength], &QueryType, sizeof(unsigned short));
	 	totalLength += 2;

		unsigned short QueryClass = htons(0x0001);
		memcpy(&sendBuff[totalLength], &QueryClass, sizeof(unsigned short));
		totalLength += 2;
 	}




    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERV_PORT);
    
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
   


   	send(sockfd, sendBuff, totalLength, 0);

          
    recv(sockfd, receiveBuff, sizeof(receiveBuff), 0);  
	processReceivedData();

    close(sockfd);
    return 0;
}

void processReceivedData(){
	unsigned short flag = (0x0000);
	memcpy(&flag, &receiveBuff[2],sizeof(unsigned short));
	unsigned short AA = htons(flag) & (0x0400);
	unsigned short Rcode = htons(flag) & (0x0003);

	if(Rcode == (0x0003))
	{
		printf("%s\n", "No such name");
	}
	else
	{
		if(AA == (0x0400))
		{
			printf("%s\n", "Authority Answer:");
		}
		else
		{
			printf("%s\n", "Non-authority Answer:");
		}
	}

	unsigned short QCount = (0x0000);
 	memcpy(&QCount, &receiveBuff[4], sizeof(unsigned short));
 	QCount = htons(QCount);

	unsigned short ACount = (0x0000);
 	memcpy(&ACount, &receiveBuff[6], sizeof(unsigned short));
 	ACount = htons(ACount);

	unsigned short AuthCount = (0x0000);
 	memcpy(&AuthCount, &receiveBuff[8], sizeof(unsigned short));
	AuthCount = htons(AuthCount);

	unsigned short AddCount = (0x0000);
 	memcpy(&AddCount, &receiveBuff[10], sizeof(unsigned short));
	AddCount = htons(AddCount);

	unsigned short QueryType = (0x0000);
	int totalLength = 12;
	int i = 0;
	for(i = 0; i < QCount; i++)
	{
		unsigned char domainLength = (0x00);
		memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
		totalLength++;

		while(1){	
			totalLength += domainLength;
			memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
			if(domainLength == 0x00)
			{
				totalLength++;
				break;
			}
			else
			{
				totalLength++;
			}
		}
		totalLength+=4;
	}

	printf("\nAnswers: %d\n", ACount);
	if((Rcode == 0x0000) && (ACount == 0x0000))
	{
		switch(QueryType){
			case 0x0001:{
				printf("	No A RRs\n");
				break;
			}
			case 0x0005:{
				printf("	No CNAME RRs\n");
				break;
			}
			case 0x000c:{
				printf("	No PTR RRs\n");
				break;
			}
			case 0x000f:{
				printf("	No MX RRs\n");
				break;
			}
		}
	}

	for(i = 0; i < ACount; i++){
		printf("total:     %d\n", totalLength);
		unsigned char domainLength = (0x00);
		unsigned char location = (0x0000);
		memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
		totalLength++;
		if((domainLength & 0xc0) == (0xc0)){
			// Pointer at the start
			memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
			printf("	Name: ");
			processPointerName(location);
			totalLength += 2;


			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			printf("type: %d\n", type);
			switch(type)
			{
				case 0x0001: // A
				{
					printf("	Address: ");
					totalLength += 8;
					unsigned char IP[4];
					memcpy(&IP[0], &receiveBuff[totalLength], sizeof(unsigned char));
					memcpy(&IP[1], &receiveBuff[totalLength + 1], sizeof(unsigned char));
					memcpy(&IP[2], &receiveBuff[totalLength + 2], sizeof(unsigned char));
					memcpy(&IP[3], &receiveBuff[totalLength + 3], sizeof(unsigned char));
					totalLength += 4;
					printf("%d.%d.%d.%d\n", IP[0], IP[1], IP[2], IP[3]);
					break;
				}
				case 0x0005: // CNAME
				{
					printf("	CNAME: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;
					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x000c: // PTR
				{
					printf("	PTR: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;
					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x000f: // MX
				{
					printf("	MX: ");
					totalLength += 10;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + 1 + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += (1 + domainLength);
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}

							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
						}
					}
					break;
				}
			}
		} 
		else
		{
			// Not Pointer (maybe compelete domain name or mix)
			// Get all name information
			printf("	Name: ");
			while(1){	
				int j = 0;
				unsigned char data = 0x00;
				for(j = 0; j < domainLength; j++){
					memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
					printf("%c", data);
				}
					
				totalLength += domainLength;
				memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
				if(domainLength == 0x00)
				{
					printf("\n");
					totalLength++;
					break;
				}
				else if((domainLength & 0xc0) == (0xc0)) // pointer mix
				{
					printf(".");
					unsigned char location = (0x0000);
					memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
					processPointerName(location);
					totalLength += 2;
					break;
				}
				else
				{
					totalLength++;
					printf(".");
				}
			}

			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			switch(type)
			{
				case 0x0001: // A
				{
					printf("	Address: ");
					totalLength += 8;
					unsigned char IP[4];
					memcpy(&IP[0], &receiveBuff[totalLength], sizeof(unsigned char));
					memcpy(&IP[1], &receiveBuff[totalLength + 1], sizeof(unsigned char));
					memcpy(&IP[2], &receiveBuff[totalLength + 2], sizeof(unsigned char));
					memcpy(&IP[3], &receiveBuff[totalLength + 3], sizeof(unsigned char));
					totalLength += 4;
					printf("%d.%d.%d.%d\n", IP[0], IP[1], IP[2], IP[3]);
					break;
				}
				case 0x0005: // CNAME
				{
					printf("	CNAME: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x000c: // PTR
				{
					printf("	PTR: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;
					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x000f: // MX
				{
					printf("	MX: ");
					totalLength += 10;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					while(1){
							//no pointer or pointer mix
							// Get all name information
							
						int j = 0;
						unsigned char data = 0x00;
						for(j = 0; j < domainLength; j++){
							memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
							printf("%c", data);
						}
								
						totalLength += domainLength;
						memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
						if(domainLength == 0x00)
						{
							totalLength++;
							printf("\n");
							break;
						}
						else
						{
							totalLength++;
							printf(".");
						}
					}
					break;
				}
			}
		}
		if(i != (ACount - 1))
			printf("\n");
	}

	printf("\nAuthority RRs: %d\n", AuthCount);
	for(i = 0; i < AuthCount; i++){
		unsigned char domainLength = (0x00);
		unsigned char location = (0x0000);
		memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
		totalLength++;
		if((domainLength & 0xc0) == (0xc0)){
			// Pointer at the start
			memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
			printf("	Name: ");
			processPointerName(location);
			totalLength += 2;


			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			switch(type)
			{
				case 0x0002: // NS
				{
					printf("	Name Server: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x0006: // SOA
				{
					totalLength += 8;
					printf("	Primary name server: ");
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}

					printf("	Responsible authority's mailbox: ");
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
			}
		} 
		else
		{
			// Not Pointer (maybe compelete domain name or mix)
			// Get all name information
			while(1){
				printf("	Name: ");
				
				int j = 0;
				unsigned char data = 0x00;
				for(j = 0; j < domainLength; j++){
					memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
					printf("%c", data);
				}
					
				totalLength += domainLength;
				memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
				if(domainLength == 0x00)
				{
					totalLength++;
					printf("\n");
					break;
				}
				else if((domainLength & 0xc0) == (0xc0)) // pointer mix
				{
					printf(".");
					unsigned char location = (0x0000);
					memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
					processPointerName(location);
					totalLength += 2;
					break;
				}
				else
				{
					totalLength++;
					printf(".");
				}
			}

			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			switch(type)
			{
				case 0x0002: // NS
				{
					printf("	Name Server: ");
					totalLength += 8;
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
				case 0x0006: // SOA
				{
					totalLength += 8;
					printf("	Primary name server: ");
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}

					printf("	Responsible authority's mailbox: ");
					memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
					totalLength++;

					if((domainLength & 0xc0) == (0xc0)) // only pointer
					{
						unsigned char location = (0x0000);
						memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
						processPointerName(location);
						totalLength += 2;
					}
					else
					{
						while(1){
							// no pointer or pointer mix
							// Get all name information
							
							int j = 0;
							unsigned char data = 0x00;
							for(j = 0; j < domainLength; j++){
								memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
								printf("%c", data);
							}
								
							totalLength += domainLength;
							memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
							if(domainLength == 0x00)
							{
								totalLength++;
								printf("\n");
								break;
							}
							else if((domainLength & 0xc0) == (0xc0)) // pointer mix
							{
								printf(".");
								unsigned char location = (0x0000);
								memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
								processPointerName(location);
								totalLength += 2;
								break;
							}
							else
							{
								totalLength++;
								printf(".");
							}
						}
					}
					break;
				}
			}
		}
		if(i != (AuthCount - 1))
			printf("\n");
	}

	printf("\nAdditional RRs: %d\n", AddCount);
	for(i = 0; i < AddCount; i++){
		unsigned char domainLength = (0x00);
		unsigned char location = (0x0000);
		memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
		totalLength++;
		if((domainLength & 0xc0) == (0xc0)){
			// Pointer at the start
			memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
			printf("	Name: ");
			processPointerName(location);
			totalLength += 2;


			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			switch(type)
			{
				case 0x0001: // A
				{
					printf("	Address: ");
					totalLength += 8;
					unsigned char IP[4];
					memcpy(&IP[0], &receiveBuff[12 + totalLength], sizeof(unsigned char));
					memcpy(&IP[1], &receiveBuff[12 + totalLength + 1], sizeof(unsigned char));
					memcpy(&IP[2], &receiveBuff[12 + totalLength + 2], sizeof(unsigned char));
					memcpy(&IP[3], &receiveBuff[12 + totalLength + 3], sizeof(unsigned char));
					totalLength += 4;
					printf("%d.%d.%d.%d\n", IP[0], IP[1], IP[2], IP[3]);
					break;
				}
				// case 0x001c: // AAAA(IPv6)
				// {
				// 	printf("	AAAA(IPv6) Address: ");
				// 	totalLength += 8;
				// 	memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));

				// 	while(1){
				// 		// Pointer at the start
				// 		// Get all name information
						
				// 		int j = 0;
				// 		unsigned char data = 0x00;
				// 		for(j = 0; j < domainLength; j++){
				// 			memcpy(&data, &receiveBuff[12 + totalLength + 1 + j], sizeof(unsigned char));
				// 			printf("%c", data);
				// 		}
							
				// 		totalLength += (1 + domainLength);
				// 		memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));
				// 		if(domainLength == 0x00)
				// 		{
				// 			printf("\n");
				// 			break;
				// 		}
				// 		else if((domainLength & 0xc0) == (0xc0)) // pointer mix
				// 		{
				// 			printf(".");
				// 			unsigned char location = (0x0000);
				// 			memcpy(&location, &receiveBuff[12 + totalLength + 1], sizeof(unsigned char));
				// 			processPointerName(location);
				// 			totalLength += 2;
				// 			break;
				// 		}
				// 		else
				// 		{
				// 			printf(".");
				// 		}

				// 		memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));
				// 	}
				// 	break;
				// }
			}
		} 
		else
		{
			// Not Pointer (maybe compelete domain name or mix)
			// Get all name information
			printf("	Name: ");
			printf("not pointer\n");
			printf("%d\n", totalLength);
			printf("%d\n", domainLength);
			while(1){				
				int j = 0;
				unsigned char data = 0x00;
				for(j = 0; j < domainLength; j++){
					memcpy(&data, &receiveBuff[totalLength + j], sizeof(unsigned char));
					printf("%c", data);
				}
					
				totalLength += domainLength;
				memcpy(&domainLength, &receiveBuff[totalLength], sizeof(unsigned char));
				if(domainLength == 0x00)
				{
					printf("\n");
					totalLength++;
					break;
				}
				else if((domainLength & 0xc0) == (0xc0)) // pointer mix
				{
					printf(".");
					unsigned char location = (0x0000);
					memcpy(&location, &receiveBuff[totalLength + 1], sizeof(unsigned char));
					processPointerName(location);
					totalLength += 2;
					break;
				}
				else
				{
					totalLength++;
					printf(".");
				}
			}

			// Get rest of trhe data
			unsigned short type = (0x0000);
			memcpy(&type, &receiveBuff[totalLength], sizeof(unsigned short));
			type = htons(type);
			totalLength += 2;

			switch(type)
			{
				case 0x0001: // A
				{
					printf("	Address: ");
					totalLength += 8;
					unsigned char IP[4];
					memcpy(&IP[0], &receiveBuff[totalLength], sizeof(unsigned char));
					memcpy(&IP[1], &receiveBuff[totalLength + 1], sizeof(unsigned char));
					memcpy(&IP[2], &receiveBuff[totalLength + 2], sizeof(unsigned char));
					memcpy(&IP[3], &receiveBuff[totalLength + 3], sizeof(unsigned char));
					totalLength += 4;
					printf("%d.%d.%d.%d\n", IP[0], IP[1], IP[2], IP[3]);
					break;
				}
				// case 0x001c: // AAAA(IPv6)
				// {
				// 	printf("	AAAA(IPv6) Address: ");
				// 	totalLength += 8;
				// 	memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));

				// 	while(1){
				// 		// Pointer at the start
				// 		// Get all name information
						
				// 		int j = 0;
				// 		unsigned char data = 0x00;
				// 		for(j = 0; j < domainLength; j++){
				// 			memcpy(&data, &receiveBuff[12 + totalLength + 1 + j], sizeof(unsigned char));
				// 			printf("%c", data);
				// 		}
							
				// 		totalLength += (1 + domainLength);
				// 		memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));
				// 		if(domainLength == 0x00)
				// 		{
				// 			printf("\n");
				// 			break;
				// 		}
				// 		else if((domainLength & 0xc0) == (0xc0)) // pointer mix
				// 		{
				// 			printf(".");
				// 			unsigned char location = (0x0000);
				// 			memcpy(&location, &receiveBuff[12 + totalLength + 1], sizeof(unsigned char));
				// 			processPointerName(location);
				// 			totalLength += 2;
				// 			break;
				// 		}
				// 		else
				// 		{
				// 			printf(".");
				// 		}

				// 		memcpy(&domainLength, &receiveBuff[12 + totalLength], sizeof(unsigned char));
				// 	}
				// 	break;
				// }
			}
		}
		if(i != (AddCount - 1))
			printf("\n");
	}
}

void processPointerName(unsigned char location){
	int totalLengthTmp = 0;
	while(1)
	{
		unsigned char domainLengthTmp = (0x00);
		memcpy(&domainLengthTmp, &receiveBuff[location + totalLengthTmp], sizeof(unsigned char));
				
		int j = 0;
		unsigned char data = 0x00;
		for(j = 0; j < domainLengthTmp; j++){
			memcpy(&data, &receiveBuff[location + totalLengthTmp + 1 + j], sizeof(unsigned char));
			printf("%c", data);
		}
		
		totalLengthTmp += (1 + domainLengthTmp);
		memcpy(&domainLengthTmp, &receiveBuff[location + totalLengthTmp], sizeof(unsigned char));
		if(domainLengthTmp == 0x00)
		{
			totalLengthTmp++;
			printf("\n");
			break;
		}
		else if(domainLengthTmp  == 0xc0) // pointer mix
		{
			unsigned char newLocation = (0x00);
			memcpy(&newLocation, &receiveBuff[location + totalLengthTmp + 1], sizeof(unsigned char));
			printf(".");
			processPointerName(newLocation);
			break;
		}
		else
		{
			printf(".");
		}
	}
}

unsigned short getXID()
{
	srand((unsigned)time(NULL));
	return rand();
}