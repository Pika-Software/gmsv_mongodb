#ifndef POINTER_H
#define POINTER_H

#include "main.h"

template<class T>
class SmartGuard;

template<class T>
class SmartPointer {
	int counter = 1;
	T* ptr;
public:
	explicit SmartPointer() 
	{
		ptr = new T;
	}

	explicit SmartPointer(T* pointer) 
	{
		ptr = pointer;
	}

	~SmartPointer() 
	{
		Global::DevMsg(1, "Deleting pointer...\n");
		delete ptr;
		ptr = nullptr;
	}

	T* get()
	{
		return ptr;
	}

	SmartGuard<T> guard(bool add_counter = true)
	{
		return SmartGuard<T>(this, add_counter);
	}

	void add() {
		counter++;
	}

	void free()
	{
		counter--;
		if (counter < 1)
			delete this;
	}

	operator bool()
	{
		return ptr != nullptr;
	}
};

template<class T>
class SmartGuard {
	SmartPointer<T>* sptr;
public:
	SmartGuard(SmartPointer<T>* smart_pointer, bool add_counter = true) {
		Global::DevMsg(1, "New guard: %p\n", this);
		sptr = smart_pointer;
		if (add_counter)
			sptr->add();
	}

	~SmartGuard() {
		Global::DevMsg(1, "Guard destroy: %p\n", this);
		sptr->free();
	}

	T* operator->()
	{
		return sptr->get();
	}
};

#endif // POINTER_H