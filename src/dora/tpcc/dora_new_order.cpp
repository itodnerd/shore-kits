/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   dora_new_order.cpp
 *
 *  @brief:  DORA TPC-C NEW-ORDER
 *
 *  @note:   Implementation of RVPs and Actions that synthesize 
 *           the TPC-C NewOrder trx according to DORA
 *
 *  @author: Ippokratis Pandis, Nov 2008
 */

#include "dora/tpcc/dora_new_order.h"


using namespace dora;
using namespace shore;
using namespace tpcc;



ENTER_NAMESPACE(dora);


//
// RVPS
//
// (1) final_nord_rvp
// (2) midway_nord_rvp
//


DEFINE_DORA_FINAL_RVP_CLASS(final_nord_rvp,new_order);



/******************************************************************** 
 *
 * NEWORDER MIDWAY RVP - enqueues the I(ORD) - I(NORD) - OL_CNT x I(OL) actions
 *
 ********************************************************************/

w_rc_t mid_nord_rvp::run() 
{
    // 1. Setup the next RVP
#ifndef ONLYDORA
    assert (_xct);
#endif

    // 0. Calculate the intratrx/total number of actions
    register int olcnt    = _in._ol_cnt;
    register int intratrx = _in._ol_cnt + 2;
    register int total    = (2*_in._ol_cnt) + 6;
    register int whid     = _in._wh_id;

    // 1. Setup the final RVP
    final_nord_rvp* frvp = _penv->new_final_nord_rvp(_tid,_xct,_xct_id,_result,_actions);

    TRACE( TRACE_TRX_FLOW, "Next phase (%d)\n", _tid);    
    typedef range_partition_impl<int>   irpImpl; 

    // 2. Generate and enqueue the (Midway->Final) actions
    //
    // 1 - INS_ORD
    // 1 - INS_NORD
    // 1 - INS_OL

    {
        // 2a. Generate the no-item-input
        no_item_nord_input_t anoitin;
        _in.get_no_item_input(anoitin);

        // 2b. Insert (ORD)
        ins_ord_nord_action* ins_ord_nord = _penv->new_ins_ord_nord_action(_tid,_xct,frvp,anoitin);
        irpImpl* my_ord_part = _penv->ord()->myPart(whid-1);

        // 2c. Insert (NORD)
        ins_nord_nord_action* ins_nord_nord = _penv->new_ins_nord_nord_action(_tid,_xct,frvp,anoitin);
        irpImpl* my_nord_part = _penv->nor()->myPart(whid-1);

        // 2d. Insert (OL) - used to be OL_CNT actions
        ins_ol_nord_action* ins_ol_nord = _penv->new_ins_ol_nord_action(_tid,_xct,frvp,_in);
        irpImpl* my_ol_part = _penv->oli()->myPart(whid-1);


        // ORD_PART_CS
        CRITICAL_SECTION(ord_part_cs, my_ord_part->_enqueue_lock);
        if (my_ord_part->enqueue(ins_ord_nord,_bWake)) {
            TRACE( TRACE_DEBUG, "Problem in enqueueing INS_ORD_NORD\n");
            assert (0); 
            return (RC(de_PROBLEM_ENQUEUE));
        }

        // NORD_PART_CS
        CRITICAL_SECTION(nord_part_cs, my_nord_part->_enqueue_lock);
        ord_part_cs.exit();
        if (my_nord_part->enqueue(ins_nord_nord,_bWake)) {
            TRACE( TRACE_DEBUG, "Problem in enqueueing INS_NORD_NORD\n");
            assert (0); 
            return (RC(de_PROBLEM_ENQUEUE));
        }

        // OLI_PART_CS
        CRITICAL_SECTION(oli_part_cs, my_ol_part->_enqueue_lock);
        nord_part_cs.exit();
        if (my_ol_part->enqueue(ins_ol_nord,_bWake)) {
            TRACE( TRACE_DEBUG, "Problem in enqueueing INS_OL_NORD\n");
            assert (0); 
            return (RC(de_PROBLEM_ENQUEUE));
        }    
    }
    
    return (RCOK);
}




/******************************************************************** 
 *
 * NEWORDER TPC-C DORA ACTIONS
 *
 ********************************************************************/


/******************************************************************** 
 *
 * - Start -> Midway
 *
 *
 * (1) R_WH_NORD_ACTION
 * (2) R_CUST_NORD_ACTION
 * (3) UPD_DIST_NORD_ACTION
 * (4) R_ITEM_NORD_ACTION
 * (5) UPD_ITEM_NORD_ACTION
 *
 * @note: Those actions may need to report something to the next (midway) RVP.
 *        Therefore, at the end of each action there may be an update of data 
 *        on the RVP.  
 *
 ********************************************************************/


// R_WH_NORD_ACTION

void r_wh_nord_action::calc_keys() 
{
    set_read_only();
    _down.push_back(_in._wh_id);
    _up.push_back(_in._wh_id);
}


w_rc_t r_wh_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<warehouse_t>* prwh = _penv->warehouse_man()->get_tuple();
    assert (prwh);

    rep_row_t areprow(_penv->warehouse_man()->ts());
    areprow.set(_penv->warehouse_desc()->maxsize()); 
    prwh->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        /* SELECT w_tax
         * FROM warehouse
         * WHERE w_id = :w_id
         *
         * plan: index probe on "W_INDEX"
         */

        // 1. retrieve warehouse (read-only)
        TRACE( TRACE_TRX_FLOW, "App: %d NO:wh-idx-nl (%d)\n", _tid, _in._wh_id);

#ifndef ONLYDORA
        e = _penv->warehouse_man()->wh_index_probe_nl(_penv->db(), 
                                                      prwh, _in._wh_id);
#endif

        if (e.is_error()) { goto done; }
        prwh->get_value(7, _prvp->_in._awh.W_TAX);

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prwh->print_tuple();
#endif

done:
    // give back the tuple
    _penv->warehouse_man()->give_tuple(prwh);
    return (e);
}



// R_CUST_NORD_ACTION

void r_cust_nord_action::calc_keys() 
{
    set_read_only();
    _down.push_back(_in._wh_id);
    _down.push_back(_in._d_id);
    _up.push_back(_in._wh_id);
    _up.push_back(_in._d_id);
}


w_rc_t r_cust_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<customer_t>* prcust = _penv->customer_man()->get_tuple();
    assert (prcust);

    rep_row_t areprow(_penv->customer_man()->ts());
    areprow.set(_penv->customer_desc()->maxsize()); 
    prcust->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        /* SELECT c_discount, c_last, c_credit
         * FROM customer
         * WHERE w_id = :w_id AND c_d_id = :d_id AND c_id = :c_id
         *
         * plan: index probe on "C_INDEX"
         */

        // 1. retrieve customer (read-only)
        TRACE( TRACE_TRX_FLOW, 
               "App: %d NO:cust-idx-nl (%d) (%d) (%d)\n", 
               _tid, _in._wh_id, _in._d_id, _in._c_id);

#ifndef ONLYDORA
        e = _penv->customer_man()->cust_index_probe_nl(_penv->db(), prcust,
                                                       _in._wh_id, _in._d_id, 
                                                       _in._c_id);
#endif

        if (e.is_error()) { goto done; }

        prcust->get_value(15, _prvp->_in._acust.C_DISCOUNT);
        prcust->get_value(13, _prvp->_in._acust.C_CREDIT, 3);
        prcust->get_value(5, _prvp->_in._acust.C_LAST, 17);

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prcust->print_tuple();
#endif

done:
    // give back the tuple
    _penv->customer_man()->give_tuple(prcust);
    return (e);
}




// UPD_DIST_NORD_ACTION

void upd_dist_nord_action::calc_keys() 
{
    _down.push_back(_in._wh_id);
    _down.push_back(_in._d_id);
    _up.push_back(_in._wh_id);
    _up.push_back(_in._d_id);
}

w_rc_t upd_dist_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<district_t>* prdist = _penv->district_man()->get_tuple();
    assert (prdist);

    rep_row_t areprow(_penv->district_man()->ts());
    areprow.set(_penv->district_desc()->maxsize()); 
    prdist->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        /* SELECT d_tax, d_next_o_id
         * FROM district
         * WHERE d_id = :d_id AND d_w_id = :w_id
         *
         * plan: index probe on "D_INDEX"
         */

        // 1. retrieve district for update
        TRACE( TRACE_TRX_FLOW, 
               "App: %d NO:dist-idx-nl (%d) (%d)\n", 
               _tid, _in._wh_id, _in._d_id);

#ifndef ONLYDORA
        e = _penv->district_man()->dist_index_probe_nl(_penv->db(), 
                                                       prdist, 
                                                       _in._wh_id, _in._d_id);
#endif

        if (e.is_error()) { goto done; }

        prdist->get_value(8, _prvp->_in._adist.D_TAX);
        prdist->get_value(10, _prvp->_in._adist.D_NEXT_O_ID);
        _prvp->_in._adist.D_NEXT_O_ID++;

        /* UPDATE district
         * SET d_next_o_id = :d_next_o_id+1
         * WHERE CURRENT OF dist_cur
         */

        // 2. Update next_o_id
        register const int next_o_id = _prvp->_in._adist.D_NEXT_O_ID;
        TRACE( TRACE_TRX_FLOW, 
               "App: %d NO:dist-upd-next-o-id-nl (%d)\n", _tid, next_o_id);

#ifndef ONLYDORA
        e = _penv->district_man()->dist_update_next_o_id_nl(_penv->db(), 
                                                            prdist, 
                                                            next_o_id);
#endif

        if (e.is_error()) { goto done; }

        // 3. Update midway RVP 
        _prvp->_in._d_next_o_id = next_o_id;            

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prdist->print_tuple();
#endif

done:
    // give back the tuple
    _penv->district_man()->give_tuple(prdist);
    return (e);
}



// R_ITEM_NORD_ACTION

#warning Only 1 field (WH) determines the ITEM table accesses! Not 2 nor item-id!

void r_item_nord_action::calc_keys() 
{
    // !!! IP: Correct is to use the _ol_supply_wh_id !!!
    set_read_only();
    _down.push_back(_in._wh_id);
    _up.push_back(_in._wh_id);
}

w_rc_t r_item_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<item_t>* pritem = _penv->item_man()->get_tuple();
    assert (pritem);

    rep_row_t areprow(_penv->item_man()->ts());
    areprow.set(_penv->item_desc()->maxsize()); 
    pritem->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        // 1. Probe item (read-only)
        register int idx=0;

        TRACE( TRACE_TRX_FLOW, "App: %d NO:r-item (%d)\n", 
               _tid, _in._ol_cnt);

        // IP: The new version of the r-tem does all the work in a single action
        for (idx=0; idx<_in._ol_cnt; idx++) {

            register int ol_i_id = _in.items[idx]._ol_i_id;
            register int ol_supply_w_id = _in.items[idx]._ol_supply_wh_id;
            assert (_in._wh_id==ol_supply_w_id); // IP: only local NewOrders

            /* SELECT i_price, i_name, i_data
             * FROM item
             * WHERE i_id = :ol_i_id
             *
             * plan: index probe on "I_INDEX"
             */
        
            TRACE( TRACE_TRX_FLOW, "App: %d NO:item-idx-nl-%d (%d)\n", 
                   _tid, idx, ol_i_id);

#ifndef ONLYDORA
            e = _penv->item_man()->it_index_probe_nl(_penv->db(), 
                                                     pritem, 
                                                     ol_i_id);
#endif

            if (e.is_error()) { goto done; }

            // 2a. Calculate the item amount
            pritem->get_value(4, _in.items[idx]._aitem.I_DATA, 51);
            pritem->get_value(3, _in.items[idx]._aitem.I_PRICE);
            pritem->get_value(2, _in.items[idx]._aitem.I_NAME, 25);        
            _in.items[idx]._item_amount = _in.items[idx]._aitem.I_PRICE * _in.items[idx]._ol_quantity; 

            // 2b. Update RVP data
            _prvp->_in.items[idx]._item_amount = _in.items[idx]._item_amount;
        }
    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    pritem->print_tuple();
#endif

done:
    // give back the tuple
    _penv->item_man()->give_tuple(pritem);
    return (e);
}



// UPD_STO_NORD_ACTION

#warning Only 1 field (WH) determines the STOCK table accesses! Not 2.

void upd_sto_nord_action::calc_keys() 
{
    // !!! IP: Correct is to use the _ol_supply_wh_id !!!
    _down.push_back(_in._wh_id);
    _up.push_back(_in._wh_id);
}


w_rc_t upd_sto_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<stock_t>* prst = _penv->stock_man()->get_tuple();
    assert (prst);

    rep_row_t areprow(_penv->stock_man()->ts());
    areprow.set(_penv->stock_desc()->maxsize()); 
    prst->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        register int idx=0;
        register int ol_i_id=0;
        register int ol_supply_w_id=0;

        TRACE( TRACE_TRX_FLOW, "App: %d NO:upd-stock (%d)\n", 
               _tid, _in._ol_cnt);

        // IP: The new version of the upd-stock does all the work in a single action
        for (idx=0; idx<_in._ol_cnt; idx++) {

            // 4. probe stock (for update)
            ol_i_id = _prvp->_in.items[idx]._ol_i_id;
            ol_supply_w_id = _prvp->_in.items[idx]._ol_supply_wh_id;


            /* SELECT s_quantity, s_remote_cnt, s_data, s_dist0, s_dist1, s_dist2, ...
             * FROM stock
             * WHERE s_i_id = :ol_i_id AND s_w_id = :ol_supply_w_id
             *
             * plan: index probe on "S_INDEX"
             */
            
            tpcc_stock_tuple* pstock = &_prvp->_in.items[idx]._astock;
            tpcc_item_tuple*  pitem  = &_prvp->_in.items[idx]._aitem;
            TRACE( TRACE_TRX_FLOW, "App: %d NO:stock-idx-nl-%d (%d) (%d)\n", 
                   _tid, idx, ol_supply_w_id, ol_i_id);

#ifndef ONLYDORA
            e = _penv->stock_man()->st_index_probe_nl(_penv->db(), prst, 
                                                      ol_supply_w_id, ol_i_id);
#endif

            if (e.is_error()) { goto done; }

            prst->get_value(0, pstock->S_I_ID);
            prst->get_value(1, pstock->S_W_ID);
            prst->get_value(5, pstock->S_YTD);
            pstock->S_YTD += _prvp->_in.items[idx]._ol_quantity;
            prst->get_value(2, pstock->S_REMOTE_CNT);        
            prst->get_value(3, pstock->S_QUANTITY);
            pstock->S_QUANTITY -= _prvp->_in.items[idx]._ol_quantity;
            if (pstock->S_QUANTITY < 10) pstock->S_QUANTITY += 91;
            prst->get_value(6+_in._d_id, pstock->S_DIST[6+_in._d_id], 25);
            prst->get_value(16, pstock->S_DATA, 51);

            char c_s_brand_generic;
            if (strstr(pitem->I_DATA, "ORIGINAL") != NULL && 
                strstr(pstock->S_DATA, "ORIGINAL") != NULL)
                c_s_brand_generic = 'B';
            else c_s_brand_generic = 'G';

            prst->get_value(4, pstock->S_ORDER_CNT);
            pstock->S_ORDER_CNT++;

            if (_in._wh_id != _prvp->_in.items[idx]._ol_supply_wh_id) { 
                pstock->S_REMOTE_CNT++;
                assert (0); // Should not happen
            }

            /* UPDATE stock
             * SET s_quantity = :s_quantity, s_order_cnt = :s_order_cnt
             * WHERE s_w_id = :w_id AND s_i_id = :ol_i_id;
             */

            TRACE( TRACE_TRX_FLOW, "App: %d NO:stock-upd-tuple-nl-%d (%d) (%d)\n", 
                   _tid, idx, pstock->S_W_ID, pstock->S_I_ID);

#ifndef ONLYDORA
            e = _penv->stock_man()->st_update_tuple_nl(_penv->db(), prst, 
                                                       pstock);
#endif

            if (e.is_error()) { goto done; }

            // update RVP
            // The RVP is updated throught the pstock
            
        } // EOF: OLCNT upd-stocks

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prst->print_tuple();
#endif

done:
    // give back the tuple
    _penv->stock_man()->give_tuple(prst);
    return (e);
}





/******************************************************************** 
 *
 * - Midway -> Final
 *
 * (6) INS_ORD_NORD_ACTION
 * (7) INS_NORD_NORD_ACTION
 * (8) INS_OL_NORD_ACTION
 *
 ********************************************************************/


// INS_ORD_NORD_ACTION


#warning Only 2 fields (WH,DI) determine the ORDER table accesses! Not 3.

void ins_ord_nord_action::calc_keys() 
{
    _down.push_back(_in._wh_id);
    _down.push_back(_in._d_id);
    _up.push_back(_in._wh_id);
    _up.push_back(_in._d_id);
}


w_rc_t ins_ord_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<order_t>* prord = _penv->order_man()->get_tuple();
    assert (prord);

    rep_row_t areprow(_penv->order_man()->ts());
    areprow.set(_penv->order_desc()->maxsize()); 
    prord->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        // 1. insert row to ORDERS table

        /* INSERT INTO orders
         * VALUES (o_id, o_d_id, o_w_id, o_c_id, o_entry_d, o_ol_cnt, o_all_local)
         */

        prord->set_value(0, _in._d_next_o_id);
        prord->set_value(1, _in._c_id);
        prord->set_value(2, _in._d_id);
        prord->set_value(3, _in._wh_id);
        prord->set_value(4, _in._tstamp);
        prord->set_value(5, 0);
        prord->set_value(6, _in._ol_cnt);
        prord->set_value(7, _in._all_local);

        TRACE( TRACE_TRX_FLOW, "App: %d NO:ord-add-tuple-nl (%d)\n", 
               _tid, _in._d_next_o_id);

#ifndef ONLYDORA
        e = _penv->order_man()->add_tuple(_penv->db(), prord, NL);
#endif

        if (e.is_error()) { goto done; }

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prord->print_tuple();
#endif

done:
    // give back the tuple
    _penv->order_man()->give_tuple(prord);
    return (e);
}



// INS_NORD_NORD_ACTION


#warning Only 2 fields (WH,DI) determine the NEW-ORDER table accesses! Not 3.

void ins_nord_nord_action::calc_keys() 
{
    _down.push_back(_in._wh_id);
    _down.push_back(_in._d_id);
    _up.push_back(_in._wh_id);
    _up.push_back(_in._d_id);
}


w_rc_t ins_nord_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<new_order_t>* prno = _penv->new_order_man()->get_tuple();
    assert (prno);

    rep_row_t areprow(_penv->new_order_man()->ts());
    areprow.set(_penv->new_order_desc()->maxsize()); 
    prno->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        // 1. insert row to NEW_ORDER table

        /* INSERT INTO new_order VALUES (o_id, d_id, w_id)
         */

        prno->set_value(0, _in._d_next_o_id);
        prno->set_value(1, _in._d_id);
        prno->set_value(2, _in._wh_id);

        TRACE( TRACE_TRX_FLOW, "App: %d NO:nord-add-tuple (%d) (%d) (%d)\n", 
               _tid, _in._wh_id, _in._d_id, _in._d_next_o_id);
    
#ifndef ONLYDORA
        e = _penv->new_order_man()->add_tuple(_penv->db(), prno, NL);
#endif

        if (e.is_error()) { goto done; }

    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prno->print_tuple();
#endif

done:
    // give back the tuple
    _penv->new_order_man()->give_tuple(prno);
    return (e);
}



// INS_OL_NORD_ACTION


#warning Only 2 fields (WH,DI) determine the ORDERLINE table accesses! Not 3.

void ins_ol_nord_action::calc_keys() 
{
    _down.push_back(_in._wh_id);
    _down.push_back(_in._d_id);
    _up.push_back(_in._wh_id);
    _up.push_back(_in._d_id);
}


w_rc_t ins_ol_nord_action::trx_exec() 
{
    assert (_penv);

    // get table tuple from the cache
    row_impl<order_line_t>* prol = _penv->order_line_man()->get_tuple();
    assert (prol);

    rep_row_t areprow(_penv->order_line_man()->ts());
    areprow.set(_penv->order_line_desc()->maxsize()); 
    prol->_rep = &areprow;

    w_rc_t e = RCOK;

    { // make gotos safe

        // 1. insert row to ORDER_LINE
        register int idx = 0;

        TRACE( TRACE_TRX_FLOW, "App: %d NO:ins-ol (%d)\n", 
               _tid, _in._ol_cnt);

        for (idx=0; idx<_in._ol_cnt; idx++) {

            /* INSERT INTO order_line
             * VALUES (o_id, d_id, w_id, ol_ln, ol_i_id, supply_w_id,
             *        '0001-01-01-00.00.01.000000', ol_quantity, iol_amount, dist)
             */

            prol->set_value(0, _in._d_next_o_id);
            prol->set_value(1, _in._d_id);
            prol->set_value(2, _in._wh_id);
            prol->set_value(3, idx+1);
            prol->set_value(4, _in.items[idx]._ol_i_id);
            prol->set_value(5, _in.items[idx]._ol_supply_wh_id);
            prol->set_value(6, _in._tstamp);
            prol->set_value(7, _in.items[idx]._ol_quantity);
            prol->set_value(8, _in.items[idx]._item_amount);
            prol->set_value(9, _in.items[idx]._astock.S_DIST[6+_in._d_id]);

            TRACE( TRACE_TRX_FLOW, 
                   "App: %d NO:ol-add-tuple-%d (%d) (%d) (%d) (%d)\n", 
                   _tid, idx, _in._wh_id, _in._d_id, _in._d_next_o_id, 
                   _in.items[idx]._ol_i_id);

#ifndef ONLYDORA
            e = _penv->order_line_man()->add_tuple(_penv->db(), prol, NL);
#endif

            if (e.is_error()) { goto done; }
        }
        
    } // goto

#ifdef PRINT_TRX_RESULTS
    // at the end of the transaction 
    // dumps the status of all the table rows used
    prol->print_tuple();
#endif

done:
    // give back the tuple
    _penv->order_line_man()->give_tuple(prol);
    return (e);
}





EXIT_NAMESPACE(dora);
