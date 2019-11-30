#ifndef __PANIC_H__
#define __PANIC_H__

/*! file  @brief Terminate the application unhappily. 
** 
** 
These functions can be used to panic the application, forcing it to terminate abnormally.
*/
/*!
Panics the application if the value passed is FALSE.
*/
#define PanicFalse PanicZero
/*!
Panics the application if the value passed is zero.
*/
#define PanicZero(x) (unsigned int) PanicNull((void *) (x))
/*!
Panics the application if the value passed is not zero.
*/
#define PanicNotZero(x) PanicNotNull((const void *) (x))
/*!
Allocates memory equal to the size of T and returns a pointer to the memory if successful. If the
memory allocation fails, the application is panicked.
*/
#define PanicUnlessNew(T) (T*)PanicUnlessMalloc(sizeof(T))

#if TRAPSET_CORE

/**
 *  \brief Panics the application unconditionally.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void Panic(void );

/**
 *  \brief Panics the application if the pointer passed is NULL, otherwise returns the
 *  pointer.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void * PanicNull(void * );

/**
 *  \brief Panics the application if the pointer passed is not NULL, otherwise returns.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void PanicNotNull(const void * );

/**
 *  \brief Allocates sz words and returns a pointer to the memory if successful. If the
 *  memory allocation fails, the application is panicked.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void * PanicUnlessMalloc(size_t sz);
#endif /* TRAPSET_CORE */
#endif
