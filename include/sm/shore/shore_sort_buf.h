/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   shore_sort_buf.h
 *
 *  @brief:  In-memory sort buffer structure
 *
 *  @author: Ippokratis Pandis, January 2008
 */

#ifndef   __SHORE_SORT_BUF_H
#define   __SHORE_SORT_BUF_H

#include "sm/shore/shore_table_man.h"


ENTER_NAMESPACE(shore);



/**********************************************************************
 *
 * This file contains the in-memory sort buffer structure definition.
 * The sort buffer is defined as a subclass of shore_table (defined in
 * shore_table.h) to take advantage of the schema and tuple value
 * operations. The data waiting to be sorted is stored in a memory
 * buffer (sort_buffer_t::_sort_buf).
 *
 * @note: To simplify the memory management, the sort buffer only works
 *        on fixed length fields.
 *        Supported sql_types_t: SQL_INT, SQL_SMALLINT.
 *        Under test: SQL_BIT
 *
 **********************************************************************/

class sort_iter_impl;


/**********************************************************************
 *
 * @class:   sort_buffer_t
 *
 * @brief:   Description of a sort buffer
 *
 **********************************************************************/

class sort_buffer_t : public table_desc_t 
{
public:

    sort_buffer_t(int field_count)
        : table_desc_t("SORT_BUF", field_count)
    { 
    }

    ~sort_buffer_t() 
    { 
    }

    /* set the schema - accepts only fixed length */
    void setup(const int index, sqltype_t type, const int len = 0) 
    {
        assert((index>=0) && (index<_field_count));
        _desc[index].setup(type, "", len);
        assert(!_desc[index].is_variable_length());
        assert(!_desc[index].allow_null());
    }

    /* needed by table_desc_t in order not to be abstract */
    bool read_tuple_from_line(table_row_t& , char* ) {
        assert (false); // should not be called'
        return (false);
    }

}; // EOF: sort_buffer_t



/**********************************************************************
 *
 * @class:   sort_man_impl
 *
 * @warning: NO THREAD-SAFE the caller should make sure that only one
 *           thread is accessing objects of this class
 *
 **********************************************************************/

class sort_man_impl : public table_man_impl<sort_buffer_t>
{    
    typedef row_impl<sort_buffer_t> sorter_tuple;    
    friend class sort_iter_impl;

protected:

    char*       _sort_buf;     /* memory buffer */
    int         _tuple_size;   /* tuple size */
    int         _tuple_count;  /* # of tuples in buffer */
    int         _buf_size;     /* size of the buffer (in # of tuples) */
    bool        _is_sorted;    /* shows if sorted */
    tatas_lock  _sorted_lock; 

    rep_row_t*  _preprow;      /* used for the tuple->format() */

    /* count _tuple_size and allocate buffer */
    void init();

    /* retrieve a tuple */
    bool get_sorted(const int index, sorter_tuple* ptuple); 

public:

    sort_man_impl(sort_buffer_t* aSortBufferDesc, rep_row_t* aprow, int row_count)
        : table_man_impl<sort_buffer_t>(aSortBufferDesc, row_count, false),
          _sort_buf(NULL), _tuple_size(0), _tuple_count(0), _buf_size(0), 
          _preprow(aprow), _is_sorted(false)
    {
    }

    ~sort_man_impl()
    {
        if (_sort_buf)
            delete [] _sort_buf;
    }


    /* add current tuple to the sort buffer */
    void add_tuple(sorter_tuple& atuple);

    /* return a sort iterator */
    w_rc_t get_sort_iter(ss_m* db, sort_iter_impl* &sort_iter);

    /* sort tuples on the first field value */
    void   sort();

    inline int count() { return (_tuple_count); }

    void   reset();

}; // EOF: sort_man_impl


/**********************************************************************
 *
 * @class: sort_iter_impl
 *
 * @brief: Iterator over a sorted buffer
 *
 * @note:  Iterator that does not need a db handle, since the sorting
 *         takes place only in memory
 *
 **********************************************************************/

typedef tuple_iter_t<sort_buffer_t, int, row_impl<sort_buffer_t> > sort_scan_t;

class sort_iter_impl : public sort_scan_t 
{
public:
    typedef row_impl<sort_buffer_t> table_tuple;

private:

    sort_man_impl* _manager;
    int            _index;

public:

    sort_iter_impl(ss_m* db, sort_buffer_t* psortbuf, sort_man_impl* psortman)
        : tuple_iter_t(db, psortbuf, NL, false), _manager(psortman), _index(0)
    { 
        assert (_manager);
        assert (_file);
        W_COERCE(open_scan());
    }


    /* ------------------------------ */
    /* --- sorted iter operations --- */
    /* ------------------------------ */
    
    w_rc_t open_scan();
    w_rc_t close_scan() { return (RCOK); };
    w_rc_t next(ss_m* db, bool& eof, table_tuple& tuple);

    void   reset();

}; // EOF: sort_iter_impl



/**********************************************************************
 *
 * sort_iter_impl methods
 *
 **********************************************************************/


/********************************************************************* 
 *
 *  @fn:    open_scan
 *  
 *  @brief: Opens a scan operator
 *
 *  @note:  If the sorted buffer is not sorted, it sorts it
 *
 *********************************************************************/

inline w_rc_t sort_iter_impl::open_scan()
{ 
    assert (_file);
    assert (_file->field_count()>0);

    _manager->sort();

    _index = 0;
    _opened = true;
    return (RCOK);
}


/********************************************************************* 
 *
 *  @fn:    next
 *  
 *  @brief: Gets the next tuple pointed by the index
 *
 *********************************************************************/

inline w_rc_t sort_iter_impl::next(ss_m* db, bool& eof, table_tuple& tuple) 
{
    assert(_opened);
  
    _manager->get_sorted(_index, &tuple);
    eof = (++_index > _manager->_tuple_count);
    return (RCOK);
}


/********************************************************************* 
 *
 *  @fn:    reset
 *  
 *  @brief: Clear the fields and prepares for re-use
 *
 *********************************************************************/

inline void sort_iter_impl::reset() 
{
    // the sorter_manager should already be set
    assert (_manager);
    _index=0;
}


EXIT_NAMESPACE(shore);


#endif // __SHORE_SORT_BUF
