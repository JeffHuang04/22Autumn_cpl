#include "ramfs.h"
/* modify this file freely */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define max_deepth_think 1024
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
node *root = NULL;
typedef struct filedesc {
    bool use;
    enum {
        file = 1,
        dir = 2,
    } type;
    int offset;
    int writable;
    int readable;
    node *fileordir;
} filedesc;
filedesc filed[max_fd + 3];
void freestr(char **str,int index){
    if(index >= 1) {
        for (int i = 0; i <= index - 1; i++) {
            //char *temp = str[index];
            free(str[i]);
            str[i] = NULL;
        }
    }
    free(str);
    str = NULL;
}
void freetemp(char *temp){
    if(temp != NULL) {
        free(temp);
        temp = NULL;
    }
}
void freenode(node *temp){
    free(temp->shortname);
    temp->shortname = NULL;
    free(temp->content);
    temp->content = NULL;
    temp->sibling = NULL;
    temp->child = NULL;
    temp->size = 0;
    free(temp);
    temp = NULL;
}
int pathname_simple(char **str, char *temp_pathname) {
    int length = strlen(temp_pathname);
    if (*temp_pathname == '/') {
        int index = -1;
        int offset_ = 0;
        for (int i = 0; i <= length - 1 ; i++) {
            if(*(temp_pathname + i) == '/' ) {
                offset_ = 0;
                if(( i< length - 1 && *(temp_pathname + i + 1) == '/' ) || i == length - 1) {

                }else{
                    index ++;
                }
            }else{
                if (*(temp_pathname + i) == '.' ||
                    (*(temp_pathname + i) >= '0' && *(temp_pathname + i) <= '9') ||
                    (*(temp_pathname + i) >= 'a' && *(temp_pathname + i) <= 'z') ||
                    (*(temp_pathname + i) >= 'A' && *(temp_pathname + i) <= 'Z')){
                    if(offset_ >= length_name) {
                        return -2;
                    }
                    if(offset_ == 0){
                        str[index] = NULL;
                        str[index] = malloc(length_name + 1);
                        //memset(str[index],'\0',length_name);
                    }
                    memcpy(str[index] + offset_, temp_pathname + i,1);
                    offset_++;
                    memcpy(str[index] + offset_,"\0",1);
//                    char *temp_show = str[index];
                } else {
                    return -2;
                }
            }
        }
        return index;
    } else {
        return -2;
    }
}//没问题

int ropen(const char *pathname, int flags) {
    int length_pathname = strlen(pathname);
    if(length_pathname > length_road){
        return -1;
    }
    char **str = NULL;
    str = malloc( max_deepth_think + 1);
    char *temp_pathname = NULL;
    temp_pathname = malloc(length_pathname + 1);
    strcpy(temp_pathname,pathname);
    int index = pathname_simple(str, temp_pathname) + 1;
    node *instruction = root->child;
    node *instruction_temp = root;
    bool ifroot = false;
    if (index == -1) {
        freetemp(temp_pathname);
        freestr(str,index);
        return -1;
    } else if(index == 0) {
        instruction = root;
        ifroot = true;
    }
    if(ifroot == false) {
        if ((flags & O_CREAT) == 0) {//判断不用创建文件只需查找
            for (int i = 0; i <= index - 1; i++) {
                if (i == index - 1) {
                    if (instruction == NULL) {//最低层目录为空
                        freetemp(temp_pathname);
                        freestr(str,index);
                        return -1;
                    }
                    for (;;) {
                        if (strcmp(instruction->shortname, str[i]) == 0 /*&& instruction->type == FILE_NODE*/) {
                            if (pathname[length_pathname - 1] == '/' && instruction->type == FILE_NODE) {
                                freetemp(temp_pathname);
                                freestr(str,index);
                                return -1;
                            }
                            break;
                        }
                        if (instruction->sibling == NULL) {
                            freetemp(temp_pathname);
                            freestr(str,index);
                            return -1;
                        }
                        instruction = instruction->sibling;
                    }
                    break;
                }
                if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
                    freetemp(temp_pathname);
                    freestr(str,index);
                    return -1;
                }
                for (;;) {
                    if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == DIR_NODE) {
                        //instruction_temp = instruction;
                        instruction = instruction->child;
                        break;
                    }
                    if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                        freetemp(temp_pathname);
                        freestr(str,index);
                        return -1;
                    }
                    instruction = instruction->sibling;
                }
            }
        } else {//判断要创建文件、打开文件、打开目录
            for (int i = 0; i <= index - 1; i++) {
                if (i == index - 1) {
                    if (instruction != NULL) {
                        int flag_file = 0;
                        for (;;) {
                            //if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                            if (strcmp(str[i], instruction->shortname) == 0/* && instruction->type == FILE_NODE*/) {
                                if (pathname[length_pathname - 1] == '/' && instruction->type == FILE_NODE) {
                                    freetemp(temp_pathname);
                                    freestr(str,index);
                                    return -1;
                                }
                                flag_file = 1;
                                break;
                            }
                            //}
                            if (instruction->sibling == NULL) {//（之后循环退出的条件）
                                break;
                            }
                            instruction = instruction->sibling;
                        }//检查有没有重名的
                        if (flag_file == 1) {
                            break;
                        }
                        if (pathname[length_pathname - 1] == '/') {
                            freetemp(temp_pathname);
                            freestr(str,index);
                            return -1;
                        }
                        instruction->sibling = NULL;
                        instruction->sibling = malloc(sizeof( node) + 5);
                        instruction->sibling->sibling = NULL;
                        instruction->sibling->child = NULL;
                        instruction->sibling->shortname = NULL;
                        instruction->sibling->shortname = malloc(strlen(str[i]) + 1);
                        strcpy(instruction->sibling->shortname, str[i]);
                        instruction->sibling->type = FILE_NODE;
                        instruction->sibling->size = 0;
                        instruction = instruction->sibling;
                        break;
                    } else {//判断链表自身为空并引入新节点
                        if (pathname[length_pathname - 1] == '/') {
                            freetemp(temp_pathname);
                            freestr(str,index);
                            return -1;
                        }
                        instruction_temp->child = malloc(sizeof(struct node) + 5);
                        instruction = instruction_temp->child;
                        instruction->sibling = NULL;
                        instruction->child = NULL;
                        instruction->shortname = malloc(strlen(str[i]) + 1);
                        strcpy(instruction->shortname, str[i]);
                        instruction->type = FILE_NODE;
                        instruction->size = 0;
                        break;
                    }
                }
                if (instruction == NULL) {//排除直接建立跨级目录即这级目录为空
                    freetemp(temp_pathname);
                    freestr(str,index);
                    return -1;
                }
                for (;;) {
                    if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == DIR_NODE) {
//                        if (instruction->child == NULL) {
//                            instruction->child = malloc(sizeof(struct node));
//                        }
                        instruction_temp = instruction;
                        instruction = instruction->child;
                        break;
                    }
                    if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                        freetemp(temp_pathname);
                        freestr(str,index);
                        return -1;
                    }
                    instruction = instruction->sibling;
                }
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
        if(i == max_fd - 1){
            freetemp(temp_pathname);
            freestr(str,index);
            return -1;
        }
    }
    if (instruction->type == DIR_NODE) {
        filed[index_fd].fileordir = instruction;
        filed[index_fd].type = dir;
    } else if (instruction->type == FILE_NODE) {
        //filed[index_fd].flags = flags;
        filed[index_fd].type = file;
        filed[index_fd].fileordir = instruction;
        if ((flags & O_APPEND) == 0) {//判断不进行追加
            filed[index_fd].offset = 0;
        } else {
            filed[index_fd].offset = instruction->size;//size应包含‘/0’
        }
        if ((flags & O_WRONLY) == 0) {//判断可读
            filed[index_fd].readable = 1;
            if ((flags & O_RDWR) == 0) {
                filed[index_fd].writable = 0;
            } else {
                filed[index_fd].writable = 1;
            }
        } else {
            filed[index_fd].readable = 0;
            filed[index_fd].writable = 1;
        }
        if ((flags & O_TRUNC) != 0 && filed[index_fd].writable == 1) {//清空
            void *temp_content = instruction->content;
            instruction->content = NULL;
            free(temp_content);
            filed[index_fd].offset = 0;
            instruction->size = 0;
        }
    }
    freetemp(temp_pathname);
    freestr(str,index);
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
        filed[fd].type = dir;
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
    if (filed[fd].use == false || filed[fd].writable == 0 || filed[fd].type == dir) {
        return -1;
    }
    int need_size = filed[fd].offset + count;
    if (need_size > filed[fd].fileordir->size) {
        if (filed[fd].offset > filed[fd].fileordir->size) {
            void *temp = realloc(filed[fd].fileordir->content, filed[fd].offset + 1);
            filed[fd].fileordir->content = temp;
            for (int i = filed[fd].fileordir->size; i <= filed[fd].offset - 1; i++) {
                memcpy(filed[fd].fileordir->content + i, "\0", 1);
            }
        }
        void *temp = realloc(filed[fd].fileordir->content, need_size);
        filed[fd].fileordir->content = temp;
        filed[fd].fileordir->size = need_size;
    }
    memcpy((filed[fd].fileordir->content + filed[fd].offset), buf, count);
    filed[fd].offset = filed[fd].offset + count;
    char *temp_f = filed[fd].fileordir->content;
    return count;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if(fd < 0 || fd >= max_fd){
        return -1;
    }
    if (filed[fd].use==false || filed[fd].readable == 0 || filed[fd].type == dir) {
        return -1;
    }
    if(filed[fd].fileordir->content == NULL) {
        return -1;
    }
    int need = 0;//如果是负值会如何
    if ((int )(filed[fd].offset + count) > filed[fd].fileordir->size) {
        need = filed[fd].fileordir->size - filed[fd].offset;
    } else {
        need = count;
    }
    if (need < 0) {
        return -1;
    }
    memcpy(buf, filed[fd].fileordir->content + filed[fd].offset, need);
    filed[fd].offset = filed[fd].offset + need;
    return need;
}

off_t rseek(int fd, off_t offset, int whence) {
    if(fd < 0 || fd >= 4096){
        return -1;
    }
    if(filed[fd].use == false){
        return -1;
    }
    int temp_offset = 0;
    if (whence == SEEK_SET) {
        temp_offset = offset;
    } else if (whence == SEEK_CUR) {
        temp_offset = filed[fd].offset + offset;
    } else if(whence == SEEK_END){
        temp_offset = filed[fd].fileordir->size + offset;
    }
    if (temp_offset < 0) {
        return -1;
    }
    filed[fd].offset = temp_offset;
    return filed[fd].offset;
}

int rmkdir(const char *pathname) {
    int length_pathname = strlen(pathname);
    if(length_pathname > length_road){
        return -1;
    }
    char **str = NULL;
    str = malloc( max_deepth_think + 1);
    char *temp_pathname = NULL;
    temp_pathname = malloc(length_pathname + 1);
    strcpy(temp_pathname,pathname);
    int index = pathname_simple(str, temp_pathname) + 1;
    if (/*str == NULL*/index == -1 || index == 0) {
        freetemp(temp_pathname);
        freestr(str,index);
        return -1;
    }
    node *instruction = root->child;//从头遍历链表
    node *instruction_temp = root;
    for (int i = 0; i <= index - 1; i++) {
        if (i == index - 1) {
            if (instruction != NULL) {
                for (;;) {
                    //if (instruction->shortname != NULL) {//判断链表自身是否为空（只有第一次循环有用）
                    if (strcmp(str[i], instruction->shortname) == 0 ) {
                        freetemp(temp_pathname);
                        freestr(str,index);
                        return -1;
                    }
                    //}
                    if (instruction->sibling == NULL) {//（之后循环退出的条件）
                        break;
                    }
                    instruction = instruction->sibling;
                }//检查有没有重名的
                instruction->sibling = malloc(sizeof(struct node) + 5);
                instruction->sibling->sibling = NULL;
                instruction->sibling->child = NULL;
                instruction->sibling->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->sibling->shortname, str[i]);
                instruction->sibling->type = DIR_NODE;
                instruction->sibling->size = 0;
                freetemp(temp_pathname);
                freestr(str,index);
                return 0;
            } else {//判断链表自身为空并引入新节点
                instruction_temp->child = malloc(sizeof(struct node) + 5);
                instruction = instruction_temp->child;
                instruction->sibling = NULL;
                instruction->child = NULL;
                instruction->shortname = malloc(strlen(str[i]) + 1);
                strcpy(instruction->shortname, str[i]);
                instruction->type = DIR_NODE;
                instruction->size = 0;
                freetemp(temp_pathname);
                freestr(str,index);
                return 0;
            }
        }
        if (instruction == NULL) {//排除直接建立跨级目录即这级目录为空
            freetemp(temp_pathname);
            freestr(str,index);
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == DIR_NODE) {
                instruction_temp = instruction;
                instruction = instruction->child;
                break;
            }
            if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                freetemp(temp_pathname);
                freestr(str,index);
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
}

int rrmdir(const char *pathname) {
    int length_pathname = strlen(pathname);
    if(length_pathname > length_road){
        return -1;
    }
    char **str = NULL;
    str = malloc( max_deepth_think + 1);
    char *temp_pathname = NULL;
    temp_pathname = malloc(length_pathname + 1);
    strcpy(temp_pathname,pathname);
    int index = pathname_simple(str, temp_pathname) + 1;
    if (index == 0 || index == -1) {
        freetemp(temp_pathname);
        freestr(str,index);
        return -1;
    }
    node *instruction = root->child;
    node *temp_instruction = root->child;//要删除的目录
    node *temp_instruction_up = root;//要删除的上一级目录
    node *temp = NULL;//暂存需要free的目录
    for (int i = 0; i <= index; i++) {
        if (i == index) {
            if (instruction == NULL) {//该目录为空『为首节点/为中间节点/为末节点/（既为首节点又为末节点）
                if (temp_instruction_up->child == temp_instruction && temp_instruction->sibling == NULL) {//既为首又为末
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = NULL;//malloc(sizeof (struct node));
                    freenode(temp);
                } else if (temp_instruction_up->child == temp_instruction) {//首节点但不是末节点
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = temp_instruction->sibling;
                    freenode(temp);
                } else {
                    node *temp_nextup = NULL;
                    temp_nextup = temp_instruction_up->child;
                    for (;;) {//寻找删除元素的上一个节点
                        if (strcmp(temp_nextup->sibling->shortname, str[index - 1]) == 0
                            && temp_nextup->sibling->type == DIR_NODE) {
                            //temp_nextup = temp_nextup->sibling;
                            break;
                        }
                        temp_nextup = temp_nextup->sibling;
                    }
                    if (temp_instruction->sibling == NULL) {//尾节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = NULL;
                        freenode(temp);
                    } else {//中间节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = temp_instruction->sibling;
                        freenode(temp);
                    }
                }
                freetemp(temp_pathname);
                freestr(str,index);
                return 0;
            } else {
                freetemp(temp_pathname);
                freestr(str,index);
                return -1;
            }
        }
        if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
            freetemp(temp_pathname);
            freestr(str,index);
            return -1;
        }
        for (;;) {
            if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == DIR_NODE) {
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
                freetemp(temp_pathname);
                freestr(str,index);
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
}

int runlink(const char *pathname) {
    int length_pathname = strlen(pathname);
    if(length_pathname > length_road){
        return -1;
    }
    char **str = malloc( max_deepth_think + 1);
    char *temp_pathname = malloc(length_pathname + 1);
    strcpy(temp_pathname,pathname);
    int index = pathname_simple(str, temp_pathname) + 1;
    if (index == 0 || index == -1) {
        freetemp(temp_pathname);
        freestr(str,index);
        return -1;
    }
    if(pathname[length_pathname - 1] == '/') {
        freetemp(temp_pathname);
        freestr(str,index);
        return -1;
    }
    node *instruction = NULL;
    instruction = root->child;
    node *temp_instruction = root->child;
    node *temp_instruction_up = root;//上一级目录
    node *temp = NULL;
    for (int i = 0; i <= index; i++) {
        if (i == index) {
            if (instruction == NULL) {//该目录为空『为首节点/为中间节点/为末节点/（既为首节点又为末节点）
                if (temp_instruction_up->child == temp_instruction && temp_instruction->sibling == NULL) {//既为首又为末
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = NULL;
                    freenode(temp);
                } else if (temp_instruction_up->child == temp_instruction) {//首节点
                    temp = temp_instruction_up->child;
                    temp_instruction_up->child = temp_instruction->sibling;
                    freenode(temp);
                } else {
                    node *temp_nextup;
                    temp_nextup = temp_instruction_up->child;
                    for (;;) {//寻找删除元素的上一个节点
                        if (strcmp(temp_nextup->sibling->shortname, str[index - 1]) == 0 && temp_nextup->sibling->type == FILE_NODE) {
                            break;
                        }
                        temp_nextup = temp_nextup->sibling;
                    }
                    if (temp_instruction->sibling == NULL) {//尾节点
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = NULL;
                        freenode(temp);
                    } else {
                        temp = temp_nextup->sibling;
                        temp_nextup->sibling = temp_instruction->sibling;//中间节点
                        freenode(temp);
                    }
                }
                freetemp(temp_pathname);
                freestr(str,index);
                return 0;
            } else {
                freetemp(temp_pathname);
                freestr(str,index);
                return -1;
            }
        }
        if (instruction == NULL) {//排除直接查找跨级目录即这级目录为空
            freetemp(temp_pathname);
            freestr(str,index);
            return -1;
        }
        for (;;) {
            if(i <= index - 2) {
                if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == DIR_NODE) {
                    if (i == index - 2) {
                        temp_instruction_up = instruction;
                    }
                    instruction = instruction->child;
                    break;
                }
            } else if(i == index - 1){
                if (strcmp(str[i], instruction->shortname) == 0 && instruction->type == FILE_NODE) {
                    if (index == 1) {
                        temp_instruction = instruction;
                        temp_instruction_up = root;
                    } else {
                        temp_instruction = instruction;
                    }
                    instruction = instruction->child;
                    break;
                }
            }
            if (instruction->sibling == NULL) {//遍历后发现父级目录不存在
                freetemp(temp_pathname);
                freestr(str,index);
                return -1;
            }
            instruction = instruction->sibling;
        }
    }
}

void init_ramfs() {
    root = malloc(sizeof(struct node) + 5);
    root->type = DIR_NODE;
    root->shortname = malloc(2);
    strcpy(root->shortname,"/");
    root->sibling = NULL;
    root->child = NULL;//malloc(sizeof(struct node));
    root->content = NULL;
    root->size = 0;
    filedesc *p = filed;
    p = calloc(max_fd,sizeof (filedesc));
}
//
// Created by Hrs20 on 2023/2/1.
//
