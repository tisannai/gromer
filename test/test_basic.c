#include "unity.h"
#include "gromer.h"
#include <string.h>
#include <unistd.h>

void test_basics( void )
{
    gr_t  gr;
    gr_t  dup;
    char* text = "text";
    gr_d  ret;

    gr = gr_new();
    TEST_ASSERT_EQUAL( GR_DEFAULT_SIZE, gr_size( gr ) );
    TEST_ASSERT_EQUAL( 1, gr_is_empty( gr ) );
    TEST_ASSERT_EQUAL( 0, gr_is_full( gr ) );

    gr_destroy( &gr );
    TEST_ASSERT_EQUAL( NULL, gr );

    gr = gr_new_sized( 12 );
    TEST_ASSERT_EQUAL( 12, gr_size( gr ) );
    TEST_ASSERT_EQUAL( 0, gr_used( gr ) );

    gr_push( &gr, text );
    TEST_ASSERT_EQUAL( 12, gr_size( gr ) );
    TEST_ASSERT_EQUAL( 1, gr_used( gr ) );

    ret = gr_pop( gr );
    TEST_ASSERT_EQUAL( text, ret );
    TEST_ASSERT_EQUAL( 0, gr_used( gr ) );

    /* Push 13 items, so that size is doubled, and check that. */
    for ( int i = 0; i < 13; i++ ) {
        gr_push( &gr, text );
    }

    TEST_ASSERT_EQUAL( 24, gr_size( gr ) );
    TEST_ASSERT_EQUAL( 13, gr_used( gr ) );
    /* Push 11 more, and check that size is same as used. */
    for ( int i = 0; i < 11; i++ ) {
        gr_push( &gr, text );
    }
    TEST_ASSERT( gr_size( gr ) == gr_used( gr ) );
    TEST_ASSERT_EQUAL( 1, gr_is_full( gr ) );
    TEST_ASSERT_EQUAL( 0, gr_is_empty( gr ) );

    gr_reset( gr );
    TEST_ASSERT_EQUAL( 0, gr_used( gr ) );
    gr_resize( &gr, GR_DEFAULT_SIZE );
    TEST_ASSERT_EQUAL( GR_DEFAULT_SIZE, gr_size( gr ) );

    gr_add( &gr, text );
    TEST_ASSERT_EQUAL( GR_DEFAULT_SIZE, gr_size( gr ) );
    TEST_ASSERT_EQUAL( 1, gr_used( gr ) );
    TEST_ASSERT_EQUAL( text, gr_last( gr ) );
    TEST_ASSERT_EQUAL( text, gr_nth( gr, 0 ) );

    ret = gr_remove( &gr );
    TEST_ASSERT_EQUAL( text, ret );
    TEST_ASSERT_EQUAL( NULL, gr );


    gr_add( &gr, text );
    gr_add( &gr, NULL );
    gr_add( &gr, text );
    dup = gr_duplicate( gr );
    for ( gr_size_t i = 0; i < gr_used( gr ); i++ ) {
        TEST_ASSERT( gr_nth( dup, i ) == gr_nth( gr, i ) );
    }

    gr_destroy( &gr );
    gr_destroy( &dup );
    gr_destroy( &gr );
}


int gr_compare_fn( const gr_d a, const gr_d b )
{
    if ( a == b )
        return 1;
    else
        return 0;
}


void test_random_access( void )
{
    gr_t     gr;
    char*    text = "text";
    gr_d     tmp;
    gr_pos_t pos;

    gr = gr_new();
    gr_insert_at( &gr, 0, text );
    gr_insert_at( &gr, 0, NULL );
    TEST_ASSERT_EQUAL( NULL, gr_data( gr )[ 0 ] );
    TEST_ASSERT_EQUAL( text, gr_last( gr ) );

    tmp = gr_swap( gr, 0, text );
    gr_swap( gr, 1, tmp );
    TEST_ASSERT_EQUAL( text, gr_first( gr ) );
    TEST_ASSERT_EQUAL( NULL, gr_last( gr ) );

    pos = gr_find( gr, NULL );
    TEST_ASSERT_EQUAL( 1, pos );

    pos = gr_find( gr, text );
    TEST_ASSERT_EQUAL( 0, pos );

    pos = gr_find_with( gr, gr_compare_fn, NULL );
    TEST_ASSERT_EQUAL( 1, pos );

    pos = gr_find_with( gr, gr_compare_fn, text );
    TEST_ASSERT_EQUAL( 0, pos );


    for ( int i = 0; i < GR_DEFAULT_SIZE; i++ ) {
        gr_insert_at( &gr, -1, text );
    }

    TEST_ASSERT_EQUAL( 2 * GR_DEFAULT_SIZE, gr_size( gr ) );

    for ( int i = 0; i < GR_DEFAULT_SIZE; i++ ) {
        gr_insert_if( gr, gr_used( gr ) - 1, text );
    }

    TEST_ASSERT_EQUAL( 2 * GR_DEFAULT_SIZE, gr_size( gr ) );

    for ( int i = 0; i < GR_DEFAULT_SIZE / 2; i++ ) {
        tmp = gr_delete_at( gr, 0 );
        TEST_ASSERT_EQUAL( text, tmp );
    }

    for ( int i = 0; i < GR_DEFAULT_SIZE / 2; i++ ) {
        gr_delete_at( gr, gr_used( gr ) - 1 );
    }

    TEST_ASSERT_EQUAL( text, gr_first( gr ) );
    TEST_ASSERT_EQUAL( text, gr_last( gr ) );

    TEST_ASSERT_EQUAL_STRING( text, gr_item( gr, 1, char* ) );

    char*     item;
    gr_size_t idx = 0;
    gr_each( gr, item, char* )
    {
        TEST_ASSERT_EQUAL( idx, gr_idx );
        TEST_ASSERT_EQUAL_STRING( text, item );
        idx++;
    }

    for ( int i = 0; i < 2 * GR_DEFAULT_SIZE; i++ ) {
        gr_pop( gr );
    }

    TEST_ASSERT_EQUAL( NULL, gr_first( gr ) );
    TEST_ASSERT_EQUAL( NULL, gr_last( gr ) );
    TEST_ASSERT_EQUAL( NULL, gr_nth( gr, 0 ) );

    gr_swap( gr, 0, NULL );
    gr_insert_if( gr, 0, text );
    gr_delete_at( gr, 0 );
    gr_delete_at( gr, 0 );

    TEST_ASSERT_EQUAL( GR_NOT_INDEX, gr_find( gr, text ) );
    TEST_ASSERT_EQUAL( GR_NOT_INDEX, gr_find_with( gr, gr_compare_fn, text ) );

    gr_destroy( &gr );
    gr_remove( &gr );
    gr_swap( gr, 0, NULL );

    TEST_ASSERT_EQUAL( 0, gr_is_empty( gr ) );
    TEST_ASSERT_EQUAL( 0, gr_is_full( gr ) );

    gr = gr_new_sized( 0 );
    TEST_ASSERT_EQUAL( GR_MIN_SIZE, gr_size( gr ) );
    gr_destroy( &gr );
}


int gr_sort_compare( const gr_d a, const gr_d b )
{
    char* sa = *((char**)a);
    char* sb = *((char**)b);

    return strcmp( sa, sb );
}


void test_sorting( void )
{
    gr_t  gr;
    char* str1 = "aaa";
    char* str2 = "bbb";
    char* str3 = "ccc";

    gr = gr_new();
    gr_push( &gr, str3 );
    gr_push( &gr, str1 );
    gr_push( &gr, str2 );

    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 0, char* ), str3 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 1, char* ), str1 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 2, char* ), str2 );

    gr_sort( gr, gr_sort_compare );

    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 0, char* ), str1 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 1, char* ), str2 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 2, char* ), str3 );

    gr_destroy( &gr );
}


void test_static( void )
{
    gr_t  gr;
    char* str1 = "aaa";
    char* str2 = "bbb";

    gr_local_use( gr, buf, 8 );

    TEST_ASSERT_TRUE( gr_get_local( gr ) );

    gr_push( &gr, str1 );
    gr_push( &gr, str2 );
    gr_push( &gr, str1 );
    gr_push( &gr, str2 );
    gr_push( &gr, str1 );
    gr_push( &gr, str2 );
    gr_push( &gr, str1 );
    gr_push( &gr, str2 );

    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 0, char* ), str1 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 1, char* ), str2 );
    TEST_ASSERT_EQUAL_STRING( gr_item( gr, 2, char* ), str1 );

    TEST_ASSERT_TRUE( gr_get_local( gr ) );

    gr_push( &gr, str1 );
    gr_push( &gr, str2 );

    TEST_ASSERT_FALSE( gr_get_local( gr ) );

    gr_destroy( &gr );

    gr = gr_use( buf, gr_struct_size( 8 ) );
    gr_destroy( &gr );
}



void test_alloc( void )
{
    gr_t      gr;
    gr_d      gd;
    gr_size_t bytes;

    gr = gr_new_page( 2 );

    bytes = gr_alloc_pages( 0, NULL );
    TEST_ASSERT_TRUE( gr_total_size( gr ) == ( 2 * bytes ) );

    gd = gr_alloc( gr, 1 );
    TEST_ASSERT_TRUE( gd != NULL );
    TEST_ASSERT_TRUE( gr->used == 1 );

    gd = gr_alloc( gr, 2 );
    TEST_ASSERT_TRUE( gd != NULL );
    TEST_ASSERT_TRUE( gr->used == 2 );

    gd = gr_alloc( gr, 8 );
    TEST_ASSERT_TRUE( gd != NULL );
    TEST_ASSERT_TRUE( gr->used == 3 );

    gd = gr_alloc( gr, 9 );
    TEST_ASSERT_TRUE( gd != NULL );
    TEST_ASSERT_TRUE( gr->used == 5 );

    gd = gr_alloc( gr, ( gr_size( gr ) - gr->used ) * sizeof( gr_d ) );
    TEST_ASSERT_TRUE( gd != NULL );

    gd = gr_alloc( gr, 1 );
    TEST_ASSERT_TRUE( gd == NULL );

    gr_free( gr );

    gr = gr_new_page( 0 );
    TEST_ASSERT_TRUE( gr_total_size( gr ) == bytes );
    gr_free( gr );
}
