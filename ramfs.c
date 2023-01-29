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
    enum {
        file = 1,
        dir = 2,
    }type;
    int offset;
    int flags;
    int writable;
    int readable;
    node *fileordir;
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
    node *instruction = root->child;
    if((flags & 0b0001000000) == 0 ) {//判断不用创建文件
        for (int i = 0; i <= index - 1; i++) {
            if (i == index - 1) {
                if (instruction == NULL) {//最低层目录为空
                    free(temp_string);
                    free(str);
                    return -1;
                }
                for (;;) {
                    if (strcmp(instruction->shortname, str[i])) {
                        break;
                    }
                    if (instruction->sibling == NULL) {
                        free(temp_string);
                        free(str);
                        return -1;
                    }
                    instruction = instruction->sibling;
                }
            }
            if (instruction->shortname == NULL) {//排除直接查找跨级目录即这级目录为空
                free(temp_string);
                free(str);
                return -1;
            }
            for (;;) {
                if (strcmp(str[i], instruction->shortname) == 0) {
                    instruction = instruction->child;
                    break;
                }
                if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                    free(temp_string);
                    free(str);
                    return -1;
                }
                instruction = instruction->sibling;
            }
        }
    } else {//判断要创建文件（与创建目录大同小异）
        int count = strlen(str[index - 1]);
        if (count >= 4 && str[index - 1][count - 1] == 't' && str[index - 1][count - 2] == 'x'
            && str[index - 1][count - 3] == 't' && str[index - 1][count - 4] == '.') {
            for (int i = 0; i <= index - 1; i++) {
                if (i == index - 1) {
                    for (;;) {
                        if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                            if (strcmp(str[i], instruction->shortname) == 0) {
                                free(temp_string);
                                free(str);
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
                        instruction->sibling->type = FILE_NODE;
                    } else {//判断链表自身为空并引入新节点
                        instruction->sibling = NULL;
                        instruction->shortname = malloc(strlen(str[i]) + 1);
                        strcpy(instruction->shortname, str[i]);
                        instruction->type = FILE_NODE;
                    }
                }
                if (instruction->shortname == NULL) {//排除直接建立跨级目录即这级目录为空
                    free(temp_string);
                    free(str);
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
                    if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                        free(temp_string);
                        free(str);
                        return -1;
                    }
                    instruction = instruction->sibling;
                }
            }
        } else {
            free(temp_string);
            free(str);
            return -1;
        }
    }
    int index_fd = 0;
    for (int i = 0; i <= max_fd - 1; i++) {
        if(filed[i].use == false){
            index_fd = i;
            filed[i].use == true;
            break;
        }
    }
    if(instruction->type == DIR_NODE) {
        filed[index_fd].fileordir = instruction;
        filed[index_fd].type = dir;
    } else if(instruction->type == FILE_NODE) {
        filed[index_fd].flags = flags;
        filed[index_fd].type = file;
        filed[index_fd].fileordir = instruction;
        if((flags & 0b10000000000) == 0) {//判断不进行追加
            filed[index_fd].offset = 0;
        } else {
            filed[index_fd].offset = instruction->size - 1;//size应包含‘/0’
        }
        if((flags & 1) == 0) {//判断可读
            filed[index_fd].readable = 1;
            if((flags & 0b100) == 0) {
                filed[index_fd].writable = 0;
            }else{
                filed[index_fd].writable = 1;
            }
        } else {
            filed[index_fd].readable = 0;
            filed[index_fd].writable = 1;
        }
        if((flags & 0b1000000000) != 0 && filed[index_fd].writable == 1) {
            free(instruction->contnet);
            filed[index_fd].offset = 0;
        }
    }
    free(temp_string);
    free(str);
    return index_fd;
}
int rclose(int fd) {
    if(filed[fd].use == true){
        filed[fd].use = false;
        filed[fd].fileordir = NULL;
        filed[fd].writable = 0;
        filed[fd].readable = 0;
        filed[fd].offset = 0;
        filed[fd].type = 0;
        filed[fd].flags = 0;
        return 0;
    } else {
        return -1;
    }
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
        free(temp_string);
        free(str);
        return -1;
    }
    int count = strlen(str[index - 1]);
    if (count >= 4 && str[index - 1][count - 1] == 't' && str[index - 1][count - 2] == 'x'
        && str[index - 1][count - 3] == 't' && str[index - 1][count - 4] == '.') {
        free(temp_string);
        free(str);
        return -1;
    }//判断最后类型是目录
    node *instruction = root->child;//从头遍历链表
    for (int i = 0; i <= index - 1; i++) {
        if (i == index - 1) {
            for (;;) {
                if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                    if (strcmp(str[i], instruction->shortname) == 0) {
                        free(temp_string);
                        free(str);
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
                free(temp_string);
                free(str);
                return 0;
            } else {//判断链表自身为空并引入新节点
                instruction->sibling = NULL;
                instruction->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->shortname, str[i]);
                instruction->type = DIR_NODE;
                free(temp_string);
                free(str);
                return 0;
            }
        }
        if (instruction->shortname == NULL) {//排除直接建立跨级目录即这级目录为空
            free(temp_string);
            free(str);
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
            if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                free(temp_string);
                free(str);
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
}

int rrmdir(const char *pathname) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    int index = pathname_simple(str, temp_string, pathname);
    if (index == 0) {
        return -1;
    }
    int count = strlen(str[index - 1]);
    if (count >= 4 && str[index - 1][count - 1] == 't' && str[index - 1][count - 2] == 'x'
        && str[index - 1][count - 3] == 't' && str[index - 1][count - 4] == '.') {
        free(temp_string);
        free(str);
        return -1;
    }//判断最后类型是目录
    node *instruction;
    instruction = root->child;
    node *temp_instruction;
    node *temp_instruction_up;//上一级目录
    node *temp;
    for (int i = 0; i <= index; i++) {
        if(i == index){
            if(instruction == NULL) {//该目录为空『为首节点/为中间节点/为末节点/（既为首节点又为末节点）
                if(temp_instruction_up->child == temp_instruction && temp_instruction->sibling == NULL) {//既为首又为末
                    if(temp_instruction_up == root) {
                        temp = temp_instruction_up->child;
                        temp_instruction_up->child = malloc(sizeof (struct node));
                        free(temp);
                    }else {
                        temp = temp_instruction_up->child;
                        temp_instruction_up->child = NULL;
                        free(temp);
                    }
                }else if (temp_instruction_up->child == temp_instruction) {//首节点
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = temp_instruction->sibling;
                    free(temp);
                }else {
                    node *temp_nextup;
                    temp_nextup = temp_instruction_up->child;
                    for (;;) {//寻找删除元素的上一个节点
                        if(strcmp(temp_nextup->sibling->shortname, str[index - 1])){
                            break;
                        }
                        temp_nextup = temp_nextup->sibling;
                    }
                    if(temp_instruction->sibling == NULL) {//尾节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = NULL;
                        free(temp);
                    }else {
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = temp_instruction->sibling;//中间节点
                        free(temp);
                    }
                }
                free(temp_string);
                free(str);
                return 0;
            }else {
                free(temp_string);
                free(str);
                return -1;
            }
        }
        if (instruction->shortname == NULL) {//排除直接查找跨级目录即这级目录为空
            free(temp_string);
            free(str);
            return -1;
        }
        for(;;){
            if (strcmp(str[i], instruction->shortname) == 0) {
                if(i == index - 1) {
                    temp_instruction = instruction;
                }
                if(index >= 2) {
                    if (i == index - 2) {
                        temp_instruction_up = instruction;
                    }
                } else {
                    temp_instruction_up = root;
                }
                instruction = instruction->child;
                break;
            }
            if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                free(temp_string);
                free(str);
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
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
