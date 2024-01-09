
#include "tinyos.h"
#include "kernel_dev.h"
#include "kernel_streams.h"
#include "kernel_socket.h"
#include "kernel_cc.h"
#include "kernel_sched.h"


void initializePipe(pipe_cb* pipecb){
	pipecb->hasData = COND_INIT;
	pipecb->hasSpace = COND_INIT;
	pipecb->wPosition = 0;
	pipecb->rPosition = 0;
	pipecb->availableBytes = 0;

}

int  isEmpty(pipe_cb* pipecb){
	if (pipecb->availableBytes==0)
		return 1;
	return 0;
			
}

int isFull(pipe_cb* pipecb){
	if (pipecb->availableBytes==PIPE_BUFFER_SIZE)
		return 1;
	return 0;

}

int goRight(int p){
	return (p+1)%PIPE_BUFFER_SIZE ;

}
int returnError(){
	return -1;
}

int pipe_write(void* pipecb_t, const char* buf, unsigned int n){
	if(pipecb_t==NULL||buf==NULL)
		return -1;
	pipe_cb* pipecb = pipecb_t;
	int w = pipecb->wPosition;
	int counter = 0;
	

	 if(pipecb->writer==NULL) {
		 return -1;//if writer is closed,we don't write
	} 

	if(pipecb->reader==NULL) {
		 return -1;//if writer is closed,we don't write
	} 

	while(isFull(pipecb)==1 && pipecb->reader!=NULL){
		kernel_wait(&pipecb->hasSpace,SCHED_PIPE);

	}
	if(pipecb->reader==NULL) 
		 return -1;//if writer is closed,we don't write

	while(counter<n){


		if(isFull(pipecb)==1 && pipecb->reader==NULL){
			pipecb->wPosition = w;
			return counter;

		}
		while(isFull(pipecb)==1){
			pipecb->wPosition=w;
			kernel_broadcast(&pipecb->hasData);
			kernel_wait(&pipecb->hasSpace,SCHED_PIPE);
		}

		pipecb->BUFFER[w] = buf[counter];
		w = goRight(w);
		counter ++ ;
		pipecb->availableBytes++;

	}

	pipecb->wPosition=w;
	kernel_broadcast(&pipecb->hasData);
	return counter;


}

int pipe_read(void* pipecb_t, char* buf, unsigned int n){
	if(pipecb_t==NULL||buf==NULL)
		return -1;
	pipe_cb* pipecb = pipecb_t;
	int r=pipecb->rPosition;
	int counter=0;

	if(pipecb->reader==NULL) {
		 return -1;//if writer is closed,we don't write
	} 

	if(isEmpty(pipecb)==1 && pipecb->writer==NULL)
		return 0;

	while(isEmpty(pipecb)==1 && pipecb->writer!=NULL)
		kernel_wait(&pipecb->hasData,SCHED_PIPE);


	while(counter<n){

		if(isEmpty(pipecb)==1 && pipecb->writer==NULL){
			pipecb->rPosition= r;
			return counter;

		}
		while(isEmpty(pipecb)==1 ){
			pipecb->rPosition=r;
		    kernel_broadcast(&pipecb->hasSpace);
		    kernel_wait(&pipecb->hasData,SCHED_PIPE);
		}

		buf[counter] = pipecb->BUFFER[r];
		r = goRight(r);
		counter++;
		pipecb->availableBytes--;
	}

  pipecb->rPosition=r;
  kernel_broadcast(&pipecb->hasSpace);
  return counter;
}


int pipe_writer_close(void* _pipecb){
	if(_pipecb==NULL)
		return -1;
	pipe_cb* pipecb = _pipecb ;
	if(!pipecb)
		return -1;
	pipecb->writer=NULL;
	kernel_broadcast(&pipecb->hasData);
	if(!pipecb->reader && !pipecb)
		free(pipecb);

	return 0;
}

int pipe_reader_close(void* _pipecb){
	if(_pipecb==NULL)
		return -1;
	pipe_cb* pipecb = _pipecb ;
	if(!pipecb)
		return -1;
	pipecb->reader=NULL;
	kernel_broadcast(&pipecb->hasSpace);
	if(!pipecb->writer && !pipecb)
		free(pipecb);

	return 0 ;
}

void* open(){
	return NULL;
}

 static file_ops reader_file_ops = {
 	.Open = open ,
 	.Read = pipe_read ,
 	.Write = returnError,
 	.Close = pipe_reader_close 

};

static file_ops  writer_file_ops = {
	.Open = open,
	.Read = returnError,
	.Write = pipe_write,
	.Close = pipe_writer_close
};


int sys_Pipe(pipe_t* pipe ){
	Fid_t fid[2];
	FCB*  fcb[2];
	if(FCB_reserve(2,fid,fcb)==0){
		return -1;
	}
	pipe_cb* pipecb= xmalloc(sizeof(pipe_cb));
	initializePipe(pipecb);

	pipe->read=fid[0];
	pipe->write=fid[1];


	pipecb->reader= fcb[0];
	pipecb->writer= fcb[1];

	fcb[0]->streamobj=pipecb;
	fcb[1]->streamobj=pipecb;

	fcb[0]->streamfunc= &reader_file_ops;
	fcb[1]->streamfunc= &writer_file_ops;

	return 0;





}
