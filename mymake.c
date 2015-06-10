#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "test.h"

#define NONE_FILE 0
#define C_FILE 1
#define CPP_FILE 2

char target[256] = "a.out";
char includes[512] = {'\0'};
char libs[256] = {'\0'};//如果检测到与参数传入时再初始化，是不是可以避免浪费空间
char* sources = NULL;

char* templeTar="####### options\n\n"
"TARGET        = ";

char* templeInc="\n\n"
"CC            = gcc\n"
"CFLAGS        = -g -Wall\n\n"
 "INCPATH       = ";

char* templeLib="\n\n"
"LIBS          = ";

char* templeSrc="\n\n\n"
"####### Files\n\n"
"SOURCES       = ";

char* templeLeft="\n\n"
"OBJECTS       = $(SOURCES:.c=.o)\n\n\n"
"####### Build rules\n\n"
"all: Makefile $(TARGET)\n\n"
"$(TARGET):  $(OBJECTS)\n"
"\t$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)\n\n"
"OUTPUT_OPTION = -o $@\n\n"
"COMPILE.c = $(CC) $(CFLAGS) $(INCPATH) -c\n\n"
"%.o: %.c\n"
"\t$(COMPILE.c) $(OUTPUT_OPTION) $<\n\n\n"
"include $(SOURCES:.c=.d)\n\n"
"%.d: %.c\n"
"\tset -e; rm -f $@; \\\n"
"\t$(CC) -MM $(CFLAGS) $< > $@.$$$$; \\\n"
"\tsed 's,\\($*\\)\\.o[ :]*,\\1.o $@ : ,g' < $@.$$$$ > $@; \\\n"
"\trm -f $@.$$$$\n\n"
"clean:\n"
"\t-rm ./$(TARGET) ./*.o ./*.d\n"
     ".PHONY: clean\n";

static char* strrev(char* str);
static int isSource(char* filename);
static void scanSource(void);
static void genFile(void);
static void readArg(int argc, char* argv[]);

int main(int argc, char *argv[])
{
     if (argc > 1)
          readArg(argc, argv);
     scanSource();
     genFile();
     return 0;
}

int isSource(char* filename)
{
     if (strncmp(".",filename,1) == 0)//忽略隐藏文件
     return NONE_FILE;
     
     char* tmp = strrev(filename);
     
     if (strncasecmp("c.",tmp,2) == 0){
          free(tmp);
          return C_FILE;
     }
     
     if (strncasecmp("ppc.",tmp,4) == 0){
          free(tmp);
          return CPP_FILE;
     }
     
     free(tmp);
     return NONE_FILE;
}

char* strrev(char* str)
{
     if (!str) return "";
     int len = strlen(str);
     
     char* tmp = (char*)malloc(len+1);
     memcpy(tmp,str,len+1);
     
     int t = !(len%2)? 1:0;
     int i,j;
     
     for (i=len-1, j=0; i>(len/2-t); i--){
          char ch = tmp[i];
          tmp[i]=tmp[j];
          tmp[j++]=ch;
     }
     return tmp;
}

void scanSource(void)
{
     
     DIR* dp;
     struct dirent *dirp;
     int totallen = 0;
     

     if ( (dp=opendir(".")) == NULL)
          perror("opendir");
     while ( (dirp=readdir(dp)) != NULL)
          if (isSource(dirp->d_name) == C_FILE)
               totallen += (strlen(dirp->d_name)+10);//先遍历一遍获得总长度,10是预留给其他字符比如" \"的长度               
     closedir(dp);

     if (totallen == 0){
          printf("no source files found!\n");
          exit(1);
     }
     sources=(char*)malloc(totallen);
     memset(sources,0,totallen);
     
     if ( (dp=opendir(".")) == NULL)
          perror("opendir");
     
     while ( (dirp=readdir(dp)) != NULL && !isSource(dirp->d_name));
     sprintf(sources,"%s",dirp->d_name);//
     
     while ( (dirp=readdir(dp)) != NULL)
          if (isSource(dirp->d_name))
               sprintf(sources+strlen(sources)," \\\n\t\t%s",dirp->d_name);
               
     printf("%s\n",sources);
     
}

void genFile(void)
{
     FILE* fp;
     if ( (fp=fopen("Makefile","w")) == NULL)
          perror("fopen");

     fwrite(templeTar,strlen(templeTar),1,fp);
     fwrite(target,strlen(target),1,fp);
     
     fwrite(templeInc,strlen(templeInc),1,fp);
     fwrite(includes,strlen(includes),1,fp);
     
     fwrite(templeLib,strlen(templeLib),1,fp);
     fwrite(libs,strlen(libs),1,fp);

     fwrite(templeSrc,strlen(templeSrc),1,fp);
     fwrite(sources,strlen(sources),1,fp);
         
     fwrite(templeLeft,strlen(templeLeft),1,fp);
       
     
}

void readArg(int argc, char* argv[])
{
     char* str;
     char c;
     int i;
     for (i=1; i<argc; i++){
          str = argv[i];
          if (str[0]!='-' && str[0]!='/')
               continue;
          c = str[1];
          if (c == 'o'){
               memcpy(target,str+2,strlen(str)-2);
          }
          if (c == 'l'){
               sprintf(libs+strlen(libs),"%s ",str);
          }
          if (c == 'I'){
               sprintf(includes+strlen(includes),"%s ",str);
          }
     }
}
