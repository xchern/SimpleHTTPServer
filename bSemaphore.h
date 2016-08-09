#include <pthread.h>

class BSemaphore {
	public:
		BSemaphore (int init_status = 1) {
			block_flag = !init_status;
			pthread_mutex_init(&mtx, NULL);
			pthread_cond_init(&cv, NULL);
		}
		~BSemaphore () {
			pthread_mutex_destroy(&mtx);
			pthread_cond_destroy(&cv);
		}
		void wait(void) {
			pthread_mutex_lock(&mtx);
			while (block_flag) pthread_cond_wait(&cv, &mtx);
			block_flag = true;
			pthread_mutex_unlock(&mtx);
		}
		void signal(void) {
			pthread_mutex_lock(&mtx);
			block_flag = false;
			pthread_cond_signal(&cv);
			pthread_mutex_unlock(&mtx);
		}
	protected:
		bool block_flag;
		pthread_mutex_t mtx;
		pthread_cond_t cv;
};
