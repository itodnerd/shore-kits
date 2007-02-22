/* -*- mode:C++; c-basic-offset:4 -*- */

#include "workload/tpch/tpch_tbl_parsers.h"
#include "workload/tpch/tpch_struct.h"
#include "workload/tpch/tpch_type_convert.h"
#include "util/trace.h"



/* internal constants */

#define MAX_LINE_LENGTH 1024



/* helper functions */

static void progress_reset();
static void progress_update();
static void progress_done();
static void store_string(char* dest, char* src);



/* definitions of exported functions */

void tpch_parse_tbl_CUSTOMER(Db* db, FILE* fd) {
  
    char linebuffer[MAX_LINE_LENGTH];

    TRACE(TRACE_DEBUG, "Populating CUSTOMER...\n");
    progress_reset();
    tpch_customer_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));
        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.C_CUSTKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.C_NAME, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.C_ADDRESS, tmp);
        tmp = strtok(NULL, "|");
        tup.C_NATIONKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.C_PHONE, tmp);
        tmp = strtok(NULL, "|");
        tup.C_ACCTBAL = atof(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.C_MKTSEGMENT, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.C_COMMENT, tmp);

        // insert tuple into database
        Dbt key(&tup.C_CUSTKEY, sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_LINEITEM(Db* db, FILE* fd) {
  
    char linebuffer[MAX_LINE_LENGTH];

    printf("Populating LINEITEM...\n");
    progress_reset();
    tpch_lineitem_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.L_ORDERKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.L_PARTKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.L_SUPPKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.L_LINENUMBER = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.L_QUANTITY = atof(tmp);
        tmp = strtok(NULL, "|");
        tup.L_EXTENDEDPRICE = atof(tmp);
        tmp = strtok(NULL, "|");
        tup.L_DISCOUNT = atof(tmp);
        tmp = strtok(NULL, "|");
        tup.L_TAX = atof(tmp);
        tmp = strtok(NULL, "|");
        tup.L_RETURNFLAG = tmp[0];
        tmp = strtok(NULL, "|");
        tup.L_LINESTATUS = tmp[0];
        tmp = strtok(NULL, "|");
        tup.L_SHIPDATE = datestr_to_timet(tmp);
        tmp = strtok(NULL, "|");
        tup.L_COMMITDATE = datestr_to_timet(tmp);
        tmp = strtok(NULL, "|");
        tup.L_RECEIPTDATE = datestr_to_timet(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.L_SHIPINSTRUCT, tmp);
        tmp = strtok(NULL, "|");
        tup.L_SHIPMODE = modestr_to_shipmode(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.L_COMMENT, tmp);

        // insert tuple into database
        // key is composed of 2 fields 	L_ORDERKEY, L_LINENUMBER
        Dbt key(&tup.L_ORDERKEY, 2 * sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_NATION(Db* db, FILE* fd) {
  
    char linebuffer[MAX_LINE_LENGTH];
 
    printf("Populating NATION...\n");
    progress_reset();
    tpch_nation_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.N_NATIONKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.N_NAME = nnamestr_to_nname(tmp);
        tmp = strtok(NULL, "|");
        tup.N_REGIONKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.N_COMMENT, tmp);

        // insert tuple into database
        Dbt key(&tup.N_NATIONKEY, sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_ORDERS(Db* db, FILE* fd) {
    
    char linebuffer[MAX_LINE_LENGTH];

    printf("Populating ORDERS...\n");
    progress_reset();
    tpch_orders_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.O_ORDERKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.O_CUSTKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.O_ORDERSTATUS = tmp[0];
        tmp = strtok(NULL, "|");
        tup.O_TOTALPRICE = atof(tmp);
        tmp = strtok(NULL, "|");
        tup.O_ORDERDATE = datestr_to_timet(tmp);
        tmp = strtok(NULL, "|");
        tup.O_ORDERPRIORITY = prioritystr_to_orderpriorty(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.O_CLERK, tmp);
        tmp = strtok(NULL, "|");
        tup.O_SHIPPRIORITY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.O_COMMENT, tmp);

        // insert tuple into database
        // key is composed of 2 fields 	O_ORDERKEY and O_CUSTKEY
        Dbt key(&tup.O_ORDERKEY, 2 * sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_PART(Db* db, FILE* fd) {

    char linebuffer[MAX_LINE_LENGTH];
 
    printf("Populating PART...\n");
    progress_reset();
    tpch_part_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.P_PARTKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_NAME, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_MFGR, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_BRAND, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_TYPE, tmp);
        tmp = strtok(NULL, "|");
        tup.P_SIZE = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_CONTAINER, tmp);
        tmp = strtok(NULL, "|");
        tup.P_RETAILPRICE = atof(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.P_COMMENT, tmp);

        // insert tuple into database
        Dbt key(&tup.P_PARTKEY, sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_PARTSUPP(Db* db, FILE* fd) {
    
    char linebuffer[MAX_LINE_LENGTH];

    printf("Populating PARTSUPP...\n");
    progress_reset();
    tpch_partsupp_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.PS_PARTKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.PS_SUPPKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.PS_AVAILQTY = atoi(tmp);
        tmp = strtok(NULL, "|");
        tup.PS_SUPPLYCOST = atof(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.PS_COMMENT, tmp);
	
        // insert tuple into database
        // key is composed of 2 fields 	PS_PARTKEY and PS_SUPPKEY
        Dbt key(&tup.PS_PARTKEY, 2 * sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_REGION(Db* db, FILE* fd) {

    char linebuffer[MAX_LINE_LENGTH];

    printf("Populating REGION...\n");
    progress_reset();
    tpch_region_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.R_REGIONKEY= atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.R_NAME, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.R_COMMENT, tmp);

        // insert tuple into database
        Dbt key(&tup.R_REGIONKEY, sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        progress_update();
    }

    progress_done();
}



void tpch_parse_tbl_SUPPLIER(Db* db, FILE* fd) {

    char linebuffer[MAX_LINE_LENGTH];
    static int count = 0;

    printf("Populating SUPPLIER...\n");
    progress_reset();
    tpch_supplier_tuple tup;

    while (fgets(linebuffer, MAX_LINE_LENGTH, fd)) {
        // clear the tuple
        memset(&tup, 0, sizeof(tup));

        // split line into tab separated parts
        char* tmp = strtok(linebuffer, "|");
        tup.S_SUPPKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.S_NAME, tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.S_ADDRESS, tmp);
        tmp = strtok(NULL, "|");
        tup.S_NATIONKEY = atoi(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.S_PHONE, tmp);
        tmp = strtok(NULL, "|");
        tup.S_ACCTBAL= atof(tmp);
        tmp = strtok(NULL, "|");
        store_string(tup.S_COMMENT, tmp);

        // insert tuple into database
        Dbt key(&tup.S_SUPPKEY, sizeof(int));
        Dbt data(&tup, sizeof(tup));
        db->put(NULL, &key, &data, 0);

        if (count++ < 10 && 0) {
            TRACE(TRACE_ALWAYS, "Inserting supplier tuple (%d|%s|%s|%d|%s|%lf|%s)\n",
                  tup.S_SUPPKEY,
                  tup.S_NAME,
                  tup.S_ADDRESS,
                  tup.S_NATIONKEY,
                  tup.S_PHONE,
                  tup.S_ACCTBAL.to_double(),
                  tup.S_COMMENT);
        }

        progress_update();
    }

    progress_done();
}



/* definitions of helper functions */


#define PROGRESS_INTERVAL 100000

static unsigned long progress = 0;

static void progress_reset() {
    progress = 0;
}

static void progress_update() {
    if ( (progress++ % PROGRESS_INTERVAL) == 0 ) {
        printf(".");
        fflush(stdout);
        progress = 1; // prevent overflow
    }
}

static void progress_done() {
    printf("done\n");
    fflush(stdout);
}

static void store_string(char* dest, char* src) {
    int len = strlen(src);
    strncpy(dest, src, len);
    dest[len] = '\0';
}
