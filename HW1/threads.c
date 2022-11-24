#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread* current_thread = NULL;
static struct thread* root_thread = NULL;
static int id = 1;
static jmp_buf env_st;  //for init
//static jmp_buf env_tmp;  
//for tmp
// TODO: necessary declares, if any
//for schedule
int checkarrlen = 0;
int targetidx = 0; //current_thread ID
int curidx = 0;
struct thread* schedulearr[128];

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    //unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);   //head
    new_stack_p = new_stack +0x100*8-0x2*8;   //tail
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;
    return t;
}
void thread_add_runqueue(struct thread *t){
    if(current_thread == NULL){
        // TODO
        current_thread = t;
        root_thread = t;
        t -> parent = t;
        t -> left = NULL;
        t -> right = NULL;
    }
    else{
        // TODO
        if (current_thread->left == NULL){
            current_thread -> left = t;
            t -> parent = current_thread;
            t-> left = NULL; t-> right = NULL;
        }
        else if (current_thread -> right == NULL){
            current_thread -> right = t;
            t -> parent = current_thread;
            t-> left = NULL; t-> right = NULL;
        }
        else{
            free(t);
        }
    }
    return;
}
void thread_yield(void){
    // TODO
    if (setjmp(current_thread->env) == 0){  //load in current thread's jmp_buf
        schedule();
        dispatch();
    }
    return;
}
void dispatch(void){
    // TODO
    if (current_thread->buf_set == 0){  //if jmpbuf not set yet
        if (setjmp(current_thread->env) == 0){
            current_thread->env->sp = (unsigned long)current_thread ->stack_p;
            current_thread -> buf_set = 1;
            longjmp(current_thread->env,1);
        }
        else{
            current_thread -> fp(current_thread->arg);  
        }        
    }
    else{
        longjmp(current_thread->env,1);
    }
    current_thread -> buf_set = 0;
    thread_exit();
    return;
}
void preorder_traverse(struct thread *cur){
    //printf("Thread ID:%d\n",cur->ID);
    if (cur == current_thread){
        targetidx = curidx;
    }
    schedulearr[curidx] = cur;
    curidx++;checkarrlen++;
    if (cur -> left != NULL){
        preorder_traverse(cur->left);
    }
    if (cur -> right != NULL){
        preorder_traverse(cur->right);
    }
    return;
}
void schedule(void){
    // TODO
    checkarrlen = 0; curidx = 0; targetidx = 0;
    preorder_traverse(root_thread);
    if (targetidx == checkarrlen-1){  //the last one
        current_thread = root_thread;
    }
    else{
        current_thread = schedulearr[targetidx+1];
    }
    return;
}

void thread_exit(void){
    if(current_thread == root_thread && current_thread->left == NULL && current_thread->right == NULL){
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack);
        free(current_thread);
        root_thread = NULL;
        current_thread = NULL;
        longjmp(env_st,1);
    }
    else{
        // TODO
        if (current_thread->left == NULL && current_thread -> right == NULL){  //leaf
            struct thread *temp = current_thread;
            schedule();
            if (temp->parent->left == temp){
                temp->parent->left = NULL;
            }
            if (temp->parent->right == temp){
                temp->parent->right = NULL;
            }
            free(temp->stack);
            free(temp); 
            dispatch();
        }
        else{  //otherwise
            checkarrlen = 0; curidx = 0; targetidx = 0;
            preorder_traverse(current_thread);
            struct thread *temp = schedulearr[checkarrlen-1]; 
            //clear temp's parent
            if (temp->parent->left == temp){
                temp->parent->left = NULL;
            }
            if (temp->parent->right == temp){
                temp->parent->right = NULL;
            }
            //temp go up
            if (current_thread->parent != current_thread)
                temp -> parent = current_thread -> parent;
            else
                temp -> parent = temp;
            if (current_thread->left != NULL)
                temp -> left = current_thread -> left;
            else
                temp -> left = NULL;
            if (current_thread->right != NULL)
                temp -> right = current_thread -> right;
            else
                temp -> right = NULL;
            // parent go temp
            if (current_thread->parent != current_thread){
                if (current_thread->parent->right == current_thread){
                    current_thread->parent->right = temp;
                }
                else if (current_thread->parent->left == current_thread){
                    current_thread->parent->left = temp;
                }
            }
            //left go temp
            if (current_thread->left != NULL)
                current_thread->left-> parent = temp;
            //right go temp
            if (current_thread->right != NULL)
                current_thread->right-> parent = temp;
            if (current_thread == root_thread){//if root thread was popped
                root_thread = temp; 
            } 
            free(current_thread->stack);
            free(current_thread);
            current_thread = temp;
            schedule();
            dispatch();
        }     
    }
    return;
}
void thread_start_threading(void){
    // TODO
    if (setjmp(env_st) == 0){
        schedule();
        dispatch();
    }
    return;
}
