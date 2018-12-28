#include "dir.h"
#include <stdio.h>
#include "io.h"
#include <string.h>

int main(int argc, char* argv[])
{
    char path[100] = "C:/";
    
    struct _finddata_t fa;
    long handle;
    
    if((handle = _findfirst(strcat(path,"*"),&fa)) == -1L)
    {
        printf("The Path %s is wrong!\n",path);
        return 0;
    }
    
    do
    {
        if( fa.attrib == _A_SUBDIR && ~strcmp(fa.name,".")&& ~strcmp(fa.name,".."))
            printf("The subdirectory is %s\n",fa.name);
    }while(_findnext(handle,&fa) == 0); /* 成功找到时返回0*/ 
    
    _findclose(handle);
    
    return 0;    
}