I've seen many posts on StackOverflow about dynamic memory allocation (DMA) in C++, questions about "new operator" vs "operator new", questions about `new int(100)` vs `new int[100]`, questions about memory initialization... I think there should be an answer that summarizes everything clearly once and for all, and I'd like to give it a go. This is a very simple implementation that defines "allocators" (NOT comparable to the standard ones), using the various ways at our disposal for DMA from C and C++98.

---

C vs C++
==

Main functions for dynamic memory allocations:

 - In C (header `<cstdlib>`), we have mainly `malloc` and `calloc` and `free`. I won't talk about `realloc`.
 - in C++ (header `<new>`), we have:
   - Template single-object allocation with initialization arguments:
      - `new T( args )` 
      - `new (std::nothrow) T( args )` 
      - `delete ( T* )`
   - Template multiple-objects allocation with default initialization:
      - `new T[ size_t ]`
      - `new (std::nothrow) T[ size_t ]`
      - `delete[] ( T* )`
   - Template memory initialization without allocation for single or multiple objects:
      - `new (void*) T( args )` 
      - `new (void*) T[ size_t ]`
   - Internal _new-expressions_ for:
      - Raw memory allocation `::operator new( size_t )`;
      - Raw memory allocation without exception `::operator new( size_t, std::nothrow )`;
      - Raw memory initialization without allocation `::operator new( size_t, ptr )`.

Please look at [this post][2] for a concise comparison.

---

Legacy C dynamic allocations
==

**_Main points_**: complete type-erasure (`void*` pointers), and therefore **no construction/destruction**, size specified in bytes (typically using `sizeof`).

[`malloc( size_t )`][3] does not initialize memory at all (raw memory contains garbage, always initialize manually before use). [`calloc( size_t, size_t )`][4] initializes all bits to 0 (slight overhead, but useful for POD numeric types). Any allocated memory should be released using [`free`][5] **ONLY**. 

Construction/destruction of class instances **should be done manually** _before_ use / _before_ memory release.

---

C++ dynamic allocations
==

**_Main points_**: confusing because of similar syntaxes doing different things, **all** `delete`-statements call the destructor, **all** `delete`-statements take fully typed pointers, **some** `new`-statements return fully-typed pointers, **some** `new`-statements call _some_ constructor.

**_Warning_**: as you will see below, `new` can either be a _keyword_ OR _function_. It is best not to talk about "new operator" and/or "operator new" in order to [avoid confusions][6]. I call "`new`-statements" any valid statements that contain `new` either as a function or keyword. People also talk about "`new`-expressions", where `new` is the keyword and not the function.

## Raw memory allocation (no initialization)

**Do not use this yourself.** This is used internally by _new-expressions_ (see below).

 - `::operator new( size_t )` and `::operator new( size_t, std::nothrow )` take a size in bytes, and return a `void*` in case of success.
 - In case of failure, the former throws an exception [`std::bad_alloc`][7], the latter returns `NULL`.
 - Use [`::operator new( sizeof(T) )`][8] for a **single** object of type `T` (and [`delete`][9] for release), and [`::operator new( n*sizeof(T) )`][10] for **multiple** objects (and [`delete[]`][11] for release).

These allocations **do not** initialize memory, and in particular, they **do not** call the default-constructor on the allocated objects. Therefore you **MUST initialize ALL the elements manually** before you release the allocation using either `delete` or `delete[]`.

_**Note**_: I couldn't stress enough that you should NOT use this yourself. If you should use it, however, make sure you pass a pointer to `void` instead of a typed pointer when calling either `delete` or `delete[]` on such allocations (always after initializing manually). I have personally experienced runtime errors with non-POD types with some compilers (maybe my mistake).

## Raw memory initialization (no allocation)

**Do not use this yourself.** This is used internally by _new-expressions_ (see below).
In the following, I assume `void *ptr = ::operator new( n*sizeof(T) )` for some type `T` and size `n`.

Then `::operator new( n*sizeof(T), (T*) ptr )` initializes `n` elements of type `T` starting from `ptr` using the default constructor `T::T()`. There is **no allocation** here, only initialization using the default-constructor.

## Single-object allocation & initialization

 - `new T( args )` allocates _and_ initializes memory for a single object of type `T` using the constructor `T::T( args )`. The default constructor will not be called _unless_ arguments are omitted (ie `new T()` or even `new T`). Throws an exception `std::bad_alloc` on failure.
 - Same for `new (std::nothrow) T( args )` except that it returns `NULL` in case of failure.
 - Use `delete` to call the destructor `T::~T()` and release the corresponding memory.

## Multiple-objects allocation & initialization

 - `new T[n]` allocates _and_ initializes memory for a `n` objects of type `T` using the default constructor. Throws an exception `std::bad_alloc` on failure.
 - Idem for `new (std::nothrow) T[n]` except that it returns `NULL` in case of failure.
 - Use `delete[]` to call the destructor `T::~T()` _for each element_ and release the corresponding memory.

_**Note**_: your should use `delete` and not `delete[]` if you call `new T[1]` or the corresponding no-throw variant.

## Memory initialization (aka "placement new")

No allocation here. Regardless of how the allocation was made: 

 - `new (ptr) T(args)` calls the constructor `T::T(args)` on the memory stored at `ptr`. The default constructor is not called unless arguments are omitted.
 - `new (ptr) T[n]` calls the default constructor `T::T()` on `n` objects of type `T` stored from `ptr` to `ptr+n` (ie, `n*sizeof(T)` bytes).

---

Related posts
==

 - Concise comparison [new/delete vs malloc/free][12] 
 - More verbose [Malloc vs new][13], look at the answer of @Flexo
 - [New operator vs operator new][14], avoid the confusion by not using these terms


  [1]: https://github.com/Sheljohn/Allocators
  [2]: http://stackoverflow.com/questions/240212/what-is-the-difference-between-new-delete-and-malloc-free
  [3]: http://www.cplusplus.com/reference/cstdlib/malloc/
  [4]: http://www.cplusplus.com/reference/cstdlib/calloc/
  [5]: http://www.cplusplus.com/reference/cstdlib/free/
  [6]: http://stackoverflow.com/questions/1885849/difference-between-new-operator-and-operator-new
  [7]: http://www.cplusplus.com/reference/new/bad_alloc/
  [8]: http://www.cplusplus.com/reference/new/operator%20new/
  [9]: http://www.cplusplus.com/reference/new/operator%20delete/
  [10]: http://www.cplusplus.com/reference/new/operator%20new[]/
  [11]: http://www.cplusplus.com/reference/new/operator%20delete[]/
  [12]: http://stackoverflow.com/questions/240212/what-is-the-difference-between-new-delete-and-malloc-free
  [13]: http://stackoverflow.com/questions/184537/in-what-cases-do-i-use-malloc-vs-new
  [14]: http://stackoverflow.com/questions/1885849/difference-between-new-operator-and-operator-new
