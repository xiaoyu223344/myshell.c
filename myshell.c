#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>   
#include <sys/stat.h>    
#include <fcntl.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX_BUFFER 1024    // max line buffer
#define MAX_ARGS 64        // max # args
#define SEPARATORS " \t\n" // token sparators
#define README "readme"    // help file name
#define CLEAR "\e[1;1H\e[2J"

struct shellstatus_st
{
    int foreground;  // foreground execution flag
    char *infile;    // input redirection flag & file
    char *outfile;   // output redirection flag & file
    char *outmode;   // output redirection mode
    char *shellpath; // full pathname of shell
};
typedef struct shellstatus_st shellstatus;

extern char **environ;
char *prompt = "==>"; 

void check4redirection(char **, shellstatus *); // check command line for i/o redirection
void errmsg(char *, char *);                    // error message printout
void execute(char **, shellstatus);             // execute command from arg array
char *getcwdstr(char *, int);                   // get current work directory string
FILE *redirected_op(shellstatus *);               // return required o/p stream
char *stripath(char *);                         // strip path from filename
void syserrmsg(char *, char *);                 // system error message printout

/*******************************************************************/

int main(int argc, char **argv)
{
    FILE *ostream = stdout;   // (redirected) o/p stream
    FILE *instream = stdin;   // batch/keyboard input
    char linebuf[MAX_BUFFER]; // line buffer
    char cwdbuf[MAX_BUFFER];  // cwd buffer
    char *args[MAX_ARGS];     // pointers to arg strings
    char **arg;               // working pointer thru args
    char *readmepath;         // readme pathname
    shellstatus status;       // status structure

    /*FILE *fp;
    fp = fopen("/home/xiaoyu/lab/p1/result.txt","r");
    if(fp==NULL){
        exit(0);
    }*/

    // parse command line for batch input
    switch (argc)
    {
    case 1:
    {
        // keyboard input
        // TODO
        instream=stdin;
        break;
    }

    case 2:
    {
        // possible batch/script
        // TODO
        if(!(instream=fopen(argv[1],"r"))){
            syserrmsg("","Can not open the bash file");
            exit(0);
        }
        break;
    }
    default: // too many arguments
        fprintf(stderr, "%s command line error; max args exceeded\n"
                        "usage: %s [<scriptfile>]",
                stripath(argv[0]), stripath(argv[0]));
        exit(1);
    }

    // get starting cwd to add to readme pathname
    // TODO
    getcwd(cwdbuf,MAX_BUFFER);
    strcat(cwdbuf,"/readme");
    readmepath=(char*)malloc(sizeof(cwdbuf));
    strcpy(readmepath,cwdbuf);

    // get starting cwd to add to shell pathname
    // TODO
    getcwd(cwdbuf,MAX_BUFFER);
    strcat(cwdbuf,"/myshell");
    status.shellpath=(char*)malloc(sizeof(cwdbuf));
    strcpy(status.shellpath,cwdbuf);

    // set SHELL= environment variable, malloc and store in environment
    // TODO
    setenv("SHELL",status.shellpath,1);

    // prevent ctrl-C and zombie children
    signal(SIGINT, SIG_IGN);  // prevent ^C interrupt
    signal(SIGCHLD, SIG_IGN); // prevent Zombie children

    // keep reading input until "quit" command or eof of redirected input
    while (!feof(instream))
    {
        // (re)initialise status structure
        status.foreground = TRUE;

        // set up prompt
        // TODO

        if (argc==1)
        {
            /* code */
            getcwdstr(cwdbuf,MAX_BUFFER);
            printf("%s",cwdbuf);
        }
        

    

        // get command line from input
        if (fgets(linebuf, MAX_BUFFER, instream))
        {
            // read a line
            // tokenize the input into args array
            arg = args;
            *arg++ = strtok(linebuf, SEPARATORS); // tokenize input
            while ((*arg++ = strtok(NULL, SEPARATORS)))
                ;
            int len=0 ;
            while (args[len]&&strcmp(args[len],"<")&&strcmp(args[len],">")&&strcmp(args[len],">>")&&strcmp(args[len],"&"))
            {
                len++;
            }

            // last entry will be NULL
            if (args[0])
            {
                // check for i/o redirection
                check4redirection(args, &status);

                // check for internal/external commands
                // "cd" command
                if (!strcmp(args[0], "cd"))
                {
                    // TODO
                    ostream=redirected_op(&status);
                    char oldcwd[MAX_BUFFER];
                    getcwd(oldcwd,MAX_BUFFER);
                    switch (len)
                    {
                    case 1: fprintf(ostream,"%s\n",oldcwd);
                            break;
                    case 2: if (!chdir(args[1]))
                            {
                                getcwd(cwdbuf,MAX_BUFFER);
                                //setenv("PWD",cwdbuf,1);
                            }
                            else{
                                syserrmsg("CD","");
                            }
                            break;
                    default:
                        break;
                    }
                    //fclose(ostream);


                }
                // "clr" command
                else if (!strcmp(args[0], "clr"))
                {
                    // TODO
                    ostream=redirected_op(&status);
                    fprintf(ostream,CLEAR);
                    //fclose(ostream);
                }
                // "dir" command
                else if (!strcmp(args[0], "dir"))
                {
                    // TODO
                    DIR *dir;
                    struct dirent * ptr;
                    char path[MAX_BUFFER];
                    getcwd(path,MAX_BUFFER);
                    ostream=redirected_op(&status);
                    if (len>2)
                    {
                        for(int i = 1;i < len;i++){
                            strcpy(path,args[i]);
                            dir = opendir(path);
                            fprintf(ostream,"%s",path);
                            while((ptr = readdir(dir)) != NULL){
                                if(strcmp(ptr->d_name,".") && strcmp(ptr->d_name,".."))
                                    fprintf(ostream,"%s ",ptr->d_name);
                            }
                        }
                        closedir(dir);
                    }
                    else{
                        if (len==2)
                        {
                            strcpy(path,args[1]);
                        }
                        dir = opendir(path);
                        while((ptr = readdir(dir)) != NULL){
                            if(strcmp(ptr->d_name,".") && strcmp(ptr->d_name,".."))
                            fprintf(ostream,"%s",ptr->d_name);
                            fprintf(ostream,"%s"," ");
                        }
                        fprintf(ostream,"%s","\n");
                        closedir(dir);    
                        
                    }
                    //fclose(ostream);               

                }
                // "echo" command
                else if (!strcmp(args[0], "echo"))
                {
                    // TODO
                    ostream=redirected_op(&status);
                    for (int i = 1; args[i]; i++)
                    {
                        fprintf(ostream,"%s ",args[i]);
                    }
                    fprintf(ostream,"\n");
                    //fclose(ostream);
                    
                }
                // "environ" command
                else if (!strcmp(args[0], "environ"))
                {

                    // TODO
                    ostream=redirected_op(&status);
                    for(int i=0;environ[i]!=NULL;i++)
                        fprintf(ostream,"%s \n",environ[i]);
                    //fclose(ostream);
                }
                // "help" command
                else if (!strcmp(args[0], "help"))
                {
                    args[0] = "more";
                    args[1] = readmepath;
                    args[2] = NULL;
                    execute(args,status);
                }
                // "pause" command - note use of getpass - this is a made to measure way of turning off
                //  keyboard echo and returning when enter/return is pressed
                else if (!strcmp(args[0], "pause"))
                {
                    // TODO
                    getpass("Press enter to continue");
                }
                // "quit" command
                else if (!strcmp(args[0], "quit"))
                {
                    break;
                }
                // else pass command on to OS shell
                // TODO
                else{
                    execute(args,status);
                }

            }
        }
    }
    return 0;
}

/***********************************************************************

void check4redirection(char ** args, shellstatus *sstatus);

check command line args for i/o redirection & background symbols
set flags etc in *sstatus as appropriate

***********************************************************************/

void check4redirection(char **args, shellstatus *sstatus)
{
    sstatus->infile = NULL; // set defaults
    sstatus->outfile = NULL;
    sstatus->outmode = NULL;

    while (*args)
    {
        // input redirection
        if (!strcmp(*args, "<"))
        {
            // TODO
            sstatus->infile=*++args;
            printf("%s\n",sstatus->infile);
        }
        // output direction
        else if (!strcmp(*args, ">") || !strcmp(*args, ">>"))
        {
            // TODO
            if(!strcmp(*args, ">")){
                sstatus->outmode="w";
            }
            else{
                sstatus->outmode="a";
            }
            sstatus->outfile=*++args;
        }
        else if (!strcmp(*args, "&"))
        {
            // TODO
            sstatus->foreground=FALSE;
        }
        args++;
    }
}

/***********************************************************************

  void execute(char ** args, shellstatus sstatus);

  fork and exec the program and command line arguments in args
  if foreground flag is TRUE, wait until pgm completes before
    returning

***********************************************************************/

void execute(char **args, shellstatus sstatus)
{
    int status;
    pid_t child_pid;
    char tempbuff[MAX_BUFFER];
    // char tempbuf[MAX_BUFFER];

    switch (child_pid = fork())
    {
    case -1:
        syserrmsg("fork", NULL);
        break;
    case 0:
        // execution in child process
        // reset ctrl-C and child process signal trap
        signal(SIGINT, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        // i/o redirection */
        // TODO
        if (sstatus.infile)
        {
            freopen(sstatus.infile,"r",stdin);
        }
        if (sstatus.outfile)
        {
            freopen(sstatus.outfile,sstatus.outmode,stdout);
        }
        if ((strcmp(args[0],"sort"))==0)
        {
            for (int i=1;args[i];i++)
            {
                /* code */
                args[i]=args[i+1];
            }
            for (int i = 0; args[i] ; i++)
            {
                /* code */
                if ((strcmp(args[i],">"))==0)
                {
                    /* code */
                    for (int j = i; args[j] ; j++)
                    {
                        /* code */
                        args[j]=args[j+1];
                    }
                    break;
                }
                
            }
            
            
        }
        
        // set PARENT = environment variable, malloc and put in nenvironment
        // TODO
        getcwd(tempbuff,MAX_BUFFER);
        strcat(tempbuff,"/myshell");
        sstatus.shellpath=(char*)malloc(sizeof(tempbuff));
        strcpy(sstatus.shellpath,tempbuff);
        setenv("PARENT",sstatus.shellpath,1);

        // final exec of program
        execvp(args[0], args);
        syserrmsg("exec failed -", args[0]);
        exit(127);
    }

    // continued execution in parent process
    // TODO
    if(sstatus.foreground){
        waitpid(child_pid, &status, 0);
    if (WIFSIGNALED(status))
        printf("Child terminated abnormally, signal %d\n", WTERMSIG(status));
    }
    else{
        waitpid(child_pid,&status,WNOHANG);
    }
    
}

/***********************************************************************

 char * getcwdstr(char * buffer, int size);

return start of buffer containing current working directory pathname

***********************************************************************/

char *getcwdstr(char *buffer, int size)
{
    // TODO
    getcwd(buffer,MAX_BUFFER);
    strcat(buffer,"==>");
    return buffer;
}

/***********************************************************************

FILE * redirected_op(shellstatus ststus)

  return o/p stream (redirected if necessary)

***********************************************************************/

FILE *redirected_op(shellstatus *status)
{
    FILE *ostream = stdout;
    // TODO
    if(status->outfile){
            //printf("%s\n",status.outfile);
            ostream=fopen(status->outfile,status->outmode);
            if (ostream==NULL)
            {
                syserrmsg("","Can not open the redirected file");
                exit(0);

            }
            
    }
    return ostream;
}

/*******************************************************************

  char * stripath(char * pathname);

  strip path from file name

  pathname - file name, with or without leading path

  returns pointer to file name part of pathname
            if NULL or pathname is a directory ending in a '/'
                returns NULL
*******************************************************************/

char *stripath(char *pathname)
{
    char *filename = pathname;

    if (filename && *filename)
    {                                      // non-zero length string
        filename = strrchr(filename, '/'); // look for last '/'
        if (filename)                      // found it
            if (*(++filename))             //  AND file name exists
                return filename;
            else
                return NULL;
        else
            return pathname; // no '/' but non-zero length string
    }                        // original must be file name only
    return NULL;
}

/********************************************************************

void errmsg(char * msg1, char * msg2);

print an error message (or two) on stderr

if msg2 is NULL only msg1 is printed
if msg1 is NULL only "ERROR: " is printed
*******************************************************************/

void errmsg(char *msg1, char *msg2)
{
    fprintf(stderr, "ERROR: ");
    if (msg1)
        fprintf(stderr, "%s; ", msg1);
    if (msg2)
        fprintf(stderr, "%s; ", msg2);
    return;
    fprintf(stderr, "\n");
}

/********************************************************************

  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char *msg1, char *msg2)
{
    errmsg(msg1, msg2);
    perror(NULL);
    return;
}
