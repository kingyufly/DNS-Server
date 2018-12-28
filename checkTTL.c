#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>  

#define MAX_SIZE 512


pthread_t tid1;  
int rc1 = 0;  


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

int main()
{
    // DIR *dir;
    // struct dirent *ptr;
    // char folder[MAX_SIZE] = "./RR";
    // dir = opendir(folder);
    // while((ptr = readdir(dir)) != NULL)
    // {
    // 	if(strcmp(ptr->d_name, "..") == 0 || strcmp(ptr->d_name, ".") == 0)
    // 	    ;
    // 	else
    // 	{
    //         char sub_folder[MAX_SIZE] = {'\0'};
    //         strcpy(sub_folder, folder);
    //         sub_folder[strlen(folder)] = '/';
    //         int i = 0;
    //         for(i = 0; i < strlen(ptr->d_name); i++)
    //         {
    //             sub_folder[strlen(folder) + i + 1] = (ptr->d_name)[i];
    //         }
    //         sub_folder[strlen(sub_folder)] = '\0';

    //         DIR *sub_dir;
    //         struct dirent *sub_ptr;
    //         sub_dir = opendir(sub_folder);
    //         while((sub_ptr = readdir(sub_dir)) != NULL)
    //         {
    //             if(strcmp(sub_ptr->d_name, "..") == 0 || strcmp(sub_ptr->d_name, ".") == 0)
    //                 ;
    //             else
    //             {
    //                 char file_path[MAX_SIZE] = {'\0'};
    //                 strcpy(file_path, sub_folder);
    //                 file_path[strlen(sub_folder)] = '/';
    //                 int j = 0;
    //                 for(j = 0; j < strlen(sub_ptr->d_name); j++)
    //                 {
    //                     file_path[strlen(sub_folder) + j + 1] = (sub_ptr->d_name)[j];
    //                 }
    //                 file_path[strlen(file_path)] = '\0';

    //                 char new_file_path[MAX_SIZE] = {'\0'};
    //                 strcpy(new_file_path, file_path);

    //                 new_file_path[strlen(file_path)] = '1';
    //                 new_file_path[strlen(file_path) + 1] = '\0';

    //                 if(strcmp(ptr->d_name, "MX") == 0)
    //                 {
    //                     FILE* out;
    //                     FILE* in;
       
    //                     in = fopen(new_file_path, "w");
    //                     if ((out = fopen(file_path, "r")) == NULL)  
    //                     {     
    //                         fclose(out);
    //                     }
    //                     else
    //                     {
    //                         char tmp1[MAX_SIZE];
    //                         char ttl_tmp[MAX_SIZE];
    //                         char tmp2[MAX_SIZE];
                            
    //                         while(fscanf(out,"%s",tmp1) != EOF) 
    //                         {
    //                             unsigned int ttl = 0;
    //                             char ttl_tmp[10];
    //                             fscanf(out,"%s", ttl_tmp);
    //                             ttl = atoi(ttl_tmp);
    //                             ttl--;
    //                             fscanf(out,"%s",tmp2);
    //                             fprintf(in, "%s %d %s\n", tmp1, ttl, tmp2);
    //                         }

    //                         remove(file_path);
    //                         rename(new_file_path, file_path);

    //                         fclose(out);
    //                         fclose(in);
    //                     }
    //                 }
    //                 else
    //                 {
    //                     FILE* out;
    //                     FILE* in;
       
    //                     in = fopen(new_file_path, "w");
    //                     if ((out = fopen(file_path, "r")) == NULL)  
    //                     {     
    //                         fclose(out);
    //                     }
    //                     else
    //                     {
    //                         char tmp1[MAX_SIZE];
    //                         char ttl_tmp[MAX_SIZE];
                            
    //                         while(fscanf(out,"%s",tmp1) != EOF) 
    //                         {
    //                             unsigned int ttl = 0;
    //                             char ttl_tmp[10];
    //                             fscanf(out,"%s", ttl_tmp);
    //                             ttl = atoi(ttl_tmp);
    //                             ttl--;
    //                             fprintf(in, "%s %d\n", tmp1, ttl);
    //                         }

    //                         remove(file_path);
    //                         rename(new_file_path, file_path);

    //                         fclose(out);
    //                         fclose(in);
    //                     }
    //                 }
    //             }
    //         }
    //         closedir(sub_dir);
    //     }
    // }
    // closedir(dir);


     rc1 = pthread_create(&tid1, NULL, thread1, &tid1);  
            if(rc1 != 0)  
                printf("Thread create error!");  
    while(1)
        ;

}

