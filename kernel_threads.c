
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include "util.h"
#include "kernel_streams.h"






/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{ 

	
}

/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) cur_thread();
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval){


  CURTHREAD->ptcb->exited=1 ;
  CURTHREAD->ptcb->exitval=exitval;
 
  kernel_broadcast(&CURTHREAD->ptcb->exit_cv);
  rlist_remove(&(CURTHREAD ->ptcb)-> ptcb_list_node);

  CURPROC->thread_count--;
   if (CURTHREAD->ptcb->refcount==0)
   {
     free(CURTHREAD->ptcb)
   }
   if (CURPROC->thread_count==1){
  
     PCB* curproc = CURPROC;

     if (curproc->args){
       free(curproc->args)
       curproc->args =NULL;
     }

     for (int i = 0; i < MAX_FILEID; ++i){
       if (curproc->FIDT[i] != NULL){
         FCB_decref(curproc->FIDT[i]);
         curproc->FIDT[i] = NULL ;
       }
     }

     PCB* initpcb = get_pcb(1) ;
     while(!is_rlist_empty(& curproc ->children_list)){
       rlnode* child = rlist_pop_front(& curproc ->children_list);
       child ->pcb->parent = initpcb;

     } 



    if (!is_rlist_empty(& curproc->exited_list){
      rlist_append(& initpcb->exited_list,& curproc->exited_list);
      kernel_broadcast(& initpcb->child_exit);
    }

    if (curproc->parent != NULL){
      rlist_push_front(& curproc->parent->exited_list, &curproc->exited_node);
      kernel_broadcast(& curproc->parent->child_exit);

    }

    curproc->main_thread = NULL;  


    curproc->state = ZOMBIE;

  }



   

   kernel_sleep(EXITED, SCHED_USER) ;

   return;}
 

   




PTCB* create_ptcb(Task task,int argl, void* args){


    PTCB* ptcb = (PTCB*)xmalloc(sizeof(PTCB));
    rlnode_init(& ptcb->ptcb_list_node,ptcb);
    ptcb->task =task;
    ptcb ->argl = argl;
    ptcb -> ref_count = 1;
    ptcb -> args = args;


    ptcb -> detached  = 0;
    ptcb -> exited = 0 ;
    ptcb -> exit_cv = COND_INIT;

    return ptcb ;
  } 


void start_new_thread(){
   
  int exitval ;
  
  Task call = CURTHREAD->ptcb->task;
  int argl = CURTHREAD -> ptcb->argl;
  void* args = CURTHREAD -> ptcb ->args;
  

  exitval = call(argl,args);
  sys_ThreadExit(exitval);
  
}
