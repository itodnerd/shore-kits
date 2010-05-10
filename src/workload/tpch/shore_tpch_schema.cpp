/* -*- mode:C++; c-basic-offset:4 -*-
   Shore-kits -- Benchmark implementations for Shore-MT
   
   Copyright (c) 2007-2009
   Data Intensive Applications and Systems Labaratory (DIAS)
   Ecole Polytechnique Federale de Lausanne
   
   All Rights Reserved.
   
   Permission to use, copy, modify and distribute this software and
   its documentation is hereby granted, provided that both the
   copyright notice and this permission notice appear in all copies of
   the software, derivative works or modified versions, and any
   portions thereof, and that both notices appear in supporting
   documentation.
   
   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS
   DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
   RESULTING FROM THE USE OF THIS SOFTWARE.
*/

/** @file:   shore_tpch_schema.h
 *
 *  @brief:  Declaration of the TPC-H tables
 *
 *  @author: Ippokratis Pandis, June 2009
 *
 */


/*********************************************************************
 *
 * TPC-H SCHEMA
 * 
 * This file contains the classes for tables in the TPC-H benchmark.
 * A class derived from tpch_table_t (which inherits from table_desc_t) 
 * is created for each table in the databases.
 *
 *********************************************************************/


/*
 * indices created on the tables are:
 *
 * 1. NATION 
 * a. primary (unique) index on nation(n_nationkey)
 * b. secondary index on nation(n_regionkey)
 *
 * 2. REGION
 * a. primary (unique) index on region(r_regionkey)
 * 
 * 3. PART
 * a. primary (unique) index on part(p_partkey)
 *
 * 4. SUPPLIER
 * a. primary (unique) index on supplier(s_suppkey)
 * b. secondary index on supplier(s_nationkey)
 *
 * 5. PARTSUPP
 * a. primary (unique) index on partsupp(ps_partkey, ps_suppkey)
 * b. secondary index on partsupp(ps_partkey)
 * c. secondary index on partsupp(ps_suppkey)
 *
 * 6. CUSTOMER
 * a. primary (unique) index on customer(c_custkey)
 * b. secondary index on customer(c_nationkey)
 *
 * 7. ORDERS
 * a. primary (unique) index on orders(o_orderkey)
 * b. secondary index on orders(o_custkey)
 *
 * 8. LINEITEM
 * a. primary (unique) index on lineitem(l_orderkey, l_linenumber)
 * b. secondary index on lineitem(l_orderkey)
 * c. secondary index on lineitem(l_partkey, l_suppkey)
 * d. secondary index on lineitem(l_shipdate)
 */


#include "workload/tpch/shore_tpch_schema.h"

using namespace shore;

ENTER_NAMESPACE(tpch);

nation_t::nation_t(string sysname) : 
    table_desc_t("NATION", TPCH_NATION_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,   "N_NATIONKEY");
    _desc[1].setup(SQL_FIXCHAR,  "N_NAME", 25);
    _desc[2].setup(SQL_INT,   "N_REGIONKEY");
    _desc[3].setup(SQL_FIXCHAR,  "N_COMMENT", 152);

    int keys[1] = { 0 }; // IDX { N_NATIONKEY }

    int fkeys[1] = { 2 }; // IDX { N_REGIONKEY }

    // depending on the system name, create the corresponding indexes 

    // baseline - Regular 
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
        // create unique index n_index on (n_nationkey)
        create_primary_idx("N_INDEX", 0, keys, 1);

        create_index("N_FK_REGION", 0, fkeys, 1, false);
    }
}


region_t::region_t(string sysname) : 
    table_desc_t("REGION", TPCH_REGION_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,   "R_REGIONKEY");     
    _desc[1].setup(SQL_FIXCHAR,  "R_NAME", 25);
    _desc[2].setup(SQL_FIXCHAR,  "R_COMMENT", 25);

    int keys[1] = { 0 }; // IDX { R_REGIONKEY }

    // baseline - Regular 
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name); 
        // create unique index r_index on (r_regionkey)
        create_primary_idx("R_INDEX", 0, keys, 1);
    }
}


supplier_t::supplier_t(string sysname) : 
    table_desc_t("SUPPLIER", TPCH_SUPPLIER_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,   "S_SUPPKEY");
    _desc[1].setup(SQL_FIXCHAR,  "S_NAME", 25);
    _desc[2].setup(SQL_FIXCHAR,  "S_ADDRESS", 40);
    _desc[3].setup(SQL_INT,   "S_NATIONKEY");
    _desc[4].setup(SQL_FIXCHAR,  "S_PHONE", 15);
    _desc[5].setup(SQL_FLOAT, "S_ACCTBAL");
    _desc[6].setup(SQL_FIXCHAR,  "S_COMMENT", 101);

    int keys[1] = {0}; // IDX { S_SUPPKEY}
				
    int fkeys[1] = {3}; // IDX { S_NATIONKEY }
                                
    // baseline - Regular
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
 
        // create unique index s_index on (s_suppkey)
        create_primary_idx("S_INDEX", 0, keys, 1);

        create_index("S_FK_NATION", 0, fkeys, 1, false);
    } 
}


part_t::part_t(string sysname) : 
    table_desc_t("PART", TPCH_PART_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,   "P_PARTKEY");
    _desc[1].setup(SQL_FIXCHAR,  "P_NAME", 55);  
    _desc[2].setup(SQL_FIXCHAR,  "P_MFGR", 25); 
    _desc[3].setup(SQL_FIXCHAR,  "P_BRAND", 10);   
    _desc[4].setup(SQL_FIXCHAR,  "P_TYPE", 25);    
    _desc[5].setup(SQL_INT,   "P_SIZE");    
    _desc[6].setup(SQL_FIXCHAR,  "P_CONTAINER", 10);  
    _desc[7].setup(SQL_FLOAT, "P_RETAILPRICE"); 
    _desc[8].setup(SQL_FIXCHAR,  "P_COMMENT", 23);
                                
    int keys[1] = { 0 }; // IDX { P_PARTKEY }
        
    if (sysname.compare("baseline") == 0) {
        TRACE(TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
            
        create_primary_idx("P_INDEX", 0, keys, 1);
    }
}

partsupp_t::partsupp_t(string sysname) : 
    table_desc_t("PARTSUPP", TPCH_PARTSUPP_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,    "PS_PARTKEY");
    _desc[1].setup(SQL_INT,    "PS_SUPPKEY");       
    _desc[2].setup(SQL_INT,    "PS_AVAILQTY");
    _desc[3].setup(SQL_FLOAT,  "PS_SUPPLYCOST");  
    _desc[4].setup(SQL_FIXCHAR,   "PS_COMMENT", 199); 
        
    int keys[2] = { 0, 1 }; // IDX { PS_PARTKEY, PS_SUPPKEY }
                
    int fkeys1[1] = { 0 }; // IDX { PS_PARTKEY }
        
    int fkeys2[1] = { 1 }; // IDX { PS_SUPPEKY }

    // baseline - regular indexes
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
            
        // create unique index ps_index on (ps_partkey)
        create_primary_idx("PS_INDEX", 0, keys, 2);
            
        create_index("PS_FK_PART", 0, fkeys1, 1, false);
            
        create_index("PS_FK_SUPP", 0, fkeys2, 1, false);
    }
}


customer_t::customer_t(string sysname) : 
    table_desc_t("CUSTOMER", TPCH_CUSTOMER_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,    "C_CUSTKEY");
    _desc[1].setup(SQL_FIXCHAR,   "C_NAME", 25);       
    _desc[2].setup(SQL_FIXCHAR,   "C_ADDRESS", 40);       
    _desc[3].setup(SQL_INT,    "C_NATIONKEY");  
    _desc[4].setup(SQL_FIXCHAR,   "C_PHONE", 15);  
    _desc[5].setup(SQL_FLOAT,  "C_ACCTBAL");   
    _desc[6].setup(SQL_FIXCHAR,   "C_MKTSEGMENT", 10);
    _desc[7].setup(SQL_FIXCHAR,   "C_COMMENT", 117);

    int keys[1] = { 0 }; // IDX { C_CUSTKEY }
        
    int fkeys[1] = { 3 }; // IDX { C_NATIONKEY }

    // baseline - regular indexes
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
 
        // create unique index c_index on (c_custkey)
        create_primary_idx("C_INDEX", 0, keys, 1);
 
        create_index("C_FK_NATION", 0, fkeys, 1, false);
    }
}


orders_t::orders_t(string sysname) : 
    table_desc_t("ORDERS", TPCH_ORDERS_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,   "O_ORDERKEY");
    _desc[1].setup(SQL_INT,   "O_CUSTKEY");       
    _desc[2].setup(SQL_CHAR,   "O_ORDERSTATUS");       
    _desc[3].setup(SQL_FLOAT, "O_TOTALPRICE");       
    _desc[4].setup(SQL_FIXCHAR,  "O_ORDERDATE", 15);
    _desc[5].setup(SQL_FIXCHAR,  "O_ORDERPRIORITY", 15); 
    _desc[6].setup(SQL_FIXCHAR,  "O_CLERK", 15);
    _desc[7].setup(SQL_INT,   "O_SHIPPRIORITY");
    _desc[8].setup(SQL_FIXCHAR,  "O_COMMENT", 79);

    int keys[1] = { 0 }; // IDX { O_ORDERKEY }
        
    int fkeys[1] = { 1 }; // IDX { O_CUSTKEY }
         

    // baseline - regular indexes
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);
 
        // create unique index o_index on (o_orderkey)
        create_primary_idx("O_INDEX", 0, keys, 1);
             
        create_index("O_FK_CUSTKEY", 0, fkeys, 1, false);
    }
}


lineitem_t::lineitem_t(string sysname) : 
    table_desc_t("LINEITEM", TPCH_LINEITEM_FCOUNT) 
{
    // table schema
    _desc[0].setup(SQL_INT,    "L_ORDERKEY");
    _desc[1].setup(SQL_INT,    "L_PARTKEY");
    _desc[2].setup(SQL_INT,    "L_SUPPKEY");
    _desc[3].setup(SQL_INT,    "L_LINENUMBER");
    _desc[4].setup(SQL_FLOAT,  "L_QUANTITY");
    _desc[5].setup(SQL_FLOAT,  "L_EXTENDEDPRICE");
    _desc[6].setup(SQL_FLOAT,  "L_DISCOUNT");
    _desc[7].setup(SQL_FLOAT,  "L_TAX");
    _desc[8].setup(SQL_CHAR,   "L_RETURNFLAG");
    _desc[9].setup(SQL_CHAR,   "L_LINESTATUS");
    _desc[10].setup(SQL_FIXCHAR,  "L_SHIPDATE", 15);
    _desc[11].setup(SQL_FIXCHAR,  "L_COMMITDATE", 15);
    _desc[12].setup(SQL_FIXCHAR,  "L_RECEIPTDATE", 15);
    _desc[13].setup(SQL_FIXCHAR,  "L_SHIPINSTRUCT", 25);
    _desc[14].setup(SQL_FIXCHAR,  "L_SHIPMODE", 10);
    _desc[15].setup(SQL_FIXCHAR,  "L_COMMENT", 44);

    int keys[2] = {0, 3}; // IDX { L_ORDERKEY, L_LINENUMBER }

    int fkeys1[1] = { 0 }; // IDX { L_ORDERKEY }

    int fkeys2[2] = {1, 2}; // IDX { L_PARTKEY, L_SUPPKEY }

    int keys2[1] = {10}; // IDX { L_SHIPDATE }


    // baseline - regular indexes
    if (sysname.compare("baseline")==0) {
        TRACE( TRACE_DEBUG, "Regular idxs for (%s)\n", _name);

        // create unique index l_index on ()
        create_primary_idx("L_INDEX", 0, keys, 2);

        create_index("L_FK_ORDERKEY", 0, fkeys1, 1, false);
        create_index("L_FK_PARKSUPP", 0, fkeys2, 2, false);

        create_index("L_IDX_SHIPDATE", 0, keys2, 1, false);
    }
}


EXIT_NAMESPACE(tpch);
