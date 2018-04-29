#ifndef GROMER_H
#define GROMER_H

/**
 * @file   gromer.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Mar  3 19:07:07 2018
 *
 * @brief  Gromer - Growing container for pointers.
 *
 */

#include <stdlib.h>
#include <stdint.h>



#ifndef GROMER_NO_ASSERT
#include <assert.h>
/** Default assert. */
#define gr_assert assert
#else
/** Disabled assertion. */
#define gr_assert( cond ) (void)( ( cond ) || gr_void_assert() )
#endif


#ifndef GR_DEFAULT_SIZE
/** Default size for Gromer. */
#define GR_DEFAULT_SIZE 16
#endif

/** Minimum size for pointer array. */
#define GR_MIN_SIZE 2

/** Outsize Gromer index. */
#define GR_NOT_INDEX -1


/** Size type. */
typedef uint64_t gr_size_t;

/** Position type. */
typedef int64_t gr_pos_t;

/** Data pointer type. */
typedef void* gr_d;


/**
 * Gromer struct.
 */
struct gr_struct_s
{
    gr_size_t size;      /**< Reservation size for data (N mod 2==0). */
    gr_size_t used;      /**< Used count for data. */
    gr_d      data[ 0 ]; /**< Pointer array. */
};
typedef struct gr_struct_s gr_s; /**< Gromer struct. */
typedef gr_s*              gr_t; /**< Gromer pointer. */
typedef gr_t*              gr_p; /**< Gromer pointer reference. */


/** Resize function type. */
typedef int ( *gr_resize_fn_p )( gr_p gp, gr_size_t new_size, gr_d state );

/** Compare function type. */
typedef int ( *gr_compare_fn_p )( const gr_d a, const gr_d b );


/** Iterate over all items. */
#define gr_for_each( gr, iter, cast )                                             \
    for ( gr_size_t gr_idx = 0;                                                   \
          ( iter = ( cast )( gr )->data[ gr_idx ] ) && ( gr_idx < ( gr )->used ); \
          gr_idx++ )

/** Item at index with casting to target type. */
#define gr_item( gr, idx, cast ) ( ( cast )( gr )->data[ ( idx ) ] )

/** Item at index with casting to target type. */
#define gr_assign( gr, idx, item ) ( ( gr )->data[ ( idx ) ] = ( item ) )

/** Shift item from first index. */
#define gr_shift( gp ) gr_delete_at( gp, 0 )

/** Insert item to first index. */
#define gr_unshift( gp, item ) gr_insert_at( gp, 0, item )

/** Gromer struct size. */
#define gr_struct_size( size ) ( sizeof( gr_s ) + ( size ) * sizeof( gr_d ) )

/** Make allocation for local (stack) Gromer. */
#define gr_local_use( gr, buf, size )   \
    gr_d buf[ gr_struct_size( size ) ]; \
    gr_use( &gr, buf, gr_struct_size( size ) )



/* Short names for functions. */

/** @cond gromer_none */
#define grnew gr_new
#define grsiz gr_new_sized
#define grdes gr_destroy
#define grres gr_resize
#define gruse gr_used
#define grrss gr_size
#define grdat gr_data
#define grfst gr_first
#define grlst gr_last
#define grnth gr_nth
#define gremp gr_is_empty
#define grfll gr_is_full
#define grpsh gr_push
#define grpop gr_pop
#define gradd gr_add
#define grrem gr_remove
#define grrst gr_reset
#define grdup gr_duplicate
#define grswp gr_swap
#define grins gr_insert_at
#define griif gr_insert_if
#define grdel gr_delete
#define grfnd gr_find
#define grfnw gr_find_with

#define grfor gr_for_each
/** @endcond gromer_none */



#ifdef GROMER_MEM_API

/*
 * GR_MEM_API allows to use custom memory allocation functions,
 * instead of the default: gr_malloc, gr_free, gr_realloc.
 *
 * If GR_MEM_API is used, the user must provide implementation for the
 * above functions and they must be compatible with malloc etc. Also
 * Gromer assumes that gr_malloc sets all new memory to zero.
 *
 * Additionally user should compile the library by own means.
 */

extern void* gr_malloc( size_t size );
extern void gr_free( void* ptr );
extern void* gr_realloc( void* ptr, size_t size );

#else

/* Default to common memory management functions. */

/** Reserve memory. */
#define gr_malloc( size ) calloc( size, 1 )

/** Release memory. */
#define gr_free free

/** Re-reserve memory. */
#define gr_realloc realloc

#endif


/* ------------------------------------------------------------
 * Create and destroy:
 */


/**
 * Create Gromer container with default size (GR_DEFAULT_SIZE).
 *
 * @return Gromer.
 */
gr_t gr_new( void );


/**
 * Create Gromer with size.
 *
 * Minimum size is GR_MINSIZE. Size is forced if given size is too
 * small.
 *
 * @param size Initial size.
 *
 * @return Gromer.
 */
gr_t gr_new_sized( gr_size_t size );


/**
 * Use existing memory allocation for Gromer.
 *
 * @param gp   Gromer reference.
 * @param mem  Allocation for Gromer.
 * @param size Allocation size (in bytes).
 */
void gr_use( gr_p gp, gr_d mem, gr_size_t size );


/**
 * Destroy Gromer.
 *
 * @param gp Gromer reference.
 */
void gr_destroy( gr_p gp );


/**
 * Resize Gromer to new_size.
 *
 * If new_size is smaller than usage, no action is performed.
 *
 * @param gp       Gromer reference.
 * @param new_size Requested size.
 */
void gr_resize( gr_p gp, gr_size_t new_size );


/**
 * Push item to end of container.
 *
 * gr_push() can be used for efficient stack operations. Gromer is
 * resized if these is no space available.
 *
 * @param gp   Gromer reference.
 * @param item Item to push.
 */
void gr_push( gr_p gp, gr_d item );


/**
 * Pop item from end of container.
 *
 * gr_pop() can be used for efficient stack operations. No operation
 * is Gromer is empty or Gromer is NULL.
 *
 * @param gr Gromer.
 *
 * @return Popped item (or NULL).
 */
gr_d gr_pop( gr_t gr );


/**
 * Add item to end of container.
 *
 * If "*gp" is NULL, Gromer is created and item is added. Gromer is resized
 * if these is no space available.
 *
 * @param gp   Gromer reference.
 * @param item Item to add.
 */
void gr_add( gr_p gp, gr_d item );


/**
 * Remove item from end of container.
 *
 * If container becomes empty, it will be destroyed.
 *
 * @param gp Gromer reference.
 *
 * @return Popped item (or NULL).
 */
gr_d gr_remove( gr_p gp );


/**
 * Reset Gromer to empty.
 *
 * @param gr Gromer.
 */
void gr_reset( gr_t gr );


/**
 * Duplicate Gromer.
 *
 * @param gr Gromer to duplicate.
 *
 * @return Duplicated gromer.
 */
gr_t gr_duplicate( gr_t gr );


/**
 * Swap item in Gromer with given "item".
 *
 * @param gr   Gromer.
 * @param pos  Addressed item position.
 * @param item Item to swap in.
 *
 * @return Item swapped out.
 */
gr_d gr_swap( gr_t gr, gr_size_t pos, gr_d item );


/**
 * Insert item to given position.
 *
 * Gromer is resized if these is no space available.
 *
 * @param gp   Gromer reference.
 * @param pos  Position.
 * @param item Item to insert.
 */
void gr_insert_at( gr_p gp, gr_size_t pos, gr_d item );


/**
 * Insert item to given position.
 *
 * Gromer is not resized. Item is not inserted if it does not fit.
 *
 * @param gr   Gromer.
 * @param pos  Position.
 * @param item Item to insert.
 *
 * @return 1 if item was inserted.
 */
int gr_insert_if( gr_t gr, gr_size_t pos, gr_d item );


/**
 * Delete item from position.
 *
 * Gromer is not destroyed if last item is deleted.
 *
 * @param gr  Gromer.
 * @param pos Position.
 *
 * @return Item from delete position.
 */
gr_d gr_delete_at( gr_t gr, gr_size_t pos );


/**
 * Sort Gromer items.
 *
 * @param gr      Gromer.
 * @param compare Compare function.
 */
void gr_sort( gr_t gr, gr_compare_fn_p compare );


/**
 * Return count of container usage.
 *
 * @param gr Gromer.
 *
 * @return Usage count.
 */
gr_size_t gr_used( gr_t gr );


/**
 * Return Gromer reservation size.
 *
 * @param gr Gromer.
 *
 * @return Reservation size.
 */
gr_size_t gr_size( gr_t gr );


/**
 * Return container data array.
 *
 * @param gr Gromer.
 *
 * @return Data array.
 */
gr_d* gr_data( gr_t gr );


/**
 * Return first item.
 *
 * @param gr Gromer.
 *
 * @return First item.
 */
gr_d gr_first( gr_t gr );


/**
 * Return last item.
 *
 * @param gr Gromer.
 *
 * @return Last item.
 */
gr_d gr_last( gr_t gr );


/**
 * Return nth item.
 *
 * @param gr  Gromer.
 * @param pos Item position.
 *
 * @return Indexed item.
 */
gr_d gr_nth( gr_t gr, gr_size_t pos );


/**
 * Return empty status.
 *
 * @param gr Gromer.
 *
 * @return 1 if empty, 0 if not.
 */
int gr_is_empty( gr_t gr );


/**
 * Return full status.
 *
 * @param gr Gromer.
 *
 * @return 1 if full, 0 if not.
 */
int gr_is_full( gr_t gr );


/**
 * Find item from Gromer.
 *
 * @param gr   Gromer.
 * @param item Item to find.
 *
 * @return Item index (or GR_NOT_INDEX).
 */
gr_pos_t gr_find( gr_t gr, gr_d item );


/**
 * Find item from Gromer using compare function.
 *
 * @param gr      Gromer.
 * @param compare Compare function.
 * @param ref     Item to find.
 *
 * @return Item index (or GR_NOT_INDEX).
 */
gr_pos_t gr_find_with( gr_t gr, gr_compare_fn_p compare, gr_d ref );


/**
 * Set Gromer as local.
 *
 * @param gr  Gromer.
 * @param val Local (or not).
 */
void gr_set_local( gr_t gr, int val );


/**
 * Return Gromer local mode.
 *
 * @param gr Gromer.
 *
 * @return 1 if local (else 0).
 */
int gr_get_local( gr_t gr );


#endif
