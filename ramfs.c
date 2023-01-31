#include "ramfs.h"
/* modify this file freely */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define max_fd 4096
#define length_name 32
#define length_road 1024
typedef struct node {
    enum {
        FILE_NODE = 1,
        DIR_NODE = 2
    } type;
    struct node *child;
    struct node *sibling;
    void *content;
    int size;
    char *shortname;
} node;
node *root;
typedef struct filedesc {
    bool use;
    enum {
        file = 1,
        dir = 2,
    } type;
    int offset;
    //int flags;
    int writable;
    int readable;
    node *fileordir;
} filedesc;
filedesc filed[max_fd + 3];

int pathname_simple(char **str, char *temp_string, const char *pathname) {
    int lengthofroad = strlen(pathname);
    if (*pathname == '/' && lengthofroad <= length_road) {
        int index = 0;
        char *temp = strtok(temp_string, "/");
        while (temp != NULL) {
            if (strlen(temp) > length_name) {
        //        str = NULL;
                return 0;
            }
            str[index] = temp;
            int length = strlen(str[index]);
            for (int i = 0; i <= length - 1 ; i++) {
                if(*(str[index]+i) == 46 || (*(str[index]+i) >= '0' && *(str[index]+i) <= '9' ) ||
                        (*(str[index]+i) >= 'a' && *(str[index]+i) <= 'z' )||
                        (*(str[index]+i) >= 'A' && *(str[index]+i) <= 'Z' )){

                } else {
                    return 0;
                }
            }
            temp = strtok(NULL, "/");
            index++;
        }
        return index;
    } else {
    //    str = NULL;
        return 0;
    }
}

int ropen(const char *pathname, int flags) {
    char *temp_string = malloc(length_road + 1);
    strcpy(temp_string, pathname);
    char **str = malloc(length_road + 1);
    int index = pathname_simple(str, temp_string, pathname);
    if (index == 0) {
        free(temp_string);
        free(str);
        return -1;
    }
    int length_pathname = strlen(pathname);
    if(pathname[length_pathname - 1] == '/') {
        free(temp_string);
        free(str);
        return -1;
    }
    node *instruction = root->child;
    node *instruction_temp = root;
    if ((flags & O_CREAT) == 0) {//判断不用创建文件
        for (int i = 0; i <= index - 1; i++) {
            if (i == index - 1) {
                if (instruction == NULL) {//最低层目录为空
                    free(temp_string);
                    free(str);
                    return -1;
                }
                for (;;) {
                    if (strcmp(instruction->shortname, str[i]) == 0 && instruction->type == FILE_NODE) {
                        break;
                    }
                    if (instruction->sibling == NULL) {
                        free(temp_string);
                        free(str);
                        return -1;
                    }
                    instruction = instruction->sibling;
                }
                break;
            }
            if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
                free(temp_string);
                free(str);
                return -1;
            }
            for (;;) {
                if (strcmp(str[i], instruction->shortname) == 0) {
                    instruction_temp = instruction;
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
            for (int i = 0; i <= index - 1; i++) {
                if (i == index - 1) {
                    if(instruction_temp->type == FILE_NODE) {
                        free(temp_string);
                        free(str);
                        return -1;
                    }
                    if (instruction != NULL) {
                        int flag_file = 0;
                        for (;;) {
                            //if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                            if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == FILE_NODE) {
                                flag_file = 1;
                                break;
                            }
                            //}
                            if (instruction->sibling == NULL) {//（之后循环退出的条件）
                                break;
                            }
                            instruction = instruction->sibling;
                        }//检查有没有重名的
                        if(flag_file == 1){
                            break;
                        }
                        instruction->sibling = malloc(sizeof(struct node));
                        instruction->sibling->sibling = NULL;
                        instruction->sibling->child = NULL;
                        instruction->sibling->shortname = malloc(strlen(str[i]) + 1);
                        strcpy(instruction->sibling->shortname, str[i]);
                        instruction->sibling->type = FILE_NODE;
                        instruction = instruction->sibling;
                        break;
                    } else {//判断链表自身为空并引入新节点
                        instruction_temp->child = malloc(sizeof(struct node));
                        instruction = instruction_temp->child;
                        instruction->sibling = NULL;
                        instruction->child = NULL;
                        instruction->shortname = malloc(strlen(str[i]) + 1);
                        strcpy(instruction->shortname, str[i]);
                        instruction->type = FILE_NODE;
                        break;
                    }
                }
                if (instruction == NULL) {//排除直接建立跨级目录即这级目录为空
                    free(temp_string);
                    free(str);
                    return -1;
                }
                for (;;) {
                    if (strcmp(str[i], instruction->shortname) == 0) {
//                        if (instruction->child == NULL) {
//                            instruction->child = malloc(sizeof(struct node));
//                        }
                        instruction_temp = instruction;
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
    int index_fd = 0;
    for (int i = 0; i <= max_fd - 1; i++) {
        if (filed[i].use == false) {
            index_fd = i;
            filed[i].use = true;
            break;
        }
    }
    if (instruction->type == DIR_NODE) {
        filed[index_fd].fileordir = instruction;
        filed[index_fd].type = dir;
    } else if (instruction->type == FILE_NODE) {
        //filed[index_fd].flags = flags;
        filed[index_fd].type = file;
        filed[index_fd].fileordir = instruction;
        if ((flags & 02000) == 0) {//判断不进行追加
            filed[index_fd].offset = 0;
        } else {
            filed[index_fd].offset = instruction->size;//size应包含‘/0’
        }
        if ((flags & 1) == 0) {//判断可读
            filed[index_fd].readable = 1;
            if ((flags & 0b10) == 0) {
                filed[index_fd].writable = 0;
            } else {
                filed[index_fd].writable = 1;
            }
        } else {
            filed[index_fd].readable = 0;
            filed[index_fd].writable = 1;
        }
        if ((flags & 01000) != 0 && filed[index_fd].writable == 1) {
            void *temp_content = instruction->content;
            instruction->content = NULL;
            free(temp_content);
            filed[index_fd].offset = 0;
            instruction->size = 0;
        }
    }
    free(temp_string);
    free(str);
    return index_fd;
}

int rclose(int fd) {
    if(fd < 0 || fd >= max_fd){
        return -1;
    }
    if (filed[fd].use == true) {
        filed[fd].use = false;
        filed[fd].fileordir = NULL;
        filed[fd].writable = 0;
        filed[fd].readable = 0;
        filed[fd].offset = 0;
        filed[fd].type = 0;
        //filed[fd].flags = 0;
        return 0;
    } else {
        return -1;
    }
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if(fd < 0 || fd >= max_fd){
        return -1;
    }
    if (filed[fd].writable == 0 || filed[fd].type == dir) {
        return -1;
    }
    if (filed[fd].fileordir->size == 0) {
        filed[fd].fileordir->content = malloc(1);
    }
    if (filed[fd].offset > filed[fd].fileordir->size) {
        void *temp = realloc(filed[fd].fileordir->content, filed[fd].offset);
        filed[fd].fileordir->content = temp;
        for (int i = filed[fd].fileordir->size; i <= filed[fd].offset - 1; i++) {
            memcpy(filed[fd].fileordir->content + i, "\0", 1);
        }
    }
    int need_size = filed[fd].offset + count;
    if (need_size > filed[fd].fileordir->size) {
        void *temp = realloc(filed[fd].fileordir->content, need_size);
        filed[fd].fileordir->content = temp;
        filed[fd].fileordir->size = need_size;
    }
    memcpy((filed[fd].fileordir->content + filed[fd].offset), buf, count);
    filed[fd].offset = filed[fd].offset + count;
    return count;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if (filed[fd].readable == 0 || filed[fd].type == dir) {
        return -1;
    }
    size_t need;
    if (filed[fd].offset + count > filed[fd].fileordir->size) {
        need = filed[fd].fileordir->size - filed[fd].offset;
    } else {
        need = count;
    }
    memcpy(buf, filed[fd].fileordir->content + filed[fd].offset, need);
    filed[fd].offset = filed[fd].offset + need;
    return need;
}

off_t rseek(int fd, off_t offset, int whence) {
    if(fd < 0 || fd >= 4096){
        return -1;
    }
    if (whence == SEEK_SET) {
        filed[fd].offset = offset;
    } else if (whence == SEEK_CUR) {
        filed[fd].offset = filed[fd].offset + offset;
    } else {
        filed[fd].offset = filed[fd].fileordir->size + offset;
    }
    if (filed[fd].offset < 0) {
        return -1;
    }
    return filed[fd].offset;
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
    node *instruction = root->child;//从头遍历链表
    node *instruction_temp = root;
    for (int i = 0; i <= index - 1; i++) {
        if (i == index - 1) {
            if(instruction_temp->type == FILE_NODE) {
                free(temp_string);
                free(str);
                return -1;
            }
            if (instruction != NULL) {
                for (;;) {
                    //if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                    if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == FILE_NODE) {
                        free(temp_string);
                        free(str);
                        return -1;
                    }
                    //}
                    if (instruction->sibling == NULL) {//（之后循环退出的条件）
                        break;
                    }
                    instruction = instruction->sibling;
                }//检查有没有重名的
                instruction->sibling = malloc(sizeof(struct node));
                instruction->sibling->sibling = NULL;
                instruction->sibling->child = NULL;
                instruction->sibling->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->sibling->shortname, str[i]);
                instruction->sibling->type = DIR_NODE;
                free(temp_string);
                free(str);
                return 0;
            } else {//判断链表自身为空并引入新节点
                instruction_temp->child = malloc(sizeof(struct node));
                instruction = instruction_temp->child;
                instruction->sibling = NULL;
                instruction->child = NULL;
                instruction->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->shortname, str[i]);
                instruction->type = DIR_NODE;
                free(temp_string);
                free(str);
                return 0;
            }
        }
        if (instruction == NULL) {//排除直接建立跨级目录即这级目录为空
            free(temp_string);
            free(str);
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0) {
//                if (instruction->child == NULL) {
//                    instruction->child = malloc(sizeof(struct node));
//                }
                instruction_temp = instruction;
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
    node *instruction = root->child;
    node *temp_instruction;//要删除的目录
    node *temp_instruction_up ;//要删除的上一级目录
    node *temp;//暂存需要free的目录
    for (int i = 0; i <= index; i++) {
        if (i == index) {
            if(temp_instruction->type == FILE_NODE){
                free(temp_string);
                free(str);
                return -1;
            }
            if (instruction == NULL) {//该目录为空『为首节点/为中间节点/为末节点/（既为首节点又为末节点）
                if (temp_instruction_up->child == temp_instruction && temp_instruction->sibling == NULL) {//既为首又为末
                        temp = temp_instruction_up->child;
                        temp_instruction_up->child = NULL;//malloc(sizeof (struct node));
                        free(temp);
                } else if (temp_instruction_up->child == temp_instruction) {//首节点但不是末节点
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = temp_instruction->sibling;
                    free(temp);
                } else {
                    node *temp_nextup;
                    temp_nextup = temp_instruction_up->child;
                    for (;;) {//寻找删除元素的上一个节点
                        if (strcmp(temp_nextup->sibling->shortname, str[index - 1]) == 0) {
                            break;
                        }
                        temp_nextup = temp_nextup->sibling;
                    }
                    if (temp_instruction->sibling == NULL) {//尾节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = NULL;
                        free(temp);
                    } else {//中间节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = temp_instruction->sibling;
                        free(temp);
                    }
                }
                free(temp_string);
                free(str);
                return 0;
            } else {
                free(temp_string);
                free(str);
                return -1;
            }
        }
        if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
            free(temp_string);
            free(str);
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0) {
                if(index == 1){
                    temp_instruction = instruction;
                    temp_instruction_up = root;
                }else {
                    if (i == index - 1) {
                        temp_instruction = instruction;
                    }
                    if (i == index - 2) {
                        temp_instruction_up = instruction;
                    }
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
        free(temp_string);
        free(str);
        return -1;
    }
    int length_pathname = strlen(pathname);
    if(pathname[length_pathname - 1] == '/') {
        free(temp_string);
        free(str);
        return -1;
    }
    node *instruction;
    instruction = root->child;
    node *temp_instruction;
    node *temp_instruction_up;//上一级目录
    node *temp;
    for (int i = 0; i <= index; i++) {
        if (i == index) {
            if(temp_instruction->type == DIR_NODE){
                free(temp_string);
                free(str);
                return -1;
            }
            if (instruction == NULL) {//该目录为空『为首节点/为中间节点/为末节点/（既为首节点又为末节点）
                if (temp_instruction_up->child == temp_instruction && temp_instruction->sibling == NULL) {//既为首又为末
                        temp = temp_instruction_up->child;
                        temp_instruction_up->child = NULL;
                        free(temp->content);
                        free(temp);
                } else if (temp_instruction_up->child == temp_instruction) {//首节点
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = temp_instruction->sibling;
                    free(temp->content);
                    free(temp);
                } else {
                    node *temp_nextup;
                    temp_nextup = temp_instruction_up->child;
                    for (;;) {//寻找删除元素的上一个节点
                        if (strcmp(temp_nextup->sibling->shortname, str[index - 1]) == 0) {
                            break;
                        }
                        temp_nextup = temp_nextup->sibling;
                    }
                    if (temp_instruction->sibling == NULL) {//尾节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = NULL;
                        free(temp->content);
                        free(temp);
                    } else {
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = temp_instruction->sibling;//中间节点
                        free(temp->content);
                        free(temp);
                    }
                }
                free(temp_string);
                free(str);
                return 0;
            } else {
                free(temp_string);
                free(str);
                return -1;
            }
        }
        if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
            free(temp_string);
            free(str);
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0) {
                if(index == 1){
                    temp_instruction = instruction;
                    temp_instruction_up = root;
                }else {
                    if (i == index - 1) {
                        temp_instruction = instruction;
                    }
                    if (i == index - 2) {
                        temp_instruction_up = instruction;
                    }
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

void init_ramfs() {
    root = malloc(sizeof(struct node));
    root->type = DIR_NODE;
    root->shortname = "/";
    root->sibling = NULL;
    root->child = NULL;//malloc(sizeof(struct node));
}
