#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), sendto() and recvfrom() */
#include <arpa/inet.h> /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <pthread.h>
#include <time.h>
#include <dirent.h>

#define MAX_SIZE 512
#define SERV_PORT 53

char recvBuff[MAX_SIZE]; /* Buffer for echo string */
char sendBuff[MAX_SIZE];
char answer[MAX_SIZE][MAX_SIZE];
char auth[MAX_SIZE][MAX_SIZE];
char addition[MAX_SIZE][MAX_SIZE];
int answer_length[MAX_SIZE] = {0};
int auth_length[MAX_SIZE] = {0};
int addition_length[MAX_SIZE] = {0};
int answer_count[MAX_SIZE] = {0};
int auth_count[MAX_SIZE] = {0};
int addition_count[MAX_SIZE] = {0};
int received_length = 0;
int send_length = 0;

pthread_t tid1;  
int rc1 = 0;  

void *thread1(void *arg);
void processReceivedData();
void processPointerName(unsigned char location);
void check_cache_answer(char name[], char type[], int query_number);
void check_cache_addition(char name[], char type[], int query_number);
void check_cache_ns_addition(char name[], char type[], int query_number);

int main(int argc, char *argv[]){
    int sock; /* Socket */
    struct sockaddr_in clientaddr; /* Local address */
    struct sockaddr_in serveraddr; /* Client address */
 
    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
       printf("socket() failed.\n");
    /* Construct local address structure */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port =htons(53);

    /* Bind to the local address */
    if ((bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr))) < 0)
       printf("bind() failed.\n");
    
    //rc1 = pthread_create(&tid1, NULL, thread1, &tid1);  
    //if(rc1 != 0)  
    //    printf("Thread create error!");  
    for (;;){ /* Run forever */
        unsigned int clientaddr_len = sizeof(clientaddr);
        received_length = recvfrom(sock, recvBuff, sizeof(recvBuff), 0, (struct sockaddr *) &clientaddr, &clientaddr_len);
        //pthread_cancel(tid1);
        processReceivedData();
        //rc1 = pthread_create(&tid1, NULL, thread1, &tid1);  
        //if(rc1 != 0)  
        //    printf("Thread create error!");  
        //printf("over\n");
        sendto(sock, sendBuff, send_length, 0, (struct sockaddr *) &clientaddr, clientaddr_len);
        //printf("sent\n");


        // Clear all the parameters

        memset(recvBuff, 0, sizeof(recvBuff)/sizeof(char));
        memset(sendBuff, 0, sizeof(sendBuff)/sizeof(char));
        memset(answer, 0, sizeof(answer)/sizeof(char));
        memset(auth, 0, sizeof(auth)/sizeof(char));
        memset(addition, 0, sizeof(addition)/sizeof(char));

        memset(answer_length, 0, sizeof(answer_length));
        memset(auth_length, 0, sizeof(auth_length));
        memset(addition_length, 0, sizeof(addition_length));
        memset(answer_count, 0, sizeof(answer_count));
        memset(auth_count, 0, sizeof(auth_count));
        memset(addition_count, 0, sizeof(addition_count));

        received_length = 0;
        send_length = 0;
    }
}

void *thread1(void *arg)  
{
    time_t t1;  
    t1 = time(NULL);  

    int timer = time(&t1);
    while(1)
    {
        time_t t2;  
        t2 = time(NULL);   
        int timestamp = time(&t2);
        if(timestamp > timer)
        {
            timer = timestamp;

            DIR *dir;
            struct dirent *ptr;
            char folder[MAX_SIZE] = "./RR";
            dir = opendir(folder);
            while((ptr = readdir(dir)) != NULL)
            {
                if(strcmp(ptr->d_name, "..") == 0 || strcmp(ptr->d_name, ".") == 0)
                    ;
                else
                {
                    char sub_folder[MAX_SIZE] = {'\0'};
                    strcpy(sub_folder, folder);
                    sub_folder[strlen(folder)] = '/';
                    int i = 0;
                    for(i = 0; i < strlen(ptr->d_name); i++)
                    {
                        sub_folder[strlen(folder) + i + 1] = (ptr->d_name)[i];
                    }
                    sub_folder[strlen(sub_folder)] = '\0';

                    DIR *sub_dir;
                    struct dirent *sub_ptr;
                    sub_dir = opendir(sub_folder);
                    while((sub_ptr = readdir(sub_dir)) != NULL)
                    {
                        if(strcmp(sub_ptr->d_name, "..") == 0 || strcmp(sub_ptr->d_name, ".") == 0)
                            ;
                        else
                        {
                            char file_path[MAX_SIZE] = {'\0'};
                            strcpy(file_path, sub_folder);
                            file_path[strlen(sub_folder)] = '/';
                            int j = 0;
                            for(j = 0; j < strlen(sub_ptr->d_name); j++)
                            {
                                file_path[strlen(sub_folder) + j + 1] = (sub_ptr->d_name)[j];
                            }
                            file_path[strlen(file_path)] = '\0';

                            char new_file_path[MAX_SIZE] = {'\0'};
                            strcpy(new_file_path, file_path);

                            new_file_path[strlen(file_path)] = '1';
                            new_file_path[strlen(file_path) + 1] = '\0';

                            if(strcmp(ptr->d_name, "MX") == 0)
                            {
                                FILE* out;
                                FILE* in;
               
                                in = fopen(new_file_path, "w");
                                if ((out = fopen(file_path, "r")) == NULL)  
                                {     
                                    fclose(out);
                                }
                                else
                                {
                                    char tmp1[MAX_SIZE];
                                    char ttl_tmp[MAX_SIZE];
                                    char tmp2[MAX_SIZE];
                                    
                                    while(fscanf(out,"%s",tmp1) != EOF) 
                                    {
                                        unsigned int ttl = 0;
                                        char ttl_tmp[10];
                                        fscanf(out,"%s", ttl_tmp);
                                        ttl = atoi(ttl_tmp);
                                        ttl--;
                                        fscanf(out,"%s",tmp2);
                                        if(ttl <= 0)
                                            ;
                                        else
                                            fprintf(in, "%s %d %s\n", tmp1, ttl, tmp2);
                                    }

                                    remove(file_path);
                                    rename(new_file_path, file_path);

                                    fclose(out);
                                    fclose(in);
                                }
                            }
                            else
                            {
                                FILE* out;
                                FILE* in;
               
                                in = fopen(new_file_path, "w");
                                if ((out = fopen(file_path, "r")) == NULL)  
                                {     
                                    fclose(out);
                                }
                                else
                                {
                                    char tmp1[MAX_SIZE];
                                    char ttl_tmp[MAX_SIZE];
                                    
                                    while(fscanf(out,"%s",tmp1) != EOF) 
                                    {
                                        unsigned int ttl = 0;
                                        char ttl_tmp[10];
                                        fscanf(out,"%s", ttl_tmp);
                                        ttl = atoi(ttl_tmp);
                                        ttl--;
                                        if(ttl <= 0)
                                            ;
                                        else
                                            fprintf(in, "%s %d\n", tmp1, ttl);
                                    }

                                    remove(file_path);
                                    rename(new_file_path, file_path);

                                    fclose(out);
                                    fclose(in);
                                }
                            }
                        }
                    }
                    closedir(sub_dir);
                }
            }
            closedir(dir);
        }
    }
}

void processReceivedData(){
    memcpy(&sendBuff, &recvBuff, received_length);
    unsigned short flag = (0x0000);
    memcpy(&flag, &recvBuff[2],sizeof(unsigned short));
    unsigned short RD = htons(flag) & (0x0100);

    unsigned short QCount = (0x0000);
    memcpy(&QCount, &recvBuff[4], sizeof(unsigned short));
    QCount = htons(QCount);

    unsigned short QueryType = (0x0000);
    
    int totalLength = 0;
    int i = 0;

    int flag_A = 0;
    int flag_reply_code = 0;

    printf("%d\n", QCount);
    for(i = 0; i < QCount; i++)
    {
        printf("%d\n", i);
        unsigned char domainLength = (0x00);
        memcpy(&domainLength, &recvBuff[12 + totalLength], sizeof(unsigned char));
        //printf("Name: ");
        char query_domain_name[MAX_SIZE];
        int query_domain_name_length = 0;
        while(1)
        {
            int j = 0;
            unsigned char data = 0x00;
            for(j = 0; j < domainLength; j++){
                memcpy(&data, &recvBuff[12 + totalLength + 1 + j], sizeof(unsigned char));
                query_domain_name[query_domain_name_length] = data;
                query_domain_name_length++;
            }
                    
            totalLength += (1 + domainLength);
            memcpy(&domainLength, &recvBuff[12 + totalLength], sizeof(unsigned char));
            if(domainLength == 0x00)
            {
                query_domain_name[query_domain_name_length] = '\0';
                totalLength++;
                break;
            }
            else
            {
                query_domain_name[query_domain_name_length] = '.';
                query_domain_name_length++;
            }
        }
        printf("%s\n", query_domain_name);

        unsigned short QueryType = (0x0000);
        memcpy(&QueryType, &recvBuff[12 + totalLength], sizeof(unsigned short));
        QueryType = htons(QueryType);
        totalLength += 2;

        // for query class
        totalLength += 2;

        char query_type[MAX_SIZE];
        switch(QueryType)
        {
            case 0x0001: //A
            {
                memcpy(query_type, "A", 2);
                query_type[1] = '\0';
                break;
            }
            case 0x0005: //CNAME
            {
                memcpy(query_type, "CNAME", 5);
                query_type[5] = '\0';
                break;
            }
            case 0x000c: //PTR
            {
                memcpy(query_type, "PTR", 4);
                query_type[3] = '\0';
                break;
            }
            case 0x000f: //MX
            {
                memcpy(query_type, "MX", 3);
                query_type[2] = '\0';
                break;
            }
            default:
            {
                printf("error");
            }
        }
        
        printf("check file\n");
        check_cache_answer(query_domain_name, query_type, i);
        printf("check file\n");
        //printf("123\n");
        // query to root server or TLD according to 
        if(strlen(answer[i]) == 0)
        {
            printf("No cache query upper level server\n");
            printf("%s\n", query_domain_name);
            //flag_A = 1;
            // if there are no cache, then return the TLD server address
            char *TLD;
            char TLD_domain[MAX_SIZE][MAX_SIZE];
            TLD = strtok(query_domain_name, ".");
            printf("%s\n", TLD);
            int level = 0;
            while(TLD != NULL)
            {
                level++;
                int length = strlen(TLD);
                memcpy(&TLD_domain[level], TLD, length);
                TLD_domain[level][length] = '\0';
                TLD = strtok(NULL, ".");
            }
            printf("%s\n", TLD_domain[level]);

            FILE* upop;
            int name_length = strlen(TLD_domain[level]);
            // char name_dup[strlen(name)];
            // strcpy(name_dup, name);

            char file_path[6 + name_length];
            file_path[0] ='.';
            file_path[1] ='/';
            file_path[2] ='N';
            file_path[3] ='S';
            file_path[4] ='/';
            int x = 0;

            for(x = 0; x < name_length; x++)
            {
                file_path[5 + x] = TLD_domain[level][x];
            }

            file_path[5 + name_length] = '\0';

            printf("%s\n", file_path);

            if ((upop = fopen(file_path, "r")) == NULL)  
            {     
                flag_reply_code = 3;

                upop = fopen("SOA", "r");

                time_t t = time(0); 
                char tmp[11]; 
                strftime(tmp, sizeof(tmp), "%Y%m%d00", localtime(&t)); 
                int time = atoi(tmp);

                char PNSC[MAX_SIZE];
                fscanf(upop,"%s",PNSC);

                char TTLC[MAX_SIZE];
                fscanf(upop,"%s",TTLC);
                int ttl = atoi(TTLC);

                char RAMC[MAX_SIZE];
                fscanf(upop,"%s",RAMC);

                char RefIC[MAX_SIZE];
                fscanf(upop,"%s",RefIC);
                int refi = atoi(RefIC);

                char RetIC[MAX_SIZE];
                fscanf(upop,"%s",RetIC);
                int reti = atoi(RetIC);

                char ELC[MAX_SIZE];
                fscanf(upop,"%s",ELC);
                int el = atoi(ELC);

                char MTTLC[MAX_SIZE];
                fscanf(upop,"%s",MTTLC);
                int mttl = atoi(MTTLC);


                int totalLength = 0;

                auth_count[i]++;
                printf("auth_count%d\n", auth_count[i]);
                unsigned char name = (0x00);
                memcpy(&auth[i][totalLength], &name, sizeof(unsigned char));
                totalLength++;

                unsigned char DomainEnd = (0x00);
                memcpy(&auth[i][totalLength], &DomainEnd, sizeof(unsigned char));
                totalLength++;

                unsigned short type = htons(0x06);
                memcpy(&auth[i][totalLength], &type, sizeof(unsigned short));
                totalLength += 2;

                unsigned short class = htons(0x01);
                memcpy(&auth[i][totalLength], &class, sizeof(unsigned short));
                totalLength += 2;

                unsigned int TTL = htonl(ttl);
                memcpy(&auth[i][totalLength], &TTL, sizeof(unsigned int));
                totalLength += 4;

                unsigned short data_length = 0;
                unsigned int data_length_locaiton = totalLength;
                totalLength += 2;

                char *domain;
                //strcpy(name,name_dup);
                domain = strtok(PNSC, ".");
                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&auth[i][totalLength], &domainLength, sizeof(unsigned char));
                    totalLength++;
                    data_length++;
                    memcpy(&auth[i][totalLength], domain, length);
                    totalLength += length;
                    data_length += length;
                    domain = strtok(NULL, ".");
                }
                DomainEnd = (0x00);
                memcpy(&auth[i][totalLength], &DomainEnd, sizeof(unsigned char));
                totalLength++;
                data_length++;

                domain = strtok(RAMC, ".");
                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&auth[i][totalLength], &domainLength, sizeof(unsigned char));
                    totalLength++;
                    data_length++;
                    memcpy(&auth[i][totalLength], domain, length);
                    totalLength += length;
                    data_length += length;
                    domain = strtok(NULL, ".");
                }
                memcpy(&auth[i][totalLength], &DomainEnd, sizeof(unsigned char));
                totalLength++;
                data_length++;

                unsigned int SN = htonl(time);
                memcpy(&auth[i][totalLength], &SN, sizeof(unsigned int));
                totalLength += 4;
                data_length += 4;

                unsigned int REFI = htonl(refi);
                memcpy(&auth[i][totalLength], &REFI, sizeof(unsigned int));
                totalLength += 4;
                data_length += 4;

                unsigned int RETI = htonl(reti);
                memcpy(&auth[i][totalLength], &RETI, sizeof(unsigned int));
                totalLength += 4;
                data_length += 4;

                unsigned int EL = htonl(el);
                memcpy(&auth[i][totalLength], &EL, sizeof(unsigned int));
                totalLength += 4;
                data_length += 4;

                unsigned int MTTL = htonl(mttl);
                memcpy(&auth[i][totalLength], &MTTL, sizeof(unsigned int));
                totalLength += 4;
                data_length += 4;

                data_length = htons(data_length);
                memcpy(&auth[i][data_length_locaiton], &data_length, sizeof(unsigned short));
                // add SOA info to auth part
                auth_length[i] = totalLength;
                printf("SOA%d\n", totalLength);
            }
            else
            {
                char ns_name[MAX_SIZE];
                char ns_name_dup[MAX_SIZE];
                fscanf(upop,"%s",ns_name);
                strcpy(ns_name_dup, ns_name);
               
                char TTLC[MAX_SIZE];
                fscanf(upop,"%s",TTLC);
                int ttl = atoi(TTLC);

                int totalLength = 0;

                auth_count[i]++;
            
                char *domain;
                //strcpy(name,name_dup);
                char TLD[MAX_SIZE] = {'\0'};
                for(x = 0; x < strlen(TLD_domain[level - 1]); x++)
                {
                    TLD[x] = TLD_domain[level - 1][x];
                }
                TLD[strlen(TLD_domain[level - 1])] = '.';
                for(x = 0; x < strlen(TLD_domain[level]); x++)
                {
                    TLD[strlen(TLD_domain[level - 1]) + 1 + x] = TLD_domain[level][x];
                }

                printf("TLD: %s\n", TLD);
                domain = strtok(TLD, ".");
                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&answer[i][totalLength], &domainLength, sizeof(unsigned char));
                    totalLength++;
                    memcpy(&answer[i][totalLength], domain, length);
                    totalLength += length;
                    domain = strtok(NULL, ".");
                }
                unsigned char DomainEnd = (0x00);
                memcpy(&answer[i][totalLength], &DomainEnd, sizeof(unsigned char));
                totalLength++;

                unsigned short type = htons(0x02);
                memcpy(&answer[i][totalLength], &type, sizeof(unsigned short));
                totalLength += 2;

                unsigned short class = htons(0x01);
                memcpy(&answer[i][totalLength], &class, sizeof(unsigned short));
                totalLength += 2;

                unsigned int TTL = htonl(ttl);
                memcpy(&answer[i][totalLength], &TTL, sizeof(unsigned int));
                totalLength += 4;

                unsigned short data_length = 0;
                unsigned int data_length_locaiton = totalLength;
                totalLength += 2;

                domain = strtok(ns_name, ".");
                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&answer[i][totalLength], &domainLength, sizeof(unsigned char));
                    totalLength++;
                    data_length++;
                    memcpy(&answer[i][totalLength], domain, length);
                    totalLength += length;
                    data_length += length;
                    domain = strtok(NULL, ".");
                }
                memcpy(&answer[i][totalLength], &DomainEnd, sizeof(unsigned char));
                totalLength++;
                data_length++;

                data_length = htons(data_length);
                memcpy(&answer[i][data_length_locaiton], &data_length, sizeof(unsigned short));
                // add SOA info to auth part
                answer_length[i] = totalLength;
                printf("%s\n", "pass");
                check_cache_ns_addition(ns_name_dup, "A", i);
                printf("%s\n", "pass");
            }
        }
    }
    // set flags
    flag = htons(flag);
    flag = flag | (0x8000);
    if(flag_A == 1)
        flag = flag | (0x0400);
    if(flag_reply_code == 3)
        flag = flag | (0x0003);

    flag = htons(flag);
    memcpy(&sendBuff[2], &flag, sizeof(unsigned short));
    // set answer/authority/addition count

    unsigned short ACount = 0;
    for(i = 0; i < MAX_SIZE; i++)
    {
        if(answer_count[i] == 0)
            break;
        ACount += answer_count[i];
    }
    ACount = htons(ACount);
    printf("ACount: %d\n", ACount);
    memcpy(&sendBuff[6], &ACount, sizeof(unsigned short));


    unsigned short AuthCount = 0;
    for(i = 0; i < MAX_SIZE; i++)
    {
        printf("%d\n", auth_count[i]);
        if(auth_count[i] == 0)
            break;
        AuthCount += auth_count[i];
    }
    AuthCount = htons(AuthCount);
    memcpy(&sendBuff[8], &AuthCount, sizeof(unsigned short));
    printf("AuthCount%d\n", AuthCount);

    unsigned short AddCount = 0;
    for(i = 0; i < MAX_SIZE; i++)
    {
        if(addition_count[i] == 0)
            break;
        AddCount += addition_count[i];
    }
    AddCount = htons(AddCount);
    memcpy(&sendBuff[10], &AddCount, sizeof(unsigned short));

    totalLength = received_length;

    // set answer RR
    //int total_A_Length = 0;
    for(i = 0; i < QCount; i++)
    {
        memcpy(&sendBuff[totalLength], &answer[i], answer_length[i]);
        //printf("answer_length: %d\n", answer_length[i]);
        totalLength += answer_length[i];
    }

    //printf("totalLength: %d\n", totalLength);

    // set authority RR
    for(i = 0; i < QCount; i++)
    {
        memcpy(&sendBuff[totalLength], &auth[i], auth_length[i]);
        totalLength += auth_length[i];
        //printf("addition_length: %d\n", addition_length[i]);
    }

    // set additon RR
    //int total_A_Length = 0;
    for(i = 0; i < QCount; i++)
    {
        memcpy(&sendBuff[totalLength], &addition[i], addition_length[i]);
        totalLength += addition_length[i];
        //printf("addition_length: %d\n", addition_length[i]);
    }
    send_length = totalLength;
    //printf("totalLength%d\n", totalLength);
}

void check_cache_answer(char name[], char type[], int query_number)
{
    FILE* upop;
    int name_length = strlen(name);
    int type_length = strlen(type);
    char name_dup[strlen(name)];
    strcpy(name_dup, name);

    char file_path[6 + name_length + type_length];
    file_path[0] ='.';
    file_path[1] ='/';
    file_path[2] ='R';
    file_path[3] ='R';
    file_path[4] ='/';
    int i = 0;
    for(i = 0; i < type_length; i++)
    {
        file_path[5 + i] = type[i];
    }

    file_path[5 + type_length] = '/';

    for(i = 0; i < name_length; i++)
    {
        file_path[6 + type_length + i] = name[i];
    }

    file_path[6 + type_length + name_length] = '\0';


    //printf("%s\n", file_path);
    //printf("%d\n", strlen(file_path));

    if ((upop = fopen(file_path, "r")) == NULL)  
    {     
        printf("123no\n");
        answer[query_number][0] = '\0';
        addition[query_number][0] = '\0';
        printf("123no\n");
        //fclose(upop);
        printf("123no\n");
        return;
    }
    else
    {
        char result[MAX_SIZE];
        char mx_cname[MAX_SIZE];
        int total_answer_length = 0;
        if(strcmp(type, "MX") == 0)
        {
            int sub_answer_length = 0;
            // Get all answers
            //printf("before while\n");
            while(fscanf(upop,"%s",result) != EOF) 
            {
                strcpy(mx_cname, result); 
                unsigned int ttl = 0;
                unsigned int prefer = 0;
                char ttl_tmp[10];
                char prefer_tmp[10];
            
                fscanf(upop,"%s", ttl_tmp);
                fscanf(upop,"%s", prefer_tmp);
               
                ttl = atoi(ttl_tmp);
                prefer = atoi(prefer_tmp);

                answer_count[query_number] ++;
                // construct answer section
               
                char *domain;
                strcpy(name,name_dup);
                domain = strtok(name, ".");
                
                //printf("before domain\n");
                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&answer[query_number][sub_answer_length], &domainLength, sizeof(unsigned char));
                    sub_answer_length += 1;
                    memcpy(&answer[query_number][sub_answer_length], domain, length);
                    sub_answer_length += length;
                    domain = strtok(NULL, ".");
                }
                unsigned char DomainEnd = (0x00);
                memcpy(&answer[query_number][sub_answer_length], &DomainEnd, sizeof(unsigned char));
                sub_answer_length++;

                unsigned short QueryType = htons(0x000f);
                memcpy(&answer[query_number][sub_answer_length], &QueryType, sizeof(unsigned short));
                sub_answer_length += 2;

                unsigned short QueryClass = htons(0x0001);
                memcpy(&answer[query_number][sub_answer_length], &QueryClass, sizeof(unsigned short));
                sub_answer_length += 2;

                unsigned int TTL = htonl(ttl);
                memcpy(&answer[query_number][sub_answer_length], &TTL, sizeof(unsigned int));
                sub_answer_length += 4;

                unsigned short data_length = 0;
                domain = strtok(result, ".");
                unsigned int data_length_locaiton = sub_answer_length;
                sub_answer_length += 2;

                unsigned int PREFER = htonl(prefer);
                memcpy(&answer[query_number][sub_answer_length], &PREFER, sizeof(unsigned int));
                sub_answer_length += 2;
                data_length += 2;

                while(domain != NULL)
                {
                    int length = strlen(domain);
                    unsigned char domainLength = length;
                    memcpy(&answer[query_number][sub_answer_length], &domainLength, sizeof(unsigned char));
                    sub_answer_length += 1;
                    data_length ++;
                    memcpy(&answer[query_number][sub_answer_length], domain, length);
                    sub_answer_length += length;
                    data_length += length;
                    domain = strtok(NULL, ".");
                }

                memcpy(&answer[query_number][sub_answer_length], &DomainEnd, sizeof(unsigned char));
                sub_answer_length++;
                data_length++;

                data_length = htons(data_length);
                memcpy(&answer[query_number][data_length_locaiton], &data_length, sizeof(unsigned short));
                
                total_answer_length = sub_answer_length;
                check_cache_addition(mx_cname, "A", query_number);
            }
        }
        else
        {
            int sub_answer_length = 0;
            // Get all answers
            while(fscanf(upop,"%s",result) != EOF) 
            {
                unsigned int ttl = 0;
                char ttl_tmp[10];
                fscanf(upop,"%s", ttl_tmp);
                ttl = atoi(ttl_tmp);
                
                answer_count[query_number] ++;
                // construct answer section
                if(strcmp(type, "CNAME") == 0)
                {
                    char *domain;
                    strcpy(name, name_dup);
                    domain = strtok(name, ".");
                    
                    while(domain != NULL)
                    {
                        int length = strlen(domain);
                        unsigned char domainLength = length;
                        memcpy(&answer[query_number][sub_answer_length], &domainLength, sizeof(unsigned char));
                        sub_answer_length += 1;
                        memcpy(&answer[query_number][sub_answer_length], domain, length);
                        sub_answer_length += length;
                        domain = strtok(NULL, ".");
                    }
                    unsigned char DomainEnd = (0x00);
                    memcpy(&answer[query_number][sub_answer_length], &DomainEnd, sizeof(unsigned char));
                    sub_answer_length++;

                    unsigned short QueryType = htons(0x0005);
                    memcpy(&answer[query_number][sub_answer_length], &QueryType, sizeof(unsigned short));
                    sub_answer_length += 2;

                    unsigned short QueryClass = htons(0x0001);
                    memcpy(&answer[query_number][sub_answer_length], &QueryClass, sizeof(unsigned short));
                    sub_answer_length += 2;

                    unsigned int TTL = htonl(ttl);
                    memcpy(&answer[query_number][sub_answer_length], &TTL, sizeof(unsigned short));
                    sub_answer_length += 4;

                    unsigned short data_length = 0;
                    domain = strtok(result, ".");
                    unsigned int data_length_locaiton = sub_answer_length;
                    sub_answer_length += 2;

                    while(domain != NULL)
                    {
                        int length = strlen(domain);
                        unsigned char domainLength = length;
                        memcpy(&answer[query_number][sub_answer_length], &domainLength, sizeof(unsigned char));
                        sub_answer_length += 1;
                        data_length ++;
                        memcpy(&answer[query_number][sub_answer_length], domain, length);
                        sub_answer_length += length;
                        data_length += length;
                        domain = strtok(NULL, ".");
                    }

                    memcpy(&answer[query_number][sub_answer_length], &DomainEnd, sizeof(unsigned char));
                    sub_answer_length++;
                    data_length++;

                    data_length = htons(data_length);
                    memcpy(&answer[query_number][data_length_locaiton], &data_length, sizeof(unsigned short));
                }
                else if (strcmp(type, "A") == 0)
                {
                    char *domain;
                    strcpy(name, name_dup);
                    domain = strtok(name, ".");
                    
                    while(domain != NULL)
                    {
                        int length = strlen(domain);
                        unsigned char domainLength = length;
                        memcpy(&answer[query_number][sub_answer_length], &domainLength, sizeof(unsigned char));
                        sub_answer_length += 1;
                        memcpy(&answer[query_number][sub_answer_length], domain, length);
                        sub_answer_length += length;
                        domain = strtok(NULL, ".");
                    }
                    unsigned char DomainEnd = (0x00);
                    memcpy(&answer[query_number][sub_answer_length], &DomainEnd, sizeof(unsigned char));
                    sub_answer_length++;

                    unsigned short QueryType = htons(0x0001);
                    memcpy(&answer[query_number][sub_answer_length], &QueryType, sizeof(unsigned short));
                    sub_answer_length += 2;

                    unsigned short QueryClass = htons(0x0001);
                    memcpy(&answer[query_number][sub_answer_length], &QueryClass, sizeof(unsigned short));
                    sub_answer_length += 2;

                    unsigned int TTL = htonl(ttl);
                    memcpy(&answer[query_number][sub_answer_length], &TTL, sizeof(unsigned short));
                    sub_answer_length += 4;

                    unsigned short data_length = 0;
                    domain = strtok(result, ".");
                    unsigned int data_length_locaiton = sub_answer_length;
                    sub_answer_length += 2;


                    while(domain != NULL)
                    {
                        int IP = atoi(domain);
                        unsigned char IP_char = IP;
                        memcpy(&answer[query_number][sub_answer_length], &IP_char, sizeof(unsigned char));
                        sub_answer_length ++;
                        data_length ++;
                        domain = strtok(NULL, ".");
                    }

                    data_length = htons(0x0004);
                    memcpy(&answer[query_number][data_length_locaiton], &data_length, sizeof(unsigned short));
                }
                else // PTR
                {

                }
                total_answer_length = sub_answer_length;
            }
        }
        answer_length[query_number] = total_answer_length;
        fclose(upop);
    }
}

void check_cache_addition(char name[], char type[], int query_number)
{
    FILE* upop;
    int name_length = strlen(name);
    int type_length = strlen(type);
    char file_path[MAX_SIZE];
    file_path[0] ='.';
    file_path[1] ='/';
    file_path[2] ='R';
    file_path[3] ='R';
    file_path[4] ='/';
    int i = 0;
    for(i = 0; i < type_length; i++)
    {
        file_path[5 + i] = type[i];
    }

    file_path[5 + type_length] = '/';

    for(i = 0; i < name_length; i++)
    {
        file_path[5 + type_length + 1 + i] = name[i];
    }

    file_path[6 + type_length + name_length] = '\0';

    if ((upop = fopen(file_path, "r")) == NULL)  
    {     
        //answer[query_number][0] = '\0';
        addition[query_number][0] = '\0';
        fclose(upop);
        return;
    }
    else
    {
        char result[MAX_SIZE];
        int total_addition_length = 0;
       
        int sub_addition_length = addition_length[query_number];
        // Get all addition
        while(fscanf(upop,"%s",result) != EOF) 
        {
            unsigned int ttl = 0;
            char ttl_tmp[10];
            fscanf(upop,"%s", ttl_tmp);
            ttl = atoi(ttl_tmp);
               
            addition_count[query_number] ++;
            // construct addition section
                
            char *domain;
            domain = strtok(name, ".");
                    
            while(domain != NULL)
            {
                int length = strlen(domain);
                unsigned char domainLength = length;
                memcpy(&addition[query_number][sub_addition_length], &domainLength, sizeof(unsigned char));
                sub_addition_length += 1;
                memcpy(&addition[query_number][sub_addition_length], domain, length);
                sub_addition_length += length;
                domain = strtok(NULL, ".");
            }
            unsigned char DomainEnd = (0x00);
            memcpy(&addition[query_number][sub_addition_length], &DomainEnd, sizeof(unsigned char));
            sub_addition_length++;

            unsigned short QueryType = htons(0x0001);
            memcpy(&addition[query_number][sub_addition_length], &QueryType, sizeof(unsigned short));
            sub_addition_length += 2;

            unsigned short QueryClass = htons(0x0001);
            memcpy(&addition[query_number][sub_addition_length], &QueryClass, sizeof(unsigned short));
            sub_addition_length += 2;

            unsigned int TTL = htonl(ttl);
            memcpy(&addition[query_number][sub_addition_length], &TTL, sizeof(unsigned int));
            sub_addition_length += 4;

            unsigned short data_length = 0;
            domain = strtok(result, ".");
            unsigned int data_length_locaiton = sub_addition_length;
            sub_addition_length += 2;
            data_length += 2;

            while(domain != NULL)
            {
                int IP = atoi(domain);
                unsigned char IP_char = IP;
                memcpy(&addition[query_number][sub_addition_length], &IP_char, sizeof(unsigned char));
                sub_addition_length ++;
                data_length ++;
                domain = strtok(NULL, ".");
            }

            data_length = htons(0x0004);
            memcpy(&addition[query_number][data_length_locaiton], &data_length, sizeof(unsigned short));
                
            total_addition_length = sub_addition_length;
        }
        addition_length[query_number] = total_addition_length;
        fclose(upop);
    }
}


void check_cache_ns_addition(char name[], char type[], int query_number)
{
    FILE* upop;
    int name_length = strlen(name);
    int type_length = strlen(type);
    char file_path[MAX_SIZE];
    file_path[0] ='.';
    file_path[1] ='/';
    file_path[2] ='N';
    file_path[3] ='S';
    file_path[4] ='/';
    int i = 0;
    for(i = 0; i < type_length; i++)
    {
        file_path[5 + i] = type[i];
    }

    file_path[5 + type_length] = '/';

    for(i = 0; i < name_length; i++)
    {
        file_path[5 + type_length + 1 + i] = name[i];
    }

    file_path[6 + type_length + name_length] = '\0';

    if ((upop = fopen(file_path, "r")) == NULL)  
    {     
        //answer[query_number][0] = '\0';
        addition[query_number][0] = '\0';
        fclose(upop);
        return;
    }
    else
    {
        char result[MAX_SIZE];
        int total_addition_length = 0;
       
        int sub_addition_length = addition_length[query_number];
        // Get all addition
        while(fscanf(upop,"%s",result) != EOF) 
        {
            unsigned int ttl = 0;
            char ttl_tmp[10];
            fscanf(upop,"%s", ttl_tmp);
            ttl = atoi(ttl_tmp);
               
            addition_count[query_number] ++;
            // construct addition section
                
            char *domain;
            domain = strtok(name, ".");
                    
            while(domain != NULL)
            {
                int length = strlen(domain);
                unsigned char domainLength = length;
                memcpy(&addition[query_number][sub_addition_length], &domainLength, sizeof(unsigned char));
                sub_addition_length += 1;
                memcpy(&addition[query_number][sub_addition_length], domain, length);
                sub_addition_length += length;
                domain = strtok(NULL, ".");
            }
            unsigned char DomainEnd = (0x00);
            memcpy(&addition[query_number][sub_addition_length], &DomainEnd, sizeof(unsigned char));
            sub_addition_length++;

            unsigned short QueryType = htons(0x0001);
            memcpy(&addition[query_number][sub_addition_length], &QueryType, sizeof(unsigned short));
            sub_addition_length += 2;

            unsigned short QueryClass = htons(0x0001);
            memcpy(&addition[query_number][sub_addition_length], &QueryClass, sizeof(unsigned short));
            sub_addition_length += 2;

            unsigned int TTL = htonl(ttl);
            memcpy(&addition[query_number][sub_addition_length], &TTL, sizeof(unsigned short));
            sub_addition_length += 4;

            unsigned short data_length = 0;
            domain = strtok(result, ".");
            unsigned int data_length_locaiton = sub_addition_length;
            sub_addition_length += 2;
            data_length += 2;

            while(domain != NULL)
            {
                int IP = atoi(domain);
                unsigned char IP_char = IP;
                memcpy(&addition[query_number][sub_addition_length], &IP_char, sizeof(unsigned char));
                sub_addition_length ++;
                data_length ++;
                domain = strtok(NULL, ".");
            }

            data_length = htons(0x0004);
            memcpy(&addition[query_number][data_length_locaiton], &data_length, sizeof(unsigned short));
                
            total_addition_length = sub_addition_length;
        }
        addition_length[query_number] = total_addition_length;
        fclose(upop);
    }
}

void processPointerName(unsigned char location)
{
    int totalLengthTmp = 0;
    while(1)
    {
        unsigned char domainLengthTmp = (0x00);
        memcpy(&domainLengthTmp, &recvBuff[location + totalLengthTmp], sizeof(unsigned char));
                
        int j = 0;
        unsigned char data = 0x00;
        for(j = 0; j < domainLengthTmp; j++){
            memcpy(&data, &recvBuff[location + totalLengthTmp + 1 + j], sizeof(unsigned char));
            //printf("%c", data);
        }
        
        totalLengthTmp += (1 + domainLengthTmp);
        memcpy(&domainLengthTmp, &recvBuff[location + totalLengthTmp], sizeof(unsigned char));
        if(domainLengthTmp == 0x00)
        {
            //printf("\n");
            break;
        }
        else if(domainLengthTmp  == 0xc0) // pointer mix
        {
            unsigned char newLocation = (0x00);
            memcpy(&newLocation, &recvBuff[location + totalLengthTmp + 1], sizeof(unsigned char));
            //printf(".");
            processPointerName(newLocation);
            break;
        }
        else
        {
            //printf(".");
        }
    }
}

