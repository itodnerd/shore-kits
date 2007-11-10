/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file inmem_tpcc_env.h
 *
 *  @brief Definition of the InMemory TPC-C environment
 *
 *  @author Ippokratis Pandis (ipandis)
 */

#ifndef __INMEM_TPCC_ENV_H
#define __INMEM_TPCC_ENV_H

#include "util/guard.h"
#include "util/namespace.h"

#include "stages/tpcc/common/tpcc_scaling_factor.h"
#include "stages/tpcc/common/tpcc_struct.h"

// For the in-memory data structures
#include "util/latchedarray.h"
#include "util/bptree.h"
#include <boost/array.hpp>



ENTER_NAMESPACE(tpcc);

/* constants */

// (ip) In Lomond the data files and directory have different name
// just a quick hack to run it in both
#define INMEM_TPCC_DATA_DIR "tpcc_sf"
#define INMEM_TPCC_DATA_WAREHOUSE "WAREHOUSE.dat"
#define INMEM_TPCC_DATA_DISTRICT "DISTRICT.dat"
#define INMEM_TPCC_DATA_CUSTOMER "CUSTOMER.dat"
#define INMEM_TPCC_DATA_HISTORY "HISTORY.dat"

// these are saved-out btree/array state and should load *way* faster
#define INMEM_TPCC_SAVE_DIR INMEM_TPCC_DATA_DIR
#define INMEM_TPCC_SAVE_WAREHOUSE "WAREHOUSE.save"
#define INMEM_TPCC_SAVE_DISTRICT "DISTRICT.save"
#define INMEM_TPCC_SAVE_CUSTOMER "CUSTOMER.save"
#define INMEM_TPCC_SAVE_HISTORY "HISTORY.save"


#define WAREHOUSE_FANOUT 1
#define DISTRICT_FANOUT 10

#define cCustNodeEntries 20
#define cCustNodePad 4
#define cCustLeafEntries 40
#define cCustLeafPad 4

#define cHistNodeEntries 50
#define cHistNodePad 8
#define cHistLeafEntries 100
#define cHistLeafPad 8

#define cArch 64


////////////////////////////////////////////////////////////////////////

/** @class InMemTPCCEnv
 *  
 *  @brief Class that contains the various data structures used in the
 *  In-Memory TPC-C environment.
 */

class InMemTPCCEnv 
{
private:    

    static const int WAREHOUSE = 0;
    static const int DISTRICT = 1;
    static const int CUSTOMER = 2;
    static const int HISTORY = 3;

    static const int INMEM_PAYMENT_TABLES = 4;
    
    /** Private variables */
    bool _initialized;

public:
    
    int loaddata(c_str loadDir);
    int savedata(c_str saveDir);
    int restoredata(c_str restoreDir);


    /** Member variables */

    /* Arrays: WAREHOUSE, DISTRICT */

    typedef latchedArray<tpcc_warehouse_tuple, TPCC_SCALING_FACTOR, WAREHOUSE_FANOUT> warehouse_array_t;
    warehouse_array_t im_warehouses;

    typedef latchedArray<tpcc_district_tuple, TPCC_SCALING_FACTOR, DISTRICT_FANOUT> district_array_t;
    district_array_t im_districts;

    /* BPTrees: CUSTOMER, HISTORY */
    typedef BPlusTree<tpcc_customer_tuple_key, tpcc_customer_tuple_body,
                      cCustNodeEntries, cCustLeafEntries,
                      cCustNodePad, cCustLeafPad, cArch> customer_tree_t;
    customer_tree_t im_customers;

    typedef BPlusTree<tpcc_history_tuple_key, tpcc_history_tuple_body,
                      cHistNodeEntries, cHistLeafEntries,
                      cHistNodePad, cHistLeafPad, cArch> history_tree_t;
    history_tree_t im_histories;



    /** Construction  */

    InMemTPCCEnv() {
	_initialized = false;
        im_customers.set_name(c_str("customer"));
        im_histories.set_name(c_str("history"));
	im_warehouses.set_name("warehouse");
	im_districts.set_name("district");

    }


    ~InMemTPCCEnv() { }

    bool is_initialized() const { return _initialized; }
    void dump();


}; // EOF InMemTPCCEnv


EXIT_NAMESPACE(tpcc);


#endif
