# Gromer - Growing container for pointers

Gromer is linear container for storing pointers (objects). The storage
is a continous array.  Pointer storage grows automatically when the
usage count exceeds reservation size. Reservation is doubled at
resizing.

Gromer struct:

    Field     Type          Addr
    -----------------------------
    size      (uint64_t)  | N + 0
    used      (uint64_t)  | N + 8
    data[0]   (void*)     | N + 16

`size` gives the number of pointers that fits currently. `used`
defines the amount of pointer items stored, so far. The size of `data`
field (pointer array), matches `size`.

Base type for Gromer is `gr_t`. `gr_t` is pointer to Gromer
struct. Some functions in Gromer library require a reference to
Gromer. Reference type is `gr_p` and it is a pointer to pointer of
Gromer struct. `gr_p` is required whenever there is possibility that
Gromer might get relocated in memory. This happens when more
reservation is needed for pointer array, or when Gromer is destroyed.

Gromer can be created with default size using `gr_new()`. Default size
is 16. If user knows about usage pattern for Gromer, a specific size
can be requested using `gr_new_sized()` function.

    gr_t gr = gr_new_sized( 128 );

Gromer can be destroyed with:

    gr_destroy( &gr );

`gr_destroy()` requires Gromer reference, since `gr` is set to `NULL`
after destroy.

Items can be added to the end of the container:

    void* data = obj;
    gr_push( &gr, data );

Items can be removed from the end of the container:

    data = gr_pop( gr );

`gr_push()` takes Gromer reference as first argument, since Gromer can
be potentially resized and hence the memory location could be
changing. `gr_pop()` takes Gromer as the argument, since Gromer is not
resized when removing items.

The most ergonomic way of creating a Gromer, is to just start adding
items to it. However the Gromer handle must be initialized to `NULL`
to indicate that the Gromer does not exist yet.

    gr_t gr = NULL;
    gr_add( &gr, data1 );
    gr_add( &gr, data2 );
    ...

Counter operation for `gr_add()` is `gr_remove()`. `gr_remove()`
removes items from Gromer, and if Gromer becomes empty, Gromer is
destroyed and Gromer reference is used to set Gromer handle to `NULL`.

    data = gr_remove( &gr );

If Gromer becomes empty, `gr` is set to `NULL`.

Item can be inserted a to selected position:

    gr_insert_at( &gr, 0, data );

This would add data to the start of container and the existing data
would be shifted towards the end by one slot. If usage count exceeds
reservation size, more is reserved and `gr` is updated. Data can be
inserted also without automatic resizing:

    gr_insert_if( &gr, 10, data );

Items can be deleted from selected positions:

     data = gr_delete_at( gr, 0 );

This would delete the first item from container.

Gromer supports a number of queries. User can query container usage,
size, empty, and full status information. User can also get data from
selected position:

    data = gr_nth( gr, 10 );

Gromer has two find functions. `gr_find()` finds the data as pointer.

    data_idx = gr_find( gr, data );

If data (pointer) exists in Gromer, `data_idx` get the container index
to searched data. Otherwise `data_idx` is assigned an invalid index
(`GR_NOT_INDEX`).

Gromer can also be searched for objects. Search function is provided a
function pointer to compare function that is able to detect whether
the searched item is at current position or not.

    data_idx = gr_find_with( gr, compare_fn, data );


Gromer can also be used within stack allocated memory. First you have
to have some stack storage available. This can be done with a
convenience macro.

    gr_t gr;
    gr_local_use( gr, buf, 16 );

This will initialize `gr` to stack allocated storage where Gromer has
size of 16. Note that the stack allocation is bigger than 16, since
Gromer struct requires storage as well. `gr_local_use` uses `gr_use`
function for assigning `gr` to the storage.

When Gromer is taken into use through `gr_use` it is marked as
"local". This means that it will not be freed with `gr_destroy`. Stack
allocated Gromer is automatically changed to a heap allocated Gromer
if Gromer requires resizing. This is quite powerful optimization,
since often stack allocated memory is enough and heap reservation
(which is slowish) is not needed. `gr_destroy` can be called for
Gromer whether its "local" or not. If Gromer is "local", no memory is
released, but Gromer reference is set to NULL.

Gromer supports non-local memory management. User can allocate a
number of pages of memory.

    gr = gr_new_page( 1 );

Junks of memory can be given to various purposes.

    mem = gr_alloc( gr, 1024 );

Memory is cleared, page aligned and continuous. It provides good cache
locality. `gr_alloc` return NULL when Gromer Page(s) are
consumed. There is no automatic resizing since it is assumed that
client wants to preserve the address returned by `gr_alloc`. Memory is
released with `gr_destroy` when it not needed any more.


By default Gromer library uses malloc and friends to do heap
allocations. If you define GR_MEM_API, you can use your own memory
allocation functions.

Custom memory function prototypes:
    void* gr_malloc ( size_t size );
    void  gr_free   ( void*  ptr  );
    void* gr_realloc( void*  ptr, size_t size );


See Doxygen docs and `gromer.h` for details about Gromer API. Also
consult the test directory for usage examples.


## Gromer API documentation

See Doxygen documentation. Documentation can be created with:

    shell> rake doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building

Ceedling based flow is in use:

    shell> rake

Testing:

    shell> rake test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Gromer uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new gromer

in the directory above Gromer. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
