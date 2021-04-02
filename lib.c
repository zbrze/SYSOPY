#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<string.h>

void create_chidren(int children_num){
    for(int i = 0; i < children_num; i++){
        pid_t child = fork();
        if(child == 0){
            printf("Proces rodzica ma pid:%d Proces %d dziecka ma pid:%d\n\n\n",(int)getppid(), i, (int)getpid());
            exit(0);
        }
    }
    for(int i=0;i< children_num ;i++) wait(NULL); 
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Podano niewlasciwa liczbe argumentow");
        return 1;
    }
    printf("PID glownego programu: %d\n\n", (int)getpid());
    int children_num = atol(argv[1]);
    create_chidren(children_num);

}