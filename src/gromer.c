/**
 * @file   gromer.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sat Mar  3 19:07:07 2018
 *
 * @brief  Gromer - Growing container for pointers.
 *
 */

#include <string.h>
#include <stdlib.h>
#include "gromer.h"


/* clang-format off */

/** @cond gromer_none */
#define gr_true  1
#define gr_false 0

#define gr_smsk            0xFFFFFFFFFFFFFFFEL

#define gr_unit_size       ( sizeof( void* ) )
#define gr_byte_size( gr ) ( gr_unit_size * gm_size( gr) )
#define gr_used_size( gr ) ( gr_unit_size * gr->used )
#define gr_incr_size( gr ) ( 2*(gm_size(gr)) )

#define gr_snor(size)      (((size) & 0x1L) ? (size) + 1 : (size))
#define gr_local( gr )     ( (gr)->size & 0x1L )

#define gm_any( gr ) (     ( gr )->used > 0 )
#define gm_empty( gr )     ( ( gr )->used == 0 )
#define gm_used( gr )      ( gr )->used
#define gm_size( gr )      ( ( gr )->size & gr_smsk )
#define gm_data( gr )      ( gr )->data
#define gm_end( gr )       ( gr )->data[ ( gr )->used ]
#define gm_last( gr )      ( gr )->data[ ( gr )->used - 1 ]
#define gm_first( gr )     ( gr )->data[ 0 ]
#define gm_nth( gr, pos )  ( gr )->data[ ( pos ) ]

/** @endcond gromer_none */

/* clang-format on */



static gr_size_t gr_legal_size( gr_size_t size );
static gr_size_t gr_norm_idx( gr_t gr, gr_size_t idx );
static void gr_resize_to( gr_p gp, gr_size_t new_size );



/* ------------------------------------------------------------
 * Create and destroy:
 */


gr_t gr_new( void )
{
    return gr_new_sized( GR_DEFAULT_SIZE );
}


gr_t gr_new_sized( gr_size_t size )
{
    gr_t gr;

    size = gr_legal_size( size );

    gr = (gr_t)gr_malloc( gr_struct_size( size ) );
    gr->size = size;
    gr_set_local( gr, 0 );
    gr->used = 0;
    gr->data[ 0 ] = NULL;

    return gr;
}


void gr_use( gr_p gp, void* mem, gr_size_t size )
{
    gr_assert( ( size % sizeof( void* ) ) == 0 );
    gr_assert( size >= gr_struct_size( GR_MIN_SIZE ) );

    gr_t gr;
    gr = (gr_t)mem;
    gr->size = ( size - sizeof( gr_s ) ) / sizeof( void* );
    gr_set_local( gr, 1 );
    gr->used = 0;
    gr->data[ 0 ] = NULL;
    *gp = gr;
}


void gr_destroy( gr_p gp )
{
    if ( *gp == NULL )
        return;

    if ( !gr_local( *gp ) )
        gr_free( *gp );

    *gp = NULL;
}


void gr_resize( gr_p gp, gr_size_t new_size )
{
    new_size = gr_legal_size( new_size );

    if ( new_size >= gm_used( *gp ) )
        gr_resize_to( gp, new_size );
}


void gr_push( gr_p gp, void* item )
{
    gr_size_t new_used = gm_used( *gp ) + 1;

    if ( new_used > gm_size( *gp ) )
        gr_resize_to( gp, gr_incr_size( *gp ) );

    gm_nth( *gp, gm_used( *gp ) ) = item;
    gm_used( *gp ) = new_used;
}


void* gr_pop( gr_t gr )
{
    if ( gm_any( gr ) ) {
        void* ret = gm_last( gr );
        gm_used( gr )--;
        if ( gm_empty( gr ) )
            gm_first( gr ) = NULL;
        return ret;
    } else {
        return NULL;
    }
}


void gr_add( gr_p gp, void* item )
{
    if ( *gp == NULL )
        *gp = gr_new();
    gr_push( gp, item );
}


void* gr_remove( gr_p gp )
{
    void* ret;

    if ( *gp ) {
        ret = gr_pop( *gp );
        if ( gm_empty( *gp ) )
            gr_destroy( gp );
        return ret;
    } else {
        return NULL;
    }
}


void gr_reset( gr_t gr )
{
    gm_used( gr ) = 0;
}


gr_t gr_duplicate( gr_t gr )
{
    gr_t dup;

    dup = gr_new_sized( gm_size( gr ) );
    *dup = *gr;
    memcpy( gm_data( dup ), gm_data( gr ), gr_used_size( gr ) );

    return dup;
}


void* gr_swap( gr_t gr, gr_size_t pos, void* item )
{
    if ( gr == NULL )
        return NULL;

    if ( gm_empty( gr ) )
        return NULL;

    gr_size_t norm;
    void*     ret;

    norm = gr_norm_idx( gr, pos );
    ret = gm_nth( gr, norm );
    gm_nth( gr, norm ) = item;

    return ret;
}


void gr_insert_at( gr_p gp, gr_size_t pos, void* item )
{
    gr_size_t new_used = gm_used( *gp ) + 1;

    if ( new_used > gm_size( *gp ) )
        gr_resize_to( gp, gr_incr_size( *gp ) );

    gr_size_t norm;
    if ( pos == gm_used( *gp ) )
        norm = pos;
    else
        norm = gr_norm_idx( *gp, pos );

    if ( norm < gm_used( *gp ) ) {
        memmove( &( gm_nth( *gp, norm + 1 ) ),
                 &( gm_nth( *gp, norm ) ),
                 ( gm_used( *gp ) - norm ) * gr_unit_size );
    } else if ( norm > gm_used( *gp ) ) {
        gr_assert( 0 ); // GCOV_EXCL_LINE
    }

    gm_nth( *gp, norm ) = item;
    gm_used( *gp ) = new_used;
}


int gr_insert_if( gr_t gr, gr_size_t pos, void* item )
{
    gr_size_t new_used = gm_used( gr ) + 1;

    if ( new_used > gm_size( gr ) )
        return gr_false;

    gr_size_t norm;
    if ( pos == gm_used( gr ) )
        norm = pos;
    else
        norm = gr_norm_idx( gr, pos );

    if ( norm < gm_used( gr ) ) {
        memmove( &( gm_nth( gr, norm + 1 ) ),
                 &( gm_nth( gr, norm ) ),
                 ( gm_used( gr ) - norm ) * gr_unit_size );
    } else if ( norm > gm_used( gr ) ) {
        gr_assert( 0 ); // GCOV_EXCL_LINE
    }

    gm_nth( gr, norm ) = item;
    gm_used( gr ) = new_used;

    return gr_true;
}


void* gr_delete_at( gr_t gr, gr_size_t pos )
{
    if ( gm_empty( gr ) )
        return NULL;

    void*     ret;
    gr_size_t new_used = gm_used( gr ) - 1;

    if ( gm_used( gr ) == 1 ) {
        ret = gr_first( gr );
        gm_used( gr ) = 0;
        gm_first( gr ) = NULL;
        return NULL;
    }

    gr_size_t norm = gr_norm_idx( gr, pos );

    ret = gm_nth( gr, norm );
    memmove( &( gm_nth( gr, norm ) ),
             &( gm_nth( gr, norm + 1 ) ),
             ( gm_used( gr ) - ( norm + 1 ) ) * gr_unit_size );

    gm_used( gr ) = new_used;

    return ret;
}


void gr_sort( gr_t gr, gr_compare_fn_p compare )
{
    qsort( gr->data, gr->used, gr_unit_size, compare );
}



/* ------------------------------------------------------------
 * Queries:
 */

gr_size_t gr_used( gr_t gr )
{
    return gm_used( gr );
}


gr_size_t gr_size( gr_t gr )
{
    return gm_size( gr );
}

void** gr_data( gr_t gr )
{
    return gr->data;
}

void* gr_first( gr_t gr )
{
    return gm_first( gr );
}

void* gr_last( gr_t gr )
{
    if ( gm_any( gr ) )
        return gm_last( gr );
    else
        return NULL;
}

void* gr_nth( gr_t gr, gr_size_t pos )
{
    if ( gm_any( gr ) ) {
        gr_size_t idx;
        idx = gr_norm_idx( gr, pos );
        return gr->data[ idx ];
    } else {
        return NULL;
    }
}


int gr_is_empty( gr_t gr )
{
    if ( gr == NULL )
        return gr_false;

    if ( gm_used( gr ) <= 0 )
        return gr_true;
    else
        return gr_false;
}


int gr_is_full( gr_t gr )
{
    if ( gr == NULL )
        return gr_false;

    if ( gm_used( gr ) >= gm_size( gr ) )
        return gr_true;
    else
        return gr_false;
}


gr_pos_t gr_find( gr_t gr, void* item )
{
    for ( gr_size_t i = 0; i < gm_used( gr ); i++ ) {
        if ( gm_nth( gr, i ) == item )
            return i;
    }

    return GR_NOT_INDEX;
}


gr_pos_t gr_find_with( gr_t gr, gr_compare_fn_p compare, void* ref )
{
    for ( gr_size_t i = 0; i < gm_used( gr ); i++ ) {
        if ( compare( gm_nth( gr, i ), ref ) )
            return i;
    }

    return GR_NOT_INDEX;
}


void gr_set_local( gr_t gr, int val )
{
    if ( val != 0 )
        gr->size = gr->size | 0x1L;
    else
        gr->size = gr->size & gr_smsk;
}


int gr_get_local( gr_t gr )
{
    return gr_local( gr );
}



/* ------------------------------------------------------------
 * Internal support:
 */


/**
 * Convert size to a legal Gromer size.
 *
 * @param size Requested size.
 *
 * @return Legal size.
 */
static gr_size_t gr_legal_size( gr_size_t size )
{
    size = gr_snor( size );

    if ( size < GR_MIN_SIZE )
        size = GR_MIN_SIZE;

    return size;
}


/**
 * Normalize (possibly negative) Gromer index. Positive index is
 * saturated to Gromer length, and negative index is normalized.
 *
 * -1 means last char in Gromer, -2 second to last, etc. Index after
 * last char can only be expressed by positive indeces. E.g. for
 * Gromer with length of 4 the indeces are:
 *
 * Chars:     a  b  c  d  \0
 * Positive:  0  1  2  3  4
 * Negative: -4 -3 -2 -1
 *
 * @param gr  Gromer.
 * @param idx Index to Gromer.
 *
 * @return Unsigned (positive) index to Gromer.
 */
static gr_size_t gr_norm_idx( gr_t gr, gr_size_t idx )
{
    gr_assert( idx < gm_used( gr ) );

    if ( idx >= gm_used( gr ) )
        return gm_used( gr ) - 1; // GCOV_EXCL_LINE
    else
        return idx;
}


/**
 * Resize Gromer to requested size.
 *
 * @param gp       Gromer reference.
 * @param new_size Requested size.
 */
static void gr_resize_to( gr_p gp, gr_size_t new_size )
{
    if ( gr_get_local( *gp ) )
        *gp = (gr_t)gr_malloc( gr_struct_size( new_size ) );
    else
        *gp = (gr_t)gr_realloc( *gp, gr_struct_size( new_size ) );

    ( *gp )->size = new_size;

    /* NOTE: Setting to non-local is not needed, since size is already
     * an even value. It is here only for clarity. */
    gr_set_local( *gp, 0 );
}


/**
 * Disabled (void) assertion.
 */
void gr_void_assert( void ) // GCOV_EXCL_LINE
{
}
