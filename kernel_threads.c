
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include "util.h"
#include "kernel_streams.h"






/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args){
    PTCB* ptcb ;
    if (task != NULL){
      ptcb = create_ptcb(task,argl,args);
      TCB* tcb = spawn_thread(CURPROC,start_new_thread);
      ptcb->tcb = tcb;
      tcb ->ptcb = ptcb;
      
      rlist_push_back( &CURPROC ->ptcb_list ,& ptcb -> ptcb_list_node);
      CURPROC -> thread_count++;

      wakeup(ptcb->tcb);
      return (Tid_t)ptcb;

   }
   return NOTHREAD;
}
   
     
   PTCB* find_PTCB_withTID(Tid_t tid){ 
    PTCB* ptcb = (PTCB*)tid ;

    if(rlist_find(&(CURPROC->ptcb_list),ptcb,NULL)){ //Searching the ptcb_list of Current Process ,for ptcb using the tid of each ptcb
      return ptcb;
    }
    else
      return NULL;

   }


	


/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) cur_thread()->ptcb;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval){

  PTCB* ptcb = find_PTCB_withTID(tid)
  if(ptcb==NULL)
    return -1;

  if (tid == (Tid_t)CURTHREAD || ptcb->detached==1 || ptcb->exited==1 || CURTHREAD->owner_pcb != ptcb->tcb->owner_pcb)
      return -1;

    ptcb->refcount++;


    while(ptcb->exited==0 && ptcb->detached==0){
      kernel_wait(& ptcb->exit_cv,SCHED_USER);

    }
   if(ptcb->detached==1){
    ptcb-> refcount--;
    if(ptcb->refcount==0) free(ptcb);
    return -1;

   }


    if(exitval){
      exitval = &(ptcb->exitval);
    }
    ptcb->refcount--;
     if(ptcb->refcount==0) free(ptcb);

    return 0;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid){
  
  PTCB* ptcb = find_PTCB_withTID(tid)
  if(ptcb==NULL)
    return -1;

  if(ptcb->exited==1)
    return -1;
  if (ptcb->detached==1)
    return -1;

  ptcb->detached==1;
  kernel_broadcast(&ptcb->exit_cv);
  return 0;	
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval){


  CURTHREAD->ptcb->exited=1 ;
  CURTHREAD->ptcb->exitval=exitval;
 
  kernel_broadcast(&CURTHREAD->ptcb->exit_cv);
  // rlist_remove(&(CURTHREAD ->ptcb)-> ptcb_list_node);

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

   return;
 
}}
   




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
  TCB* curthread = cur_thread();
  
  Task call = curthread->ptcb->task;
  int argl = curthread-> ptcb->argl;
  void* args = curthread -> ptcb ->args;
  

  exitval = call(argl,args);
  sys_ThreadExit(exitval);
  
}
