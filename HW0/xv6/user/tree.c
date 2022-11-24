#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"
int dir_count,file_count;
char* fmtname(char *path){
    static char buf[DIRSIZ+1];
    char *p;
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), '\0', 1);
    //memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

int dfstree(char *curdir,int level,int lastflag,char *route,char *pastestr){
    char buf[128], *p;
    int fd_root;
    struct dirent de;
    struct stat st;
    if((fd_root = open(route, 0)) < 0){
        fprintf(1, "%s [error opening dir]\n", curdir);
        return 0;
    }
    if(fstat(fd_root, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", curdir);
        close(fd_root);
        return 0;
    }
    //main code
    int dirflag = 0;
    int undercount = 0; //contain . & ..
    switch(st.type){
        case T_FILE:
            fprintf(1, "%s [error opening dir]\n", curdir);
            break;
        case T_DIR:
            if (level == 0) printf("%s\n",curdir); 
            if(strlen(route) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, route);
            p = buf+strlen(buf);
            *p++ = '/';
            while (read(fd_root, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                undercount++;
            }
            close(fd_root);
            dirflag = 1;
            break;
    }
    if (dirflag == 1){
        fd_root = open(route, 0);
        int localcount = 0;
        while (read(fd_root, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            char next[128];
            switch(st.type){
                case T_FILE:
                    localcount++;
                    file_count++;
                    if (level != 0)printf("%s",pastestr);
                    printf("|\n");
                    if (level != 0)printf("%s",pastestr);
                    printf("+-- %s\n",fmtname(buf));
                    break;
                case T_DIR:
                    localcount++;
                    char new[128],*stamp;
                    if (localcount > 2){
                        dir_count++;
                        if (level != 0)printf("%s",pastestr);
                        printf("|\n");
                        if (level != 0)printf("%s",pastestr);
                        //printf("+-- %s\n",new/*fmtname(buf)*/);
                    } 
                    strcpy(new,fmtname(buf));
                    if (localcount > 2) {printf("+-- ");/*printf("\nnew start:\n");*/printf("%s\n",new);/*printf("\nnew end:\n");*/}
                    strcpy(next,route);
                    int len = strlen(next);
                    next[len] = '/';
                    next[len+1] ='\0';
                    stamp = next;
                    stamp += strlen(next);
                    char *deal = new;
                    deal += strlen(new);deal = '\0';
                    //char *printing = new;
                    strcpy(stamp,new);  
                    //printf("next:%s",next);   
                    if (localcount > 2){
                        char nextpaste[128];
                        strcpy(nextpaste,pastestr);
                        char *pasting = nextpaste;
                        pasting += strlen(nextpaste);
                        //printf("%s\n",nextpaste);
                        if (localcount == undercount){
                            char *cool; cool = malloc(sizeof(char)*5);
                            cool[0] =' ';cool[1] =' ';cool[2] =' ';cool[3] =' ';cool[4] ='\0';
                            strcpy(pasting,cool);
                            dfstree(new,level+1,1,next,nextpaste);
                        }     
                        else{
                            char *cool; cool = malloc(sizeof(char)*5);
                            cool[0] ='|';cool[1] =' ';cool[2] =' ';cool[3] =' ';cool[4] ='\0';
                            strcpy(pasting,cool);
                            dfstree(new,level+1,0,next,nextpaste); 
                        }            
                    }  
                    break;
            }    
        }
        close(fd_root);
    }
    return 0;
}
int maintree(char *rootdir){
    int pipefd[2];
    pipe(pipefd);
    int pid = fork();
    int *read_fd = &pipefd[0];
    int *write_fd = &pipefd[1];
    if (pid < 0){
        fprintf(2,"Tree: fork error\n");
        return 0;
    }
    else if (pid == 0){ // child
        close(*read_fd);
        dir_count = 0; file_count = 0;
        char newthing[128]; 
        dfstree(rootdir,0,0,rootdir,newthing); 
        write(*write_fd,&dir_count,sizeof(int));
        write(*write_fd,&file_count,sizeof(int));
        exit(0);
    }
    else { //parent
        close(*write_fd);
        wait(0);
        int dircountla = 0, filecountla = 0;
        read(*read_fd,&dircountla,sizeof(int));
        read(*read_fd,&filecountla,sizeof(int));
        printf("\n");
        printf("%d directories, %d files\n",dircountla,filecountla);
    }
    return 0;
}
int main(int argc, char *argv[]) {
    if (argc != 2){
        fprintf(2,"Input does not contain one root dir.\n");
        exit(0);
    }
    maintree(argv[1]);
    exit(0);
}