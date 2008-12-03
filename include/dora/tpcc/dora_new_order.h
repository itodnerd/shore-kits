/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   dora_payment.h
 *
 *  @brief:  DORA TPC-C NEW_ORDER
 *
 *  @note:   Definition of RVPs and Actions that synthesize 
 *           the TPC-C NewOrder trx according to DORA
 *
 *  @author: Ippokratis Pandis, Nov 2008
 */


#ifndef __DORA_TPCC_NEW_ORDER_H
#define __DORA_TPCC_NEW_ORDER_H


#include "dora/tpcc/dora_tpcc.h"
#include "workload/tpcc/shore_tpcc_env.h"


ENTER_NAMESPACE(dora);


using namespace shore;
using namespace tpcc;


//
// RVPS
//
// (1) ol_midway_nord_rvp
// (2) ord_midway_nord_rvp
// (3) final_nord_rvp
//


/******************************************************************** 
 *
 * @class: ol_midway_nord_rvp
 *
 * @brief: Submits the insert orderline action.
 *         Variables it holds:
 *         - d_next_o_id
 *         - it_cnt
 *         - t_stamp
 *         - it_amount
 *         - s_dist
 *         - all_local
 *
 ********************************************************************/

class ol_midway_nord_rvp : public rvp_t
{
    // data needed to be transfered
    int d_next_o_id;
    int it_cnt;
    time_t t_stamp;
    int it_amount;
    char s_dist[25];
    int all_local;
    // pointer to the payment input
    new_order_input_t*   _pnoin;
    // pointer to the shore env
    ShoreTPCCEnv* _ptpccenv;

    ol_midway_nord_rvp(const tid_t& atid, xct_t* axct, const int axctid,
                       trx_result_tuple_t &presult,
                       ShoreTPCCEnv* penv, new_order_input_t* pnoin) 
        : rvp_t(atid, axct, axctid, presult, 2, 2),  // consists of three packets
          _ptpccenv(penv), _pnoin(pnoin)
    { }

    ~ol_midway_nord_rvp() { }

    // the interface
    w_rc_t run();

}; // EOF: ol_midway_pay_rvp



// /******************************************************************** 
//  *
//  * @class: final_pay_rvp
//  *
//  * @brief: Terminal Phase of Payment
//  *
//  ********************************************************************/

// class final_pay_rvp : public terminal_rvp_t
// {
// private:
//     // pointer to the shore env
//     ShoreTPCCEnv* _ptpccenv;
// public:

//     final_pay_rvp(tid_t atid, xct_t* axct, const int axctid,
//                   trx_result_tuple_t &presult,
//                   ShoreTPCCEnv* penv) 
//         : terminal_rvp_t(atid, axct, axctid, presult, 1, 4), 
//           _ptpccenv(penv) 
//     { }
    
//     ~final_pay_rvp() { }

//     // interface
//     w_rc_t run() { assert (_ptpccenv); return (_run(_ptpccenv)); }
//     void upd_committed_stats(); // update the committed trx stats
//     void upd_aborted_stats(); // update the committed trx stats

// }; // EOF: final_pay_rvp


// //
// // ACTIONS
// //
// // (0) pay_action_impl - generic that holds a payment input
// //
// // (1) upd_wh_pay_action_impl
// // (2) upd_dist_pay_action_impl
// // (3) upd_cust_pay_action_impl
// // (4) ins_hist_pay_action_impl
// // 


// /******************************************************************** 
//  *
//  * @abstract class: pay_action_impl
//  *
//  * @brief:          Holds a payment input and a pointer to ShoreTPCCEnv
//  *
//  ********************************************************************/

// class pay_action_impl : public range_action_impl<int>
// {
// protected:

//     ShoreTPCCEnv*   _ptpccenv;
//     payment_input_t _pin;

// public:
    
//     pay_action_impl() : range_action_impl(), _ptpccenv(NULL) { }
//     virtual ~pay_action_impl() { }
//     virtual w_rc_t trx_exec()=0; // pure virtual    
//     virtual void calc_keys()=0; // pure virtual 
//     virtual void set_input(tid_t atid,
//                            xct_t* apxct,
//                            rvp_t* prvp,
//                            ShoreTPCCEnv* penv, 
//                            const payment_input_t& pin) 
//     {
//         assert (apxct);
//         assert (prvp);
//         assert (penv);
//         set(atid, apxct, prvp);
//         _ptpccenv = penv;
//         _pin = pin;
//     }
    
// }; // EOF: pay_action_impl


// // UPD_WH_PAY_ACTION
// class upd_wh_pay_action_impl : public pay_action_impl
// {
// public:    
//     upd_wh_pay_action_impl() : pay_action_impl() { }
//     ~upd_wh_pay_action_impl() { }
//     w_rc_t trx_exec();    
//     midway_pay_rvp* _m_rvp;
//     void calc_keys() {
//         _down.push_back(_pin._home_wh_id);
//         _up.push_back(_pin._home_wh_id);
//     }
// }; // EOF: upd_wh_pay_action_impl

// // UPD_DIST_PAY_ACTION
// class upd_dist_pay_action_impl : public pay_action_impl
// {
// public:   
//     upd_dist_pay_action_impl() : pay_action_impl() { }
//     ~upd_dist_pay_action_impl() { }
//     w_rc_t trx_exec();    
//     midway_pay_rvp* _m_rvp;
//     void calc_keys() {
//         _down.push_back(_pin._home_wh_id);
//         _down.push_back(_pin._home_d_id);
//         _up.push_back(_pin._home_wh_id);
//         _up.push_back(_pin._home_d_id);
//     }
// }; // EOF: upd_wh_pay_action_impl

// // UPD_CUST_PAY_ACTION
// class upd_cust_pay_action_impl : public pay_action_impl
// {
// public:    
//     upd_cust_pay_action_impl() : pay_action_impl() { }
//     ~upd_cust_pay_action_impl() { }
//     w_rc_t trx_exec();   
//     midway_pay_rvp* _m_rvp;
//     void calc_keys() {
//         _down.push_back(_pin._home_wh_id);
//         _down.push_back(_pin._home_d_id);
//         _down.push_back(_pin._c_id);
//         _up.push_back(_pin._home_wh_id);
//         _up.push_back(_pin._home_d_id);
//         _up.push_back(_pin._c_id);
//     }
// }; // EOF: upd_cust_pay_action_impl

// // INS_HIST_PAY_ACTION
// class ins_hist_pay_action_impl : public pay_action_impl
// {
// public:    
//     ins_hist_pay_action_impl() : pay_action_impl() { }
//     ~ins_hist_pay_action_impl() { }
//     w_rc_t trx_exec();    
//     tpcc_warehouse_tuple _awh;
//     tpcc_district_tuple _adist;    
//     void calc_keys() {
//         _down.push_back(_pin._home_wh_id);
//         _up.push_back(_pin._home_wh_id);
//     }
// }; // EOF: ins_hist_pay_action_impl



EXIT_NAMESPACE(dora);

#endif /** __DORA_TPCC_NEW_ORDER_H */

