/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file shore_sort_buf.cpp
 *
 *  @brief Implementation of comparison functions for sort_buffer_t 
 *
 *  @author: Ippokratis Pandis, Sept 2008
 *
 */

#include "sm/shore/shore_sort_buf.h"

using namespace shore;



const int MIN_TUPLES_FOR_SORT = 100;


/**********************************************************************
 * 
 * Implementation of Comparison functions
 *
 * @input:  obj1, obj2 of the same type, casted to (const void*)
 *
 * @return: 1 iff obj1>obj2, 0 iff obj1==obj2, -1 iff obj1<obj2
 * 
 * @note:   Currently only INT and SMALLINT types supported (both fixed 
 *          legnth)
 *
 **********************************************************************/


int compare_smallint(const void* d1, const void* d2)
{
    short data1 = *((short*)d1);
    short data2 = *((short*)d2);
    if (data1 > data2) return (1);
    if (data1 == data2) return (0);
    return (-1);
}


int compare_int(const void* d1, const void* d2)
{
    int data1 = *((int*)d1);
    int data2 = *((int*)d2);
    if (data1 > data2) return (1);
    if (data1 == data2) return (0);
    return (-1);
}


template <typename T>
int compare(const void* d1, const void* d2)
{
    T data1 = *((T*)d1);
    T data2 = *((T*)d2);
    if (data1 > data2) return (1);
    if (data1 == data2) return (0);
    return (-1);
}

/**********************************************************************
 *
 * sort_buffer_t methods
 *
 **********************************************************************/


/********************************************************************* 
 *
 *  @fn:    init
 *  
 *  @brief: Calculate the tuple size and allocate the initial memory
 *          buffer for the tuples.
 *
 *********************************************************************/

void sort_man_impl::init()
{
    assert (_ptable);

    /* calculate tuple size */
    _tuple_size = 0;

    //for (int i=0; i<_tuple_count; i++)
    for (int i=0; i<_ptable->field_count(); i++)
        _tuple_size += _ptable->desc(i)->fieldmaxsize();

    /* allocate size for MIN_TUPLES_FOR_SORT tuples */
    _sort_buf = new char[MIN_TUPLES_FOR_SORT*_tuple_size]; 
    _buf_size = MIN_TUPLES_FOR_SORT;

    _is_sorted = false;
}



/********************************************************************* 
 *
 *  @fn:    reset
 *  
 *  @brief: Clear the buffer and wait for new tuples
 *
 *********************************************************************/

void sort_man_impl::reset()
{
    assert (_ptable);
    // the soft_buf should be set
    assert (_sort_buf);
    // if buf_size>0 means that the manager has already been set
    assert (_buf_size); 
    // no need to calculate tuple size
    assert (_tuple_size);

    // memset the buffer
    memset(_sort_buf, 0, _buf_size);
    _is_sorted = false;
}


/********************************************************************* 
 *
 *  @fn:    add_tuple
 *  
 *  @brief: Inserts a new tuple in the buffer. If there is not enough
 *          space, it doubles the allocated space.
 *
 *********************************************************************/

void sort_man_impl::add_tuple(sorter_tuple& atuple)
{
    CRITICAL_SECTION(cs, _sorted_lock);

    /* setup the tuple size */
    if (!_tuple_size) init();

    /* check if more space is needed */
    if (_buf_size == _tuple_count) {
        /* double the buffer size if needed */
        char* tmp = new char[(2*_buf_size)*_tuple_size];
        memcpy(tmp, _sort_buf, _buf_size*_tuple_size);
        delete [] _sort_buf;

        _sort_buf = tmp;
        _buf_size *= 2;
    }

    /* add the current tuple to the end of the buffer */
    format(&atuple, *_preprow);
    assert (_preprow->_dest);
    memcpy(_sort_buf+(_tuple_count*_tuple_size), _preprow->_dest, _tuple_size);
    _tuple_count++;
    _is_sorted = false;
}


/********************************************************************* 
 *
 *  @fn:   sort
 *  
 *  @note: Uses the old qsort // TODO: use sort() 
 *
 *********************************************************************/

void sort_man_impl::sort()
{
    CRITICAL_SECTION(cs, _sorted_lock);

    /* compute the number of bytes used in sorting */
#if 0
    // displays buffer before/after sorting
    cout << "Before sorting: " << endl;
    if (_desc[0].type() == SQL_SMALLINT) {
        for (int i=0; i<_tuple_count; i++) {
            cout << ((short*)(_sort_buf+i*_tuple_size))[0] << " ";
        }
        else if (_desc[0].type() == SQL_INT) {
            for (int i=0; i<_tuple_count; i++) {
                cout << ((int*)(_sort_buf+i*_tuple_size))[0] << " ";
            }
        }
        cout << endl;
    }
#endif

    // does the sorting
    switch (_ptable->desc(0)->type()) {
    case SQL_SMALLINT:
        qsort(_sort_buf, _tuple_count, _tuple_size, compare_smallint); break;
    case SQL_INT:
        qsort(_sort_buf, _tuple_count, _tuple_size, compare_int); break;
    default: 
        /* not implemented yet */
        assert(false);
    }
    _is_sorted = true;

#if 0
    cout << "After sorting: " << endl;
    if (_desc[0].type() == SQL_SMALLINT) {
        for (int i=0; i<_tuple_count; i++) {
            cout << ((short*)(_sort_buf+i*_tuple_size))[0] << " ";
        }
        else if (_desc[0].type() == SQL_INT) {
            for (int i=0; i<_tuple_count; i++) {
                cout << ((int*)(_sort_buf+i*_tuple_size))[0] << " ";
            }
        }
        cout << endl;
    }
#endif
}

    
/********************************************************************* 
 *
 *  @fn:    get_sort_iter
 *  
 *  @brief: Initializes a sort_scan_impl for the particular sorter buffer
 *
 *  @note:  It is responsibility of the caller to de-allocate
 *
 *********************************************************************/

w_rc_t sort_man_impl::get_sort_iter(ss_m* db,
                                    sort_iter_impl* &sort_iter)
{
    sort_iter = new sort_iter_impl(db, _ptable, this);
    return (RCOK);
}
     


/********************************************************************* 
 *
 *  @fn:    get_sorted
 *  
 *  @brief: Returns the tuple in the buffer pointed by the index
 *
 *  @note:  It asserts if the buffer is not sorted
 *
 *********************************************************************/

bool sort_man_impl::get_sorted(const int index, sorter_tuple* ptuple)
{
    CRITICAL_SECTION(cs, _sorted_lock);

    if (_is_sorted) {
        if (index >=0 && index < _tuple_count) {
            return (load(ptuple, _sort_buf + (index*_tuple_size)));
        }
        TRACE( TRACE_DEBUG, "out of bounds index...\n");
        return (false);
    }
    TRACE( TRACE_DEBUG, "buffer not sorted yet...\n");
    return (false);
}

