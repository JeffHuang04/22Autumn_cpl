#include "ramfs.h"
/* modify this file freely */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define max_fd 4096
#define max_fileanddir 65536
#define length_name 32
#define length_road 1024
typedef struct node {
    enum {
        FILE_NODE = 1,
        DIR_NODE  = 2
    }type;
    struct node *subentries;
    struct node *sibling;
    void *contnet;
    int number_subentries;
    int size;
    char *name;
}node;
node root;
typedef struct filedesc{
    bool use;
    int offset;
    int flags;
    int writable;
    int readable;
    node *file;
}filedesc;
filedesc filed[max_fd];
void pathname_simple(char **str, char *temp_string,const char *pathname) {
    int lengthofroad = strlen(pathname);
    if(*pathname == '/' && lengthofroad <= length_road){
        char *temp = malloc(length_name + 3);
        temp = strtok(temp_string, "/");
        int index = 0;
        while (temp != NULL){
            if(strlen(temp) > length_name) {
                free(temp);
                str = NULL;
                return;
            }
            str[index] = temp;
            temp = strtok(NULL,"/");
            index ++;
        }
        if(pathname[lengthofroad - 1] == '/' && (strtok(str[index - 1],".text") !=NULL)) {
            free(temp);
            str = NULL;
            return;
        }
        free(temp);
        return;
    } else {
        str = NULL;
        return;
    }
}
int ropen(const char *pathname, int flags) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    pathname_simple(str, temp_string,pathname);
    if(str == NULL) {
        return -1;
    }
    // todo

    free(temp_string);
    free(str);
}


int rclose(int fd) {
  // TODO();
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
  // TODO();
}

ssize_t rread(int fd, void *buf, size_t count) {
  // TODO();
}

off_t rseek(int fd, off_t offset, int whence) {
  // TODO();
}

int rmkdir(const char *pathname) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    pathname_simple(str, temp_string,pathname);
    if(str == NULL) {
        return -1;
    }
    // todo

    free(temp_string);
    free(str);
}

int rrmdir(const char *pathname) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    pathname_simple(str, temp_string,pathname);
    if(str == NULL) {
        return -1;
    }
    // todo

    free(temp_string);
    free(str);
}

int runlink(const char *pathname) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    pathname_simple(str, temp_string,pathname);
    if(str == NULL) {
        return -1;
    }
    // todo

    free(temp_string);
    free(str);
}

void init_ramfs() {

}
