/*Napisz program, który rozpoczynając od katalogu podanego jako pierwszy parametr uruchomienia, idąc w głąb drzewa katalogów, 
znajdzie pliki zawierające łańcuch podany jako drugi parametr uruchomienia programu. Przeszukiwanie każdego z podkatalogów
powinno odbyć się w osobnym procesie potomnym. Wydruk wyniku wyszukiwania poprzedź wypisaniem ścieżki względnej od katalogu
podanego jako argument uruchomienia oraz numeru PID procesu odpowiedzialnego za przeglądanie określonego (pod)katalogu.
Przeszukiwanie powinno obejmować pliki tekstowe i pomijać pliki binarne/wykonywalne/obiektowe etc. Program jako trzeci parametr powinien
przyjmować maksymalną głębokość przeszukiwania licząc od katalogu podanego jako pierwszy parametr.*/
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <dirent.h>
#define __USE_XOPEN_EXTENDED 1

char* get_ext(const char* file){
    const char *dot = strrchr(file, '.');
    if(!dot || dot == file) return "";
    return (char *)dot + 1;
}

int check_if_text(char* extension){
    if(!strcmp(extension, "txt") || !strcmp(extension, "c") || !strcmp(extension, "h") || !strcmp(extension, "py")) return 1;
    return 0;
}

int find_str_in_file(char* file_path, char *str_to_find){
    FILE *f = fopen(file_path, "r");
    if(f == NULL) return 0;
     char c;
    char *buff = calloc(1, sizeof(char));
    size_t len = 0;
    while(fread(&c, sizeof(char), 1, f) == 1){

        size_t new_len = len + 1;
        char* new_buff = realloc(buff, new_len);
        buff = new_buff;
        buff[len] = c;
        len++;
    }
    if(strcasestr(buff, str_to_find) != NULL) return 1;
    return 0;
    
}

char* create_new_path(char* curr_path, char* new_file){
    char* new_path = malloc(256 * sizeof(char));
    strcpy(new_path, curr_path);
    strcat(new_path, "/");
    strcat(new_path, new_file);
    return new_path;
}
void find_dir(char* path, int depth, int max_depth, char* str_to_find){
    if(depth >= max_depth) return;
    if(path == NULL) return;
    DIR * dir = opendir(path);
    if(dir == NULL) return;
    struct stat buffer;
    lstat(path, &buffer);
     if (S_ISDIR(buffer.st_mode)){
         struct dirent *files_in_dir;
         while((files_in_dir = readdir(dir)) != NULL){
            if(strcmp(files_in_dir->d_name, ".") != 0 && strcmp(files_in_dir->d_name, "..") != 0){
                char* new_path = create_new_path(path, files_in_dir -> d_name);
                lstat(new_path, &buffer);
                if(S_ISDIR(buffer.st_mode)){
                    
                    printf("\nJestem procesem %d w katalogu %s\n", getpid(), new_path);
                     if(fork() == 0) return find_dir(new_path, ++depth, max_depth, str_to_find);
                }
                else{
                    if(check_if_text(get_ext(new_path)))
                        if(find_str_in_file(new_path, str_to_find)) printf("\nZnaleziono string w %s %d\n", new_path, getpid());  
                }
            }
        }
     }
}
int main(int argc, char* argv[]) {

    if(argc != 4) {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }
    char* dir = argv[1];
    char* find_str = argv[2];
    int max_depth = atoi(argv[3]);
    find_dir(dir, 0, max_depth, find_str);

}


