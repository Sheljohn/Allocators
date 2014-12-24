#ifndef __ALLOCATORS__
#define __ALLOCATORS__

#include <stdexcept>
#include <cstdlib>
#include <new>



		/********************     **********     ********************/
		/********************     **********     ********************/



/**
 * Basic type-traits.
 */
template <class T>
struct core_traits
{
	typedef T  val_t;
	typedef T& ref_t;
	typedef T* ptr_t;

	typedef const T  cval_t;
	typedef const T& cref_t;
	typedef const T* cptr_t;
};

#define FORWARD_TRAITS(Class) \
typedef typename Class::val_t val_t; \
typedef typename Class::ref_t ref_t; \
typedef typename Class::ptr_t ptr_t; \
typedef typename Class::cval_t cval_t; \
typedef typename Class::cref_t cref_t; \
typedef typename Class::cptr_t cptr_t; 

#define CORE_TRAITS(Type) FORWARD_TRAITS( core_traits<Type> )

// ------------------------------------------------------------------------

/**
 * Static default-constructor / destructor.
 */
template <class T>
struct constructor
{
	CORE_TRAITS(T);
	typedef constructor<T> self;

	static void construct( ptr_t p, unsigned n = 1 )
	{
		for ( unsigned i = 0; i < n; ++i, ++p ) 
			new (p) T();
	}

	static void destroy( ptr_t p, unsigned n = 1 )
	{
		for ( unsigned i = 0; i < n; ++i, ++p ) 
			p->~T();
	}
};

// ------------------------------------------------------------------------

/**
 * Allocators tags.
 */
struct tag_allocator_noalloc {};
struct tag_allocator_new {};
struct tag_allocator_malloc {};
struct tag_allocator_calloc {};

// ------------------------------------------------------------------------

/**
 * Allocation attempts throw an exception std::runtime_error.
 */
template <class T>
struct allocator_noalloc
{
	CORE_TRAITS(T);
	typedef allocator_noalloc<T> self;
	typedef tag_allocator_noalloc tag;

	static ptr_t alloc( unsigned n )
	{
		throw std::runtime_error(
			"ERROR in allocator_noalloc::alloc\n\t"
			"this allocator prohibits allocations."
		);
	}

	static void free( ptr_t ptr, unsigned n )
	{
		// do nothing
	}
};

// ------------------------------------------------------------------------

/**
 * Allocator using new/delete.
 */
template <class T>
struct allocator_new
{
	CORE_TRAITS(T);
	typedef allocator_new<T> self;
	typedef tag_allocator_new tag;

	static ptr_t alloc( unsigned n )
	{
		ptr_t ptr = NULL;

		if ( n > 0 && (ptr = (ptr_t)(::operator new( n*sizeof(T), std::nothrow ))) )
			constructor<T>::construct( ptr, n );

		return ptr;
	}

	static void free( ptr_t ptr, unsigned n )
	{ if ( ptr && n ) {

		// constructor<T>::destroy( ptr, n ); // called by delete
		if (n > 1)
			delete[] ptr;
		else
			delete ptr;
	} }
};

// ------------------------------------------------------------------------

/**
 * Allocator using malloc.
 */
template <class T>
struct allocator_malloc
{
	CORE_TRAITS(T);
	typedef allocator_malloc<T> self;
	typedef tag_allocator_malloc tag;

	static ptr_t alloc( unsigned n )
	{
		ptr_t ptr = NULL;
		
		if ( n > 0 && (ptr = (ptr_t) std::malloc( n*sizeof(T) )) )
			constructor<T>::construct( ptr, n );

		return ptr;
	}

	static void free( ptr_t ptr, unsigned n )
	{ if ( ptr && n ) {

		constructor<T>::destroy( ptr, n );
		std::free( (void*) ptr );			
	} }
};

// ------------------------------------------------------------------------

/**
 * Allocator using calloc.
 */
template <class T>
struct allocator_calloc
{
	CORE_TRAITS(T);
	typedef allocator_calloc<T> self;
	typedef tag_allocator_calloc tag;

	static ptr_t alloc( unsigned n )
	{
		ptr_t ptr = NULL;
		
		if ( n > 0 && (ptr = (ptr_t) std::calloc( n, sizeof(T) )) )
			constructor<T>::construct( ptr, n );

		return ptr;
	}

	static void free( ptr_t ptr, unsigned n )
	{ if ( ptr && n ) {

		constructor<T>::destroy( ptr, n );
		std::free( (void*) ptr );
	} }
};

#endif