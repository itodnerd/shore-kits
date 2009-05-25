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

/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file:   shore_kits.cpp
 *
 *  @brief:  Shore kits
 *
 *  @author: Ippokratis Pandis, Sept 2008
 *
 */

#include "sm/shore/shore_shell.h"

#include "workload/tpcc/shore_tpcc_env.h"
#include "workload/tpcc/shore_tpcc_client.h"

#include "workload/tm1/shore_tm1_env.h"
#include "workload/tm1/shore_tm1_client.h"

#include "workload/tpcb/shore_tpcb_env.h"
#include "workload/tpcb/shore_tpcb_client.h"


using namespace shore;

using namespace tpcc;
using namespace tm1;
using namespace tpcb;


//////////////////////////////


// Value-definitions of the different Sysnames
enum SysnameValue { snBaseline };

// Map to associate string with then enum values

static map<string,SysnameValue> mSysnameValue;

void initsysnamemap() 
{
    mSysnameValue["baseline"] = snBaseline;
}



//////////////////////////////


// Value-definitions of the different Benchmarks
enum BenchmarkValue { bmTPCC, bmTM1, bmTPCB };

// Map to associate string with then enum values

static map<string,BenchmarkValue> mBenchmarkValue;

void initbenchmarkmap() 
{
    mBenchmarkValue["tpcc"]  = bmTPCC;
    mBenchmarkValue["tm1"]   = bmTM1;
    mBenchmarkValue["tpcb"]  = bmTPCB;
}



//////////////////////////////

template<class Client,class DB>
class kit_t : public shore_shell_t
{ 
private:
    DB* _dbinst;

public:

    kit_t(const char* prompt) 
        : shore_shell_t(prompt)
    {
        // load supported trxs maps
        load_trxs_map();
    }
    ~kit_t() { }


    virtual const int inst_test_env(int argc, char* argv[]);
    virtual const int load_trxs_map(void);


    // impl of supported commands
    virtual int _cmd_TEST_impl(const int iQueriedSF, const int iSpread,
                               const int iNumOfThreads, const int iNumOfTrxs,
                               const int iSelectedTrx, const int iIterations);
    virtual int _cmd_MEASURE_impl(const int iQueriedSF, const int iSpread,
                                  const int iNumOfThreads, const int iDuration,
                                  const int iSelectedTrx, const int iIterations);    

    virtual int process_cmd_LOAD(const char* command, const char* command_tag);        



    // cmd - RESTART
    struct restart_cmd_t : public command_handler_t {
        DB* _pdb;
        restart_cmd_t(DB* adb) : _pdb(adb) { };
        ~restart_cmd_t() { }
        void setaliases() { 
            _name = string("restart"); _aliases.push_back("restart"); }
        const int handle(const char* cmd) { 
            assert (_pdb); _pdb->stop(); _pdb->start(); 
            return (SHELL_NEXT_CONTINUE); }
        const string desc() { return (string("Restart")); }               
    };
    guard<restart_cmd_t> _restarter;

    // cmd - INFO
    struct info_cmd_t : public command_handler_t {
        DB* _pdb;
        info_cmd_t(DB* adb) : _pdb(adb) { };
        ~info_cmd_t() { }
        void setaliases() { 
            _name = string("info"); _aliases.push_back("info"); _aliases.push_back("i"); }
        const int handle(const char* cmd) { 
            assert (_pdb); _pdb->info(); return (SHELL_NEXT_CONTINUE); }
        const string desc() { return (string("Prints info about the state of db instance")); }
    };
    guard<info_cmd_t> _informer;

    // cmd - STATS
    struct stats_cmd_t : public command_handler_t {
        DB* _pdb;
        stats_cmd_t(DB* adb) : _pdb(adb) { };
        ~stats_cmd_t() { }
        void setaliases() { 
            _name = string("stats"); _aliases.push_back("stats"); _aliases.push_back("st"); }
        const int handle(const char* cmd) { 
            assert (_pdb); _pdb->statistics(); return (SHELL_NEXT_CONTINUE); }
        const string desc() { return (string("Prints gathered statistics")); }
    };
    guard<stats_cmd_t> _stater;

    // cmd - DUMP
    struct dump_cmd_t : public command_handler_t {
        DB* _pdb;
        dump_cmd_t(DB* adb) : _pdb(adb) { };
        ~dump_cmd_t() { }
        void setaliases() { 
            _name = string("dump"); _aliases.push_back("dump"); _aliases.push_back("d"); }
        const int handle(const char* cmd) { 
            assert (_pdb); _pdb->dump(); return (SHELL_NEXT_CONTINUE); }
        const string desc() { return (string("Dumps db instance data")); }
    };
    guard<dump_cmd_t> _dumper;
    

    // @class: {measure,test,trxs}_cmd_t
    // @brief: Hack so that the three basic commands (measure,test,trxs)
    //         also appear at help.
    struct measure_cmd_t : public command_handler_t {
        kit_t* _kit;
        measure_cmd_t(kit_t* akit) : _kit(akit) { }
        void setaliases() { _name = string("measure"); 
            _aliases.push_back("measure"); _aliases.push_back("m"); }
        const int handle(const char* cmd) { 
            _kit->pre_process_cmd();
            return (_kit->process_cmd_MEASURE(cmd,cmd)); }
        void usage() { _kit->usage_cmd_MEASURE(); }
        const string desc() { return string("Duration-based Measument (powerrun)"); }
    };
    guard<measure_cmd_t> _measurer;

    struct test_cmd_t : public command_handler_t {
        kit_t* _kit;
        test_cmd_t(kit_t* akit) : _kit(akit) { }
        void setaliases() { _name = string("test"); _aliases.push_back("test"); }
        const int handle(const char* cmd) { 
            _kit->pre_process_cmd();
            return (_kit->process_cmd_TEST(cmd,cmd)); }
        void usage() { _kit->usage_cmd_TEST(); }
        const string desc() { return string("NumOfXcts-based Measument (powerrun)"); }
    };
    guard<test_cmd_t> _tester;

    struct trxs_cmd_t : public command_handler_t {
        kit_t* _kit;
        trxs_cmd_t(kit_t* akit) : _kit(akit) { }
        void setaliases() { _name = string("trxs"); _aliases.push_back("trxs"); }
        const int handle(const char* cmd) { 
            _kit->pre_process_cmd();
            return (_kit->process_cmd_TRXS(cmd,cmd)); }
        void usage() { TRACE( TRACE_ALWAYS, "usage: trxs\n"); }
        const string desc() { return string("Lists the avaiable transactions in the benchmark"); }
    };
    guard<trxs_cmd_t> _trxser;



    virtual const int register_commands() 
    {
        shore_shell_t::register_commands();

        // FROM SHORE_SHELL
        _fakeioer = new fake_iodelay_cmd_t(_dbinst);
        _fakeioer->setaliases();
        add_cmd(_fakeioer.get());


        // TEMPLATE-BASED
        _restarter = new restart_cmd_t(_dbinst);
        _restarter->setaliases();
        add_cmd(_restarter.get());

        _informer = new info_cmd_t(_dbinst);
        _informer->setaliases();
        add_cmd(_informer.get());

        _stater = new stats_cmd_t(_dbinst);
        _stater->setaliases();
        add_cmd(_stater.get());

        _dumper = new dump_cmd_t(_dbinst);
        _dumper->setaliases();
        add_cmd(_dumper.get());


        // JUST FOR 'help' cmd
        _measurer = new measure_cmd_t(this);
        _measurer->setaliases();
        add_cmd(_measurer.get());

        _tester = new test_cmd_t(this);
        _tester->setaliases();
        add_cmd(_tester.get());

        _trxser = new trxs_cmd_t(this);
        _trxser->setaliases();
        add_cmd(_trxser.get());


        return (0);
    }

}; // EOF: kit_t



/********************************************************************* 
 *
 *  @fn:      load_trxs_map
 *
 *  @brief:   Instanciates a client and calls the function that loads
 *            the map of supported trxs.
 *
 *  @returns: The number of supported trxs
 *
 *********************************************************************/

template<class Client,class DB>
const int kit_t<Client,DB>::load_trxs_map(void)
{
    CRITICAL_SECTION(shell_cs,_lock);
    // gets the supported trxs from the client
    return (Client::load_sup_xct(_sup_trxs));
}

/********************************************************************* 
 *
 *  @fn:      inst_test_env
 *
 *  @brief:   Instanciates the Shore environment, 
 *            Opens the database and sets the appropriate SFs
 *  
 *  @returns: 1 on error
 *
 *********************************************************************/

template<class Client,class DB>
const int kit_t<Client,DB>::inst_test_env(int argc, char* argv[]) 
{    
    // 1. Instanciate the Shore Environment
    _env = _dbinst = new DB(SHORE_CONF_FILE);

    // 2. Initialize the Shore Environment
    // the initialization must be executed in a shore context
    db_init_smt_t* initializer = new db_init_smt_t(c_str("init"), _env);
    assert (initializer);
    initializer->fork();
    initializer->join(); 
    int rv = initializer->rv();
    delete (initializer);
    initializer = NULL;

    if (rv) {
        TRACE( TRACE_ALWAYS, "Exiting...\n");
        return (rv);
    }

    // 3. set SF - if param valid
    assert (_dbinst);
    if (argc>1) {
        int queriedSF = atoi(argv[1]);
        _dbinst->set_qf(queriedSF);
    }
    _theSF = _dbinst->get_sf();


    // 4. Load supported trxs
    if (!load_trxs_map()) {
        TRACE( TRACE_ALWAYS, "No supported trxs...\nExiting...");
        return (1);
    }

    // 5. Now that everything is set, register any additional commands
    register_commands();

    // 6. Start the VAS
    return (_dbinst->start());
}



namespace shore {
    extern void shell_expect_clients(int count);
    extern void shell_await_clients();
}






/********************************************************************* 
 *
 * COMMANDS
 *
 *********************************************************************/

/** cmd: TEST **/

template<class Client,class DB>
int kit_t<Client,DB>::_cmd_TEST_impl(const int iQueriedSF, 
                                     const int iSpread,
                                     const int iNumOfThreads, 
                                     const int iNumOfTrxs,
                                     const int iSelectedTrx, 
                                     const int iIterations)
{
    // print test information
    print_TEST_info(iQueriedSF, iSpread, iNumOfThreads, 
                    iNumOfTrxs, iSelectedTrx, iIterations);

    _dbinst->newrun();
    _dbinst->upd_sf();
    _dbinst->set_qf(iQueriedSF);

    Client* testers[MAX_NUM_OF_THR];
    for (int j=0; j<iIterations && !base_client_t::is_test_aborted(); j++) {

        TRACE( TRACE_ALWAYS, "Iteration [%d of %d]\n",
               (j+1), iIterations);

        // reset starting wh id
        int wh_id = 0;

        _env->reset_stats();
        //xct_stats statsb = shell_get_xct_stats();

        // set measurement state to measure - start counting everything
        _env->set_measure(MST_MEASURE);
	usleep(10000); // let all the threads come back
	stopwatch_t timer;

        // 1. create and fork client clients
        for (int i=0; i<iNumOfThreads; i++) {
            // create & fork testing threads
            if (iSpread)
                wh_id = (i%iQueriedSF)+1;

            testers[i] = new Client(c_str("CL-%d",i), i, _dbinst, 
                                    MT_NUM_OF_TRXS, iSelectedTrx, iNumOfTrxs,
                                    wh_id, iQueriedSF);
            assert (testers[i]);
            testers[i]->fork();
        }

        // 2. join the tester threads
        for (int i=0; i<iNumOfThreads; i++) {
            testers[i]->join();
            if (testers[i]->rv()) {
                TRACE( TRACE_ALWAYS, "Error in testing...\n");
                TRACE( TRACE_ALWAYS, "Exiting...\n");
                assert (false);
            }    
            delete (testers[i]);
        }

	double delay = timer.time();
        //xct_stats stats = shell_get_xct_stats();
        _env->print_throughput(iQueriedSF,iSpread,iNumOfThreads,delay);

        // flush the log before the next iteration
	_env->set_measure(MST_PAUSE);
        TRACE( TRACE_DEBUG, "db checkpoint - start\n");
        _env->checkpoint();
        TRACE( TRACE_ALWAYS, "Checkpoint\n");
    }

    // set measurement state
    _env->set_measure(MST_DONE);
    return (SHELL_NEXT_CONTINUE);
}


/** cmd: MEASURE **/

template<class Client,class DB>
int kit_t<Client,DB>::_cmd_MEASURE_impl(const int iQueriedSF, 
                                        const int iSpread,
                                        const int iNumOfThreads, 
                                        const int iDuration,
                                        const int iSelectedTrx, 
                                        const int iIterations)
{
    // print measurement info
    print_MEASURE_info(iQueriedSF, iSpread, iNumOfThreads, iDuration, 
                       iSelectedTrx, iIterations);

    _dbinst->newrun();
    _dbinst->upd_sf();
    _dbinst->set_qf(iQueriedSF);

    // create and fork client threads
    Client* testers[MAX_NUM_OF_THR];
    // reset starting wh id
    int wh_id = 0;
    
    // set measurement state
    _env->set_measure(MST_WARMUP);
    shell_expect_clients(iNumOfThreads);
    
    // 1. create and fork client threads
    for (int i=0; i<iNumOfThreads; i++) {
        // create & fork testing threads
        if (iSpread)
            wh_id = (i%iQueriedSF)+1;

        testers[i] = new Client(c_str("%s-%d", _cmd_prompt,i), i, _dbinst, 
                                MT_TIME_DUR, iSelectedTrx, 0,
                                wh_id, iQueriedSF);
        assert (testers[i]);
        testers[i]->fork();
    }
    
    // give them some time (2secs) to start-up
    shell_await_clients();
    
    
    // 2. run iterations
    for (int j=0; j<iIterations; j++) {
        sleep(1);
        TRACE( TRACE_ALWAYS, "Iteration [%d of %d]\n",
               (j+1), iIterations);

        // set measurement state
	TRACE(TRACE_ALWAYS, "begin measurement\n");
        _env->set_measure(MST_MEASURE);

        _env->reset_stats();
	stopwatch_t timer;
	sleep(iDuration);

	double delay = timer.time();
	TRACE(TRACE_ALWAYS, "end measurement\n");
        _env->print_throughput(iQueriedSF,iSpread,iNumOfThreads,delay);
	       
        // flush the log before the next iteration
        TRACE( TRACE_DEBUG, "db checkpoint - start\n");
        _env->checkpoint();
        TRACE( TRACE_ALWAYS, "Checkpoint\n");
    }


    // 3. join the tester threads
    _env->set_measure(MST_DONE);
    for (int i=0; i<iNumOfThreads; i++) {
        testers[i]->join();
        //fprintf(stderr,".");
        if (testers[i]->rv()) {
            TRACE( TRACE_ALWAYS, "Error in testing...\n");
            assert (false);
        }    
        assert (testers[i]);
        delete (testers[i]);
    }

    // set measurement state
    _env->set_measure(MST_DONE);
    return (SHELL_NEXT_CONTINUE);
}

template<class Client,class DB>
int kit_t<Client,DB>::process_cmd_LOAD(const char* command, 
                                       const char* command_tag)
{
    TRACE( TRACE_DEBUG, "Not implemented\n");
    return (SHELL_NEXT_CONTINUE);
}


//////////////////////////////

typedef kit_t<baseline_tpcc_client_t,ShoreTPCCEnv> baselineTPCCKit;
typedef kit_t<baseline_tm1_client_t,ShoreTM1Env> baselineTM1Kit;
typedef kit_t<baseline_tpcb_client_t,ShoreTPCBEnv> baselineTPCBKit;

//////////////////////////////


int main(int argc, char* argv[]) 
{
    thread_init();

    TRACE_SET( TRACE_ALWAYS | TRACE_STATISTICS 
               //               | TRACE_NETWORK 
               //              | TRACE_QUERY_RESULTS
               //              | TRACE_PACKET_FLOW
               //               | TRACE_RECORD_FLOW
               //               | TRACE_TRX_FLOW
               //| TRACE_DEBUG
               );

    // 1. Get env vars
    initsysnamemap();
    initbenchmarkmap();
    envVar* ev = envVar::instance();       
    string benchmarkname = ev->getSysVar("benchmark");


    // 2. Initialize shell
    guard<shore_shell_t> kit = NULL;


    TRACE( TRACE_ALWAYS, "Starting (%s) kit\n", 
           benchmarkname.c_str());


    // TPC-C
    if (benchmarkname.compare("tpcc")==0) {
            kit = new baselineTPCCKit("(tpcc) ");
    }

    // TM1
    if (benchmarkname.compare("tm1")==0) {
            kit = new baselineTM1Kit("(tm1) ");
    }

    // TPC-B
    if (benchmarkname.compare("tpcb")==0) {
            kit = new baselineTPCBKit("(tpcb) ");
    }

    assert (kit.get());


    // 2. Instanciate and start the Shore environment
    if (kit->inst_test_env(argc, argv))
        return (4);


    // 4. Make sure data is loaded
    w_rc_t rcl = kit->db()->loaddata();
    if (rcl.is_error()) {
        return (5);
    }

    // set the global variable to the kit's db - for alarm() to work
    _g_shore_env = kit->db();

    // 5. Start processing commands
    kit->start();

    // 6. the Shore environment will close at the destructor of the kit
    return (0);
}


