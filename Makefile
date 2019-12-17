all: topk_thread_synchron topk_process_synchron

topk_thread_synchron: topk_thread_synchron.c
	gcc topk_thread_synchron.c -o topk_thread_synchron -lpthread

topk_process_synchron: topk_process_synchron.c
	gcc topk_process_synchron.c -o topk_process_synchron -lrt -lpthread

clean:
	rm -fr topk_thread_synchron 
	rm -fr topk_process_synchron
