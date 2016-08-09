#pragma once

#include <new>

template <typename T>
class Queue {
	public:
		Queue() {
			head.next = NULL;
			tail = & head;
		}
		~Queue() {
			clear();
		}
		void enQueue(T value) {
			tail = (tail->next = new struct Node);
			tail->next = NULL;
			tail->value = value;
		}
		T deQueue(void) {
			struct Node * first = head.next;
			T value = first->value;
			head.next = first->next;
			if (tail == first) tail = &head;
			delete first;
			return value;
		}
		T & top(void) {
			return *head.next;
		}
		void clear (void) {
			while (head.next) {
				deQueue();
			}
		}
		inline bool emptyP(void) {
			return !head.next;
		}
	protected:
		struct Node {
			T value;
			struct Node * next;
		} head, *tail;
};
