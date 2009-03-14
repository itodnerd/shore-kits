/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file tpcc_input.h
 *
 *  @brief Declaration of the (common) inputs for the TPC-C trxs
 *
 *  @author Ippokratis Pandis (ipandis)
 */

#ifndef __TPCC_INPUT_H
#define __TPCC_INPUT_H

#include "util.h"
#include "stages/tpcc/common/tpcc_const.h"
#include "stages/tpcc/common/tpcc_struct.h"


ENTER_NAMESPACE(tpcc);


/** Exported data structures */


/*********************************************************************
 * 
 * new_order_input_t
 *
 * Input for any NEW_ORDER transaction
 *
 *********************************************************************/

struct ol_item_info 
{
    int   _ol_i_id;             /* input: NURand(8191,1,100000) */
    short _ol_supply_wh_select; /* input: URand(1,100) */
    int   _ol_supply_wh_id;     /* input: x==1 -> URand(1, SF) */
    int   _ol_quantity;         /* input: URand(1,10) */        

    int   _item_amount;         /* placeholder for the item amount */
    tpcc_stock_tuple _astock;   /* placeholder for the stock tuple */
    tpcc_item_tuple _aitem;     /* placeholder for the item tuple */

    // Assignmet operator
    ol_item_info& operator= (const ol_item_info& rhs);

}; // EOF: ol_item_info


struct new_order_input_t
{
    int    _wh_id;        /* input: URand(1,SF) */
    int    _d_id;         /* input: URand(1,10) */
    int    _c_id;         /* input: NURand(1023,1,3000) */
    int    _ol_cnt;       /* input: number of items URand(5,15) */
    int    _rbk;          /* input: rollback URand(1,100) */

    time_t _tstamp;       /* placeholder for the trx start time */
    int    _all_local;    /* placeholder if all orders are on local WHs */
    int    _d_next_o_id;  /* placeholder for the next O_ID of the selected district */

    ol_item_info items[MAX_OL_PER_ORDER]; /* input: for each ol item */

    /** If _supply_wh_id = _wh_id for each item 
     *  then trx called home, else remote 
     */    

    // Construction/Destructions
    new_order_input_t() 
        : _wh_id(0), _d_id(0), _c_id(0), _ol_cnt(0), _rbk(0), _all_local(1), _d_next_o_id(-1)
    { };    

    ~new_order_input_t() { };

    // Assignment operator
    new_order_input_t& operator= (const new_order_input_t& rhs);

}; // EOF new_order_input_t




/*********************************************************************
 * 
 * payment_input_t
 *
 * Input for any PAYMENT transaction
 *
 *********************************************************************/

struct payment_input_t
{
    /**
     *  @brief PAYMENT transaction inputs:
     *  
     *  1) HOME_WH_ID int [1 .. SF] : home warehouse id
     *  2) HOME_D_ID int [1 .. 10]  : home district id
     *  3) V_CUST_WH_SELECTION int [1 .. 100] : customer warehouse selection ( 85% - 15%)
     *  4) REMOTE_WH_ID int [1 .. SF] : remote warehouse id (optional)
     *  5) REMOTE_D_ID int [1 .. 10] : remote district id (optional)
     *  6) V_CUST_IDENT_SELECTION int [1 .. 100] : customer identification selection ( 60% - 40%)
     *  7) C_ID int : customer id (C_ID = NURand(1023, 1, 3000) (optional)
     *  8) C_LAST char* : customer lastname (using NURand(255, 0, 999) (optional)
     *  9) H_AMOUNT long [1.00 .. 5,000.00] : the payment amount
     *  10) H_DATE char* : the payment time
     */

    int    _home_wh_id;             /* input: URand(1,SF) */
    int    _home_d_id;              /* input: URand(1,10) */
    int    _v_cust_wh_selection;    /* input: URand(1,100) - 85%-15% */
    int    _remote_wh_id;           /* input: URand(1,SF) */
    int    _remote_d_id;            /* input: URand(1,10) */
    int    _v_cust_ident_selection; /* input: URand(1,100) - 60%-40% */
    int    _c_id;                   /* input: NURand(1023,1,3000) */
    char   _c_last[16];             /* input: NURand(255,0,999) */
    double _h_amount;               /* input: URand(1.00,5.000) */
    int    _h_date; 

    // Construction/Destructions
    payment_input_t() 
        : _home_wh_id(0), _home_d_id(0), _v_cust_wh_selection(0),
          _remote_wh_id(0), _remote_d_id(0), _v_cust_ident_selection(0),
          _c_id(0), _h_amount(0), _h_date(0)
    { 
        memset(_c_last, '\0', 16);
    };

    ~payment_input_t() { };

    payment_input_t& operator= (const payment_input_t& rhs);

}; // EOF payment_input_t



/*********************************************************************
 * 
 * order_status_input_t
 *
 * Input for any ORDER_STATUS transaction
 *
 *********************************************************************/

struct order_status_input_t
{
    int   _wh_id;       /* input: URand(1,SF) */
    int   _d_id;        /* input: URand(1,10) */
    short _c_select;    /* input: URand(1,100) - 60%-40% */
    int   _c_id;        /* input: NURand(1023,1,3000) */
    char  _c_last[16];  /* input: NURand(255,0,999) */


    // Construction/Destructions
    order_status_input_t() 
        : _wh_id(0), _d_id(0), _c_select(0), _c_id(0)
    { 
        memset(_c_last, '\0', 16);
    };    

    ~order_status_input_t() { };

    // Assignment operator
    order_status_input_t& operator= (const order_status_input_t& rhs);

}; // EOF order_status_input_t



/*********************************************************************
 * 
 * delivery_input_t
 *
 * Input for any DELIVERY transaction
 *
 *********************************************************************/

struct delivery_input_t
{
    int    _wh_id;         /* input: URand(1,SF) */
    short  _carrier_id;    /* input: URand(1,10) */


    // Construction/Destructions
    delivery_input_t() 
        : _wh_id(0),_carrier_id(0)
    { };
    
    ~delivery_input_t() { };


    // Assignment operator
    delivery_input_t& operator= (const delivery_input_t& rhs);

}; // EOF delivery_input_t



/*********************************************************************
 * 
 * stock_level_input_t
 *
 * Input for any STOCK_LEVEL transaction
 *
 *********************************************************************/

struct stock_level_input_t
{
    int   _wh_id;         /* input */
    int   _d_id;          /* input */
    short _threshold;     /* input */

    // Construction/Destructions
    stock_level_input_t() 
        : _wh_id(0), _d_id(0), _threshold(0)
    { 
    };
    
    ~stock_level_input_t() { };


    // Assignment operator
    stock_level_input_t& operator= (const stock_level_input_t& rhs);

}; // EOF stock_level_input_t



/*********************************************************************
 * 
 * mbench_wh_input_t
 *
 * Input for any MBENCH_WH transaction
 *
 *********************************************************************/

struct mbench_wh_input_t
{
    int    _wh_id;    /* input */
    double _amount;   /* input */

    // Construction/Destructions
    mbench_wh_input_t() 
        : _wh_id(0), _amount(0)
    { };
    
    ~mbench_wh_input_t() { };

    // Assignment operator
    mbench_wh_input_t& operator= (const mbench_wh_input_t& rhs);

}; // EOF mbench_wh_input_t



/*********************************************************************
 * 
 * mbench_cust_input_t
 *
 * Input for any MBENCH_CUST transaction
 *
 *********************************************************************/

struct mbench_cust_input_t
{
    int    _wh_id;    /* input */
    int    _d_id;     /* input */
    int    _c_id;     /* input */
    double _amount;   /* input */

    // Construction/Destructions
    mbench_cust_input_t() 
        : _wh_id(0), _d_id(0), _c_id(0), _amount(0)
    { };
    
    ~mbench_cust_input_t() { };

    // Assignment operator
    mbench_cust_input_t& operator= (const mbench_cust_input_t& rhs);

}; // EOF mbench_cust_input_t



/*********************************************************************
 * 
 * populate_*_input_t
 *
 * Input for populating TPC-C DB
 *
 *********************************************************************/

struct populate_baseline_input_t 
{
    int _wh;
};

struct populate_one_unit_input_t 
{
    int _unit;
    int* _cids;
    
};




EXIT_NAMESPACE(tpcc);

#endif

