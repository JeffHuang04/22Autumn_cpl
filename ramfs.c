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
        DIR_NODE = 2
    } type;
    struct node *child;
    struct node *sibling;
    void *contnet;
    int size;
    char *shortname;
} node;
node *root;
typedef struct filedesc {
    bool use;
    int offset;
    int flags;
    int writable;
    int readable;
    node *file;
} filedesc;
filedesc filed[max_fd];

int pathname_simple(char **str, char *temp_string, const char *pathname) {
    int lengthofroad = strlen(pathname);
    if (*pathname == '/' && lengthofroad <= length_road) {
        int index = 0;
        char *temp = malloc(length_name + 3);
        temp = strtok(temp_string, "/");
        while (temp != NULL) {
            if (strlen(temp) > length_name) {
                free(temp);
                str = NULL;
                return 0;
            }
            str[index] = temp;
            temp = strtok(NULL, "/");
            index++;
        }
        if(index >= 2) {
            for (int i = 0; i <= index - 2 ; i++) {
                int count_ = strlen((str[i]));
                if(count_ >= 4
                   && str[i][count_ - 1] == 't'
                   && str[i][count_ - 2] == 'x'
                   && str[i][count_ - 3] == 't'
                   && str[i][count_ - 4] == '.'){
                    free(temp);
                    return 0;
                }
            }
        }
        int count = strlen(str[index - 1]);
        if (pathname[lengthofroad - 1] == '/' && count >= 4
            && str[index - 1][count - 1] == 't'
            && str[index - 1][count - 2] == 'x'
            && str[index - 1][count - 3] == 't' && str[index - 1][count - 4] == '.') {
            free(temp);
            str = NULL;
            return 0;
        }
        free(temp);
        return index;
    } else {
        str = NULL;
        return 0;
    }
}

int ropen(const char *pathname, int flags) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    int index = pathname_simple(str, temp_string, pathname);
    if (index == 0) {
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
    int index = pathname_simple(str, temp_string, pathname);
    if (/*str == NULL*/index == 0) {
        return -1;
    }
    int count = strlen(str[index - 1]);
    if (count >= 4 && str[index - 1][count - 1] == 't' && str[index - 1][count - 2] == 'x'
        && str[index - 1][count - 3] == 't' && str[index - 1][count - 4] == '.') {
        return -1;
    }//判断最后类型是目录
    node *instruction = malloc(sizeof(struct node) + 5);
    instruction = root->child;//从头遍历链表
    for (int i = 0; i <= index - 1; i++) {
        if (i == index - 1) {
            for (;;) {
                if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                    if (strcmp(str[i], instruction->shortname) == 0) {
                        return -1;
                    }
                }
                if (instruction->sibling == NULL) {//（之后循环退出的条件）
                    break;
                }
                instruction = instruction->sibling;
            }//检查有没有重名的
            if (instruction->shortname != NULL) {//判断链表自身不为空并引入新节点
                instruction->sibling = malloc(sizeof(struct node));
                instruction->sibling->sibling = NULL;
                instruction->sibling->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->sibling->shortname, str[i]);
                instruction->sibling->type = DIR_NODE;
                return 0;
            } else {//判断链表自身为空
                instruction->sibling = NULL;
                instruction->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->shortname, str[i]);
                instruction->type = DIR_NODE;
                return 0;
            }
        }
        if (instruction->shortname == NULL) {
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0) {
                if (instruction->child == NULL) {
                    instruction->child = malloc(sizeof(struct node));
                }
                instruction = instruction->child;
                break;
            }
            if (instruction->sibling == NULL) {
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
    free(instruction);
    free(temp_string);
    free(str);
}

int rrmdir(const char *pathname) {

    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    int index = pathname_simple(str, temp_string, pathname);
    if (index == 0) {
        return -1;
    }
    node *instruction = malloc(sizeof(struct node) + 5);
    instruction = root->child;
    for (int i = 0; i <= index - 1; i++) {

    }

    free(temp_string);
    free(str);
}

int runlink(const char *pathname) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    int index = pathname_simple(str, temp_string, pathname);
    if (index == 0) {
        return -1;
    }
    // todo

    free(temp_string);
    free(str);
}

void init_ramfs() {
    root = malloc(sizeof(struct node));
    root->type = DIR_NODE;
    root->shortname = "/";
    root->sibling = NULL;
    root->child = malloc(sizeof(struct node));
}
