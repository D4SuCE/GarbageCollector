#include <iostream>

#define STACK_MAX_SIZE 256
#define IGCT 8;

enum class oType
{
	INT,
	TWIN
};

typedef struct sObject
{
	oType type;
	unsigned char marked;

	struct sObject* next;

	union
	{
		int value;

		struct
		{
			struct sObject* head;
			struct sObject* tail;
		};
	};
}Object;

typedef struct
{
	Object* stack[STACK_MAX_SIZE];
	int stackSize;

	Object* firstObject;
	int numObjects;
	int maxObjects;
}vm;

void push(vm* vm, Object* value)
{
	vm->stack[vm->stackSize++] = value;
}

Object* pop(vm* vm)
{
	return vm->stack[--vm->stackSize];
}

vm* newVM()
{
	vm* mainVM = (vm*)malloc(sizeof(vm));
	mainVM->stackSize = 0;
	mainVM->firstObject = NULL;
	mainVM->numObjects = 0;
	mainVM->maxObjects = IGCT;
	return mainVM;
}

void mark(Object* object)
{
	if (object->marked)
	{
		return;
	}

	object->marked = 1;

	if (object->type == oType::TWIN)
	{
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(vm* vm)
{
	for (int i = 0; i < vm->stackSize; i++)
	{
		mark(vm->stack[i]);
	}
}

void marksweep(vm* vm)
{
	Object** object = &vm->firstObject;
	while (*object)
	{
		if (!(*object)->marked)
		{
			Object* unreached = *object;
			*object = unreached->next;
			free(unreached);
			vm->numObjects--;
		}
		else
		{
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
}

void gc(vm* vm)
{
	int numObjects = vm->numObjects;
	markAll(vm);
	marksweep(vm);
	vm->maxObjects = vm->numObjects * 2;
	std::cout << "Удалено " << numObjects - vm->numObjects << " объектов, " << vm->numObjects << " осталось." << std::endl;
}

Object* newObject(vm* vm, oType type)
{
	if (vm->numObjects == vm->maxObjects)
	{
		gc(vm);
	}
	Object* object = (Object*)malloc(sizeof(Object));
	object->type = type;
	object->next = vm->firstObject;
	vm->firstObject = object;
	object->marked = 0;
	vm->numObjects++;
	return object;
}

void pushInt(vm* vm, int intV)
{
	Object* object = newObject(vm, oType::INT);
	object->value = intV;
	push(vm, object);
}

Object* pushTwin(vm* vm)
{
	Object* object = newObject(vm, oType::TWIN);
	object->head = pop(vm);
	object->tail = pop(vm);
	push(vm, object);
	return object;
}


void freeVM(vm* vm)
{
	vm->stackSize = 0;
	gc(vm);
	free(vm);
}

void printObj(Object* object)
{
	switch (object->type)
	{
		case oType::INT:
		{
			std::cout << object->value << std::endl;
			break;
		}			
		case oType::TWIN:
		{
			std::cout << "(";
			printObj(object->head);
			std::cout << ", ";
			printObj(object->tail);
			std::cout << ")" << std::endl;
			break;
		}
	}
}

void first_test()
{
	std::cout << "1: Объекты на стеке сохранены." << std::endl;
	vm* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);

	gc(vm);
	freeVM(vm);
}

void second_test()
{
	std::cout << "2: Недосягаемые объекты удалены." << std::endl;
	vm* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pop(vm);
	pop(vm);

	gc(vm);
	freeVM(vm);
}

void third_test()
{
	std::cout << "3: Дотянуться до вложенных объектов." << std::endl;
	vm* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pushTwin(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	pushTwin(vm);
	pushTwin(vm);

	gc(vm);
	freeVM(vm);
}

void another_test() {
	std::cout << "4: Циклы." << std::endl;
	vm* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	Object* a = pop(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	Object* b = pop(vm);

	a->tail = b;
	b->tail = a;

	gc(vm);
	freeVM(vm);
}

void performance() {
	std::cout << "Производительность GC." << std::endl;
	vm* vm = newVM();

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 20; j++) {
			pushInt(vm, i);
		}

		for (int k = 0; k < 20; k++) {
			pop(vm);
		}
	}
	freeVM(vm);
}

int main()
{
	setlocale(LC_ALL, "Russian");
	first_test();
	second_test();
	third_test();
	another_test();
	performance();
	return 0;
}