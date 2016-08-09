#pragma once

#include <pthread.h>

#include "queue.h"
#include "bSemaphore.h"

template <typename Task>
class TaskQueue : protected Queue<Task>{
	public:
		TaskQueue(void (*_workerFunction)(Task), int _threadCount = 4) {
			workerFunction = _workerFunction;
			threadCount = _threadCount;
			pthread_mutex_init(&queue_lock, NULL);
			// set manager
			manager_blocker = new BSemaphore(0);
			pthread_create(&manager_thread, NULL, manager, this);
		}
		~TaskQueue() {
			// get all tasks arranged and excuted
			shutdown = true;
			manager_blocker->signal();
			void * status;
			pthread_join(manager_thread, &status);
			pthread_mutex_destroy(&queue_lock);
			delete manager_blocker;
		}
		void addTask(Task task) {
			pthread_mutex_lock(&queue_lock);
			this->enQueue(task);
			pthread_mutex_unlock(&queue_lock);
			// wake up manager
			manager_blocker->signal();
		}
	protected:
		volatile bool shutdown = false;

		pthread_mutex_t queue_lock;

		BSemaphore * manager_blocker;
		pthread_t manager_thread;
		static void * manager(void * obj) {
			TaskQueue<Task> * that = (TaskQueue<Task> *) obj;
			// set workers
			that->workers = new struct workerCtrl[that->threadCount];
			for (int worker_id = that->threadCount - 1; worker_id >= 0; worker_id--) {
				that->workers[worker_id].that = that;
				// no task now
				that->workers[worker_id].working = false;
				that->workers[worker_id].blocker = new BSemaphore(0);
				// set worker thread
				pthread_create(&that->workers[worker_id].this_thread, NULL,
						worker, that->workers + worker_id);
			}

			// main loop
			for (;;) {
				int worker_id;
				Task task;
				// block while no avaliable worker or task
				that->manager_blocker->wait();
				for (;;) {
					// try get an avaliable worker
					for (worker_id = 0; worker_id < that->threadCount; worker_id++)
						if (!that->workers[worker_id].working) break;
					// back to block with no worker
					if (!(worker_id < that->threadCount)) break;

					// try get task
					pthread_mutex_lock(&that->queue_lock);
					if (!that->emptyP()) {// if there is task
						task = that->deQueue();
					} else {// back to block without task
						pthread_mutex_unlock(&that->queue_lock);
						if (that->shutdown) goto WAIT_FINISH;
						break;
					}
					pthread_mutex_unlock(&that->queue_lock);

					// there we have avaliable worker and task
					// just do it!
					struct workerCtrl * ctrl = &that->workers[worker_id];
					ctrl->task = task;
					ctrl->working = true;
					// wake up the worker
					ctrl->blocker->signal();
				}
			}
WAIT_FINISH:
			bool finished = false;
			for (;;) {
				finished = true;
				// check if there is worker working
				for (int worker_id = 0; worker_id < that->threadCount; worker_id++)
					if (that->workers[worker_id].working) finished = false;
				if (finished) break;
				// block for nonfinished work
				that->manager_blocker->wait();
			}
				for (int worker_id = 0; worker_id < that->threadCount; worker_id++) {
					delete that->workers[worker_id].blocker;
				}
			// TODO release worker
			delete that->workers;
			return NULL;
		}

		struct workerCtrl {
			TaskQueue<Task> * that;
			Task task;
			volatile bool working;
			BSemaphore * blocker;
			pthread_t this_thread;
		} * workers;
		int threadCount;

		void (*workerFunction)(Task);
		static void * worker(void * workerctrl) {
			struct workerCtrl * ctrl = (struct workerCtrl *) workerctrl;
			for (;;) {
				// block without task
				ctrl->blocker->wait();
				// do task
				ctrl->that->workerFunction(ctrl->task);
				ctrl->working = false;
				// unblock blocked manager
				ctrl->that->manager_blocker->signal();
			}
			return NULL;
		}
};
