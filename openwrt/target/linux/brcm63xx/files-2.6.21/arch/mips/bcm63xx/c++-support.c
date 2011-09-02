
#include <linux/kernel.h>
#include <linux/module.h>

/* void *operator new(unsigned int sz) */
void *_Znwj(unsigned int sz)
{
	return (kmalloc(sz, GFP_KERNEL));
}

EXPORT_SYMBOL(_Znwj);

/* void *operator new[](unsigned int sz)*/
void *_Znaj(unsigned int sz)
{
	return (kmalloc(sz, GFP_KERNEL));
}

EXPORT_SYMBOL(_Znaj);

/* placement new operator */
/* void *operator new (unsigned int size, void *ptr) */
void *ZnwjPv(unsigned int size, void *ptr)
{
	return ptr;
}

EXPORT_SYMBOL(ZnwjPv);

/* void operator delete(void *m) */
void _ZdlPv(void *m)
{
	kfree(m);
}

EXPORT_SYMBOL(_ZdlPv);

/* void operator delete[](void *m) */
void _ZdaPv(void *m)
{
	kfree(m);
}

EXPORT_SYMBOL(_ZdaPv);
