#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>  //for input output

using namespace std;
#pragma comment(lib,"Ws2_32.lib")  //add API

#define LOCAL_DNS "127.0.0.1"
#define DNS_PORT 53
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 1000                //DNStable size
#define NOTFOUND -1

typedef struct IPandDom
{
	string ip;						//IP address
	string domain;					//domain name
}IPandDom;

typedef struct IDTrans
{
	unsigned short oldID;			//old ID from the package
	SOCKADDR_IN client;			//socket address of client
}IDTrans;


IPandDom DNStable[AMOUNT];		//store the content reading from the dnsrelay.txt
IDTrans IDtranstable[AMOUNT];	    //store the corresponding relation between clientName and oldID which read from the buffer
int IDcount = 0;				   	//number of the trasaction in IDtranstable[AMOUNT]
char Web[LENGTH];			//a array store the domain name


/*
 * Function: int readFile(char *t)
 * Usage: read the ip address and domain name from the dnsrelay.txt
 then store them in the DNStable for retrieving
 * return numbers of rows in the DNStable
 */
int readFile(char *t)
{
	int i=0,j,k;
	string table[AMOUNT];
	ifstream infile(t,ios::in);//c++读文件的方法
	while(getline(infile,table[i]))//从左边第一个开始，读一行
	{
		i++;
		if(i==AMOUNT)
		{
			cout<<"DNStable is already full"<<endl;
			break;
		}
	}
	for(j=0;j<i;j++)
	{		
		k = table[j].find(' ');	                    //k is the positon of the space
		DNStable[j].ip = table[j].substr(0,k);     //ip is stored before the space
		DNStable[j].domain = table[j].substr(k+1); //domain name is stored after the space
     	cout<<"ip="<<DNStable[j].ip<<endl;
		cout<<"domain="<<DNStable[j].domain<<endl;
	}
	cout<<"The number of the trasactions in the DNStable="<<i<<" "<<endl;
    return i;
}


/*
 * Function: void obtAddr(char *recvbuf, int num)
 * Usage: read domain name from the recvbuffer to the 'name'
 then transform the domain name into the same form in the local database
 and store the transformed result into Web[]
 * Note:the domain name is stored between the 12th byte and (sizeof(recvnum)-16)th byte
 because domain name has a variable length and
 it is stored after the header and before the querytype and queryclass
 */
void obtAddr(char *recvbuf, int num)	//num=recvnum in byte
{
	char name[LENGTH];
	int i=0, j, k=0;
    
	memset(Web, 0, LENGTH);//初始化Web这个数组
	memcpy(name, &(recvbuf[6*sizeof(unsigned short)]), num-16);	//reference in this function's note，把第二个赋值给第一个，复制几位，把它的前num-16位赋值给第一个
    
	int len = strlen(name);//==num-16
	
	//Transforming the domain name in the recvbuffer into
	//the same form in dnsrelay.txt, according to ASCII table
	//e.g.@www#baidu%com& --> www.baidu.com\0
	//i=length of name
	//k=indexof(Web[])
	//j=number of characters to the next "."
    
	while (i < len) {
		if (name[i]>0 && name[i]<=63)
			j = name[i];
		i++;
		for (; j>0; j--,i++,k++)
			Web[k] = name[i];
		
		if (name[i] != 0)
			Web[k++] = '.';
	}
	Web[k] = '\0';
}

/*
 * Function: int Check(char* Web, int num)
 * Usage: check the domain name in the query from the resolver
 whether can be found in the dnsrelay.txt
 then return different values
 
 * Return:return -1 if not found; return index if found
 */
int Check(char* Web, int num)
{
	int k = NOTFOUND;//K=-1
	char* domain;
    cout<<"Web="<<Web<<endl;
	for (int i=0; i<num; i++) {
		domain = (char *)DNStable[i].domain.c_str();
	    cout<<"domain="<<domain<<endl;
		if (strcmp(domain, Web) == 0) {	//if found
			k = i;
			break;
		}
	}
	return k;
}


/*
 * Function: unsigned short regNewID (unsigned short oID, SOCKADDR_IN temp)
 * Usage: store the relative old ID and corresponding address in the IDTranstable
 return the index in the IDtrastable as the new ID
 (it will be transformed into net address in the following)
 ====used for convenient search ID and clientName when not found in dnsrelay.txt====
 * Return:return the index in the IDtrastable as the nID
 */
unsigned short regNewID (unsigned short oID, SOCKADDR_IN temp)//第二个参数是从哪发的地址，比如从relay发就是relay的地址
{
    IDtranstable[IDcount].oldID = oID;
    IDtranstable[IDcount].client = temp;
	IDcount++;
    
	return (unsigned short)(IDcount-1);	//return the index in the IDtrastable as the nID
}

int main()
{
    WSADATA wsaData;                //used to link the ws2_32.dll
    SOCKET  socketServer, socketLocal;	 //socketServer for DNS_Server(server) and Relay;
    //socketLocal for Relay(local) and Resolver(client)
    SOCKADDR_IN serverName, clientName, localName;	//DNS_Server(server)  Resolver(client) Relay(local)
    char recvbuf[BUF_SIZE],sendbuf[BUF_SIZE];
    char tablePath[100];                        //used for get the dnsrelay.txt
	char outerDns[16];                          //store the ip address of DNS_server (BUPT)
    int Lenclient, iSend, iRecv;
    int num,i;	                                //number of rows in DNStable
    
	/**********************INITIALISATION PART************************/
	strcpy(outerDns, "10.0.0.1");               //BUPT's DNS server IP address
	strcpy(tablePath, "C:/Users/Administrator/Desktop/dnsrelay.txt");          //DNS relay's fixed DNS table
	num = readFile(tablePath);				    //get DNStable
    
	for (i=0; i < AMOUNT; i++) {				//initialise a empty IDtranstable
		IDtranstable[i].oldID = 0;
		memset(&(IDtranstable[i].client), 0, sizeof(SOCKADDR_IN));
	}
    
    WSAStartup(MAKEWORD(2,2), &wsaData);			//initialise ws2_32.dll
    
	//Create sockets
    socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);
    
	//Construct address structure of relay(local server) and server
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_DNS);
    
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDns); //10.3.9.5
    
	//Binding local server(relay)
	if (bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName))) {
		cout << "Unsuccessful to bind port 53" << endl;
		exit(1);	//end the program
	}
	else
		cout << "Successful to bind port 53" << endl;
    
    
	/********************MAIN OPERATION OF THE RELAY******************/
	while (1) {
		Lenclient = sizeof(clientName);
        memset(recvbuf, 0, BUF_SIZE);
        
		
		//Recieve query from Resolver
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &Lenclient);//resolver给relay的
		if (iRecv == SOCKET_ERROR) {
			cout << "UNSUCCESSFUL to recieve the query: " << WSAGetLastError() << endl;
			continue;
		}
		else if (iRecv == 0) { //the peer has performed an orderly shutdown
			break;
		}
		else { // receive some data
		    //Transforming the domain name in the recvbuffer into
	        //the same form in dnsrelay.txt, according to ASCII table
	        //e.g.@www#baidu%com& --> www.baidu.com\0
            //i=length of name
	        //k=indexof(Web[])
	        //j=number of characters to the next "."
			obtAddr(recvbuf, iRecv);      	//get the domain name stored in Web[] which is in the same form in dnsrelay.txt
			int find = Check(Web, num);		//check domain name of the query stored in Web[](after transform to dnerelay.txt form)
			
			//Whether can be found in the dnsrelay.txt
			cout<<endl<<"find:"<<find;
			cout<<endl<<"Web:"<<Web;
			/////////////////PRINT1//////////////
			cout<<endl<<"Recieved the package of query from Resolver"<<endl;
			for(i = 0; i < iRecv; i++)
				printf("%.2x ",(unsigned char)recvbuf[i]);
			cout<<endl<<"Corresponding Web:"<<Web;
            
			//if not found in the DNStable
			if (find == NOTFOUND) {
                
				//ID transformation
				//rID is order in net
				//ntohs(*rID) is order in host
				//nID is order in net
				unsigned short *rID = (unsigned short *)malloc(sizeof(unsigned short));	//下面3句，形成新ID    malloc申请地址
				memcpy(rID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(   regNewID(ntohs(*rID), clientName)  );//ntohs与htons相反，htons将一个无符号短整型的主机数值转换为网络字节顺序
                
				memcpy(recvbuf, &nID, sizeof(unsigned short));
			    
				/////////////////PRINT2//////////////
				cout<<endl<<"Didnt found in the DNStable(dnsrelay.txt)"<<endl;
				printf("\t IDtransform:%.2x %.2x -->%.2x %.2x \n\n",(*rID)%0x100,(*rID)/0x100,nID%0x100,nID/0x100);
                
				//send recvbuf to DNS_server to ask for the response
				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				free(rID);	//free the memory which dynamically allocate(see malloc)
                
				//recieve recvbuf from DNS_server
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &Lenclient);//server给relay的
                
				//ID Transform(restore the Transaction ID)
				rID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(rID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*rID);
				unsigned short oID = htons(IDtranstable[m].oldID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
				//get the clientName from the IDtranstable
				clientName = IDtranstable[m].client;
                
				/////////////////PRINT3//////////////
				cout<<"Served ad Relay----Recieve from DNSserver"<<endl;
				for(i=0;i<iRecv;i++)
					printf("%.2x ",(unsigned char)recvbuf[i]);
				printf("\n\t id transformation:%.2x %.2x -->%.2x %.2x \n",(*rID)%0x100,(*rID)/0x100,oID%0x100,oID/0x100);
			    cout<<"After change the ID, send back to resolver"<<endl<<endl<<endl;
				
				//send recvbuffer to client
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				free(rID);	//free the dynamic allocated memory
			}
            
			//if found in the DNStable
			else {
				cout<<endl<<"!!FOUND in the DNStable!!"<<endl;
				//get the ID in the package
				unsigned short *rID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(rID, recvbuf, sizeof(unsigned short));
                
				//transform ID
				unsigned short nID = regNewID(ntohs(*rID), clientName);
                
				/*******Construct response***********/
				memcpy(sendbuf, recvbuf, iRecv);						//copy the package of the header and query section
				unsigned short a;
				memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//change the flag to 0 at first
                
				//modify the flag and answer count in header
				if (strcmp(DNStable[find].ip.c_str(), "0.0.0.0") == 0)//比较两个字符串，相同时返回0，第一个较大时返回时1，否则返回负值
				{
					a = htons(0x8183);//将一个无符号短整型的主机数值转换为网络字节顺序
					memcpy(&sendbuf[2], &a, sizeof(unsigned short));
					a = htons(0x0000);	//shield function(屏蔽),answer count=0
					cout<<"************Shield function:this address has been avoided*************"<<endl;
				}
				else
				{
					a = htons(0x8180);
					memcpy(&sendbuf[2], &a, sizeof(unsigned short));
					a = htons(0x0001);	//served as local server, answer count=1
					cout<<"************Served as local server, it can be quickly accessed in the dnsrelay.txt************"<<endl;
				}
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				int curLen = 0;
                
                
				//continue to construct DNS response
				char answer[16];
				unsigned short Name = htons(0xc00c); //
				memcpy(answer, &Name, sizeof(unsigned short));
				curLen += sizeof(unsigned short);
                
				unsigned short TypeA = htons(0x0001);
				memcpy(answer+curLen, &TypeA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);
                
				unsigned short ClassA = htons(0x0001);
				memcpy(answer+curLen, &ClassA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);
                
				unsigned long timeLive = htonl(0x00000100);
				memcpy(answer+curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
                
                
				if (strcmp(DNStable[find].ip.c_str(), "0.0.0.0") != 0){// if not 0.0.0.0 in the dnsrelay.txt
				    unsigned short IPLen = htons(0x0004);
			    	memcpy(answer+curLen, &IPLen, sizeof(unsigned short));
			    	curLen += sizeof(unsigned short);
                    
			    	unsigned long IP = (unsigned long) inet_addr(DNStable[find].ip.c_str());//change the ip into network byte order form
			    	memcpy(answer+curLen, &IP, sizeof(unsigned long));
			    	curLen += sizeof(unsigned long);
                    
					curLen += iRecv;
			    	//store the query package and recvbuffer(store the answer section) both in the sendbuf
					memcpy(sendbuf+iRecv, answer, curLen);
                    
					//send the response package
					iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
					if (iSend == SOCKET_ERROR) {
						cout << "Send Error!: " << WSAGetLastError() << endl;
						continue;
					}
					else if (iSend == 0)
						break;
                    
				}
				else{//if it's 0.0.0.0 in the dnsrelay.txt
					curLen += iRecv;
			    	//store the query package and recvbuffer(store the answer section) both in the sendbuf
					memcpy(sendbuf+iRecv, answer, curLen);
                    
					//send the response package
					iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
					if (iSend == SOCKET_ERROR) {
						cout << "Send Error!: " << WSAGetLastError() << endl;
						continue;
					}
					else if (iSend == 0)
						break;
				}
                
				/////////////////PRINT2//////////////
				cout<<"The package send to the resolver:"<<endl;
				for(i=0;i<(iRecv+16);i++)
					printf("%.2x ",(unsigned char)sendbuf[i]);
				cout<<endl<<endl<<endl;
				free(rID);		//free the dynamimc allocated memory
			}
		}
	}
    closesocket(socketServer);	//close the socket
	closesocket(socketLocal);
    WSACleanup();				//free the resource allocated to ws2_32.dll in initialization 
return 0;
}
