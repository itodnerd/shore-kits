/* -*- mode:C++; c-basic-offset:4 -*- */

/** @file   part_table.h
 *
 *  @brief  Declaration of each table in DORA, 
 *
 *  @note   Implemented as a vector of partitions and a routing
 *
 *  @author Ippokratis Pandis, Oct 2008
 */


#ifndef __DORA_PART_TABLE_H
#define __DORA_PART_TABLE_H

#include <cstdio>

#include "util.h"

#include "sm/shore/shore_env.h"
#include "sm/shore/shore_table.h"

#include "dora.h"


ENTER_NAMESPACE(dora);


using namespace shore;


/******************************************************************** 
 *
 * @class: part_table_t
 *
 * @brief: Class for representing a table as a set of (data) partitions
 *
 ********************************************************************/

template <typename Partition>
class part_table_t
{
public:

    typedef vector<Partition*> PartitionPtrVector;
    typedef typename vector<Partition*>::iterator pvpIt;
    typedef typename Partition::Action Action;

protected:

    ShoreEnv*          _env;    
    table_desc_t*      _table;

    // the vector of partitions
    PartitionPtrVector _ppvec;

    int                _pcnt;
    tatas_lock         _pcgf_lock;

    // processor binding
    processorid_t      _start_prs_id;
    processorid_t      _next_prs_id;
    int                _prs_range;
    tatas_lock         _next_prs_lock;


    // (ip) TODO
    //mapping table - which is build at runtime

public:

    part_table_t(ShoreEnv* env, table_desc_t* ptable,
                 const processorid_t aprs,
                 const int acpurange) 
        : _env(env), _table(ptable), 
          _start_prs_id(aprs), _next_prs_id(aprs), 
          _prs_range(acpurange),
          _pcnt(0)
    {
        assert (_env);
        assert (_table);        
        assert (aprs<=_env->get_max_cpu_count());
        assert (acpurange<=_env->get_active_cpu_count());
    }

    virtual ~part_table_t() { }    


    /** Access methods */
    PartitionPtrVector* get_vector() const { return (&_ppvec); }
    Partition* get_part(const int pos) const {
        assert (pos<_ppvec.size());
        return (_ppvec[pos]);
    }

    /** Control table */

    // configure partitions
    virtual const int config(const int apcnt);

    // add one partition
    virtual const int add_one_part(Partition* apartition) {
        return (_add_one_part(apartition));
    }

    // create one partition
    virtual const int create_one_part();

    // reset all partitions
    virtual const int reset();

    // move to another range of processors
    const int move(const processorid_t aprs, const int arange) {
        {
            CRITICAL_SECTION(next_prs_cs, _next_prs_lock);
            // 1. update processor and range
            _start_prs_id = aprs;
            _prs_range = arange;
        }
        // 2. reset
        return (reset());
    }

    // decide the next processor
    virtual processorid_t next_cpu(const processorid_t& aprd);

    // stops all partitions
    const int stop() {
        for (int i=0; i<_ppvec.size(); i++)
            _ppvec[i]->stop();
        return (0);
    }

    // prepares all partitions for a new run
    const int prepareNewRun() {
        for (int i=0; i<_ppvec.size(); i++)
            _ppvec[i]->prepareNewRun();
        return (0);
    }


    /////  Action-related methods

    // enqueues action, false on error
    inline const int enqueue(Action* paction, const int part) {
        assert (part<_pcnt);
        return (_ppvec[part]->enqueue(paction));
    }



    // For debugging

    // information
    void statistics() const {
        TRACE( TRACE_STATISTICS, "Table (%s)\n", _table->name());
        TRACE( TRACE_STATISTICS, "Parts (%d)\n", _pcnt-1);
        for (int i=0; i<_ppvec.size(); i++)
            _ppvec[i]->statistics();
    }        

    // information
    void info() const {
        TRACE( TRACE_STATISTICS, "Table (%s)\n", _table->name());
        TRACE( TRACE_STATISTICS, "Parts (%d)\n", _pcnt-1);
    }        

    // dumps information
    void dump() const {
        TRACE( TRACE_DEBUG, "Table (%s)\n", _table->name());
        TRACE( TRACE_DEBUG, "Parts (%d)\n", _pcnt-1);
        for (int i=0; i<_ppvec.size(); i++)
            _ppvec[i]->dump();
    }        

private:

    const int _create_one_part();
    const int _add_one_part(Partition* apartition);

}; // EOF: part_table_t



/** partition_t interface */



/****************************************************************** 
 *
 * @fn:    config()
 *
 * @brief: Configures the partitions
 *
 ******************************************************************/

template <typename Partition>
const int part_table_t<Partition>::config(const int apcnt)
{
    assert (_env);
    assert (_table);
    assert (apcnt);

    TRACE( TRACE_DEBUG, "Configuring...\n");

    for (int i=0; i<apcnt; i++) {
        create_one_part();
    }
    return (0);
}



/****************************************************************** 
 *
 * @fn:    create_one_part()
 *
 * @brief: Creates one partition and adds it to the vector
 *
 ******************************************************************/

template <typename Partition>
inline const int part_table_t<Partition>::create_one_part()
{
    return (_create_one_part());
}


/****************************************************************** 
 *
 * @fn:    reset()
 *
 * @brief: Resets the partitions
 *
 * @note:  Applies the partition distribution function (next_cpu())
 *
 ******************************************************************/

template <typename Partition>
const int part_table_t<Partition>::reset()
{
    TRACE( TRACE_DEBUG, "Reseting (%s)...\n", _table->name());
    _next_prs_id = _start_prs_id;
    for (int i=0; i<_ppvec.size(); i++) {
        _ppvec[i]->reset(_next_prs_id);
        CRITICAL_SECTION(next_prs_cs, _next_prs_lock);
        _next_prs_id = next_cpu(_next_prs_id);
    }    
    return (0);
}



/****************************************************************** 
 *
 * @fn:    next_cpu()
 *
 * @brief: The partition distribution function
 *
 * @note:  Very simple (just increases processor id by one) 
 *
 * @note:  This decision  can be based among others on:
 *
 *         - aprd                    - the current cpu
 *         - _env->_max_cpu_count    - the maximum cpu count (hard-limit)
 *         - _env->_active_cpu_count - the active cpu count (soft-limit)
 *         - this->_start_prs_id     - the first assigned cpu for the table
 *         - this->_prs_range        - a range of cpus assigned for the table
 *
 ******************************************************************/

template <typename Partition>
processorid_t part_table_t<Partition>::next_cpu(const processorid_t& aprd) 
{
    int binding = envVar::instance()->getVarInt("dora-cpu-binding",0);
    if (binding==0)
        return (PBIND_NONE);

    int partition_step = envVar::instance()->getVarInt("dora-cpu-partition-step",
                                                       DF_CPU_STEP_PARTITIONS);    
    processorid_t nextprs = ((aprd+partition_step) % _env->get_active_cpu_count());
    return (nextprs);
}



/** part_table_t helpers */


/****************************************************************** 
 *
 * @fn:    _create_one_part()
 *
 * @brief: Creates one partition
 *
 ******************************************************************/

template <typename Partition>
const int part_table_t<Partition>::_create_one_part()
{
    //    TRACE( TRACE_DEBUG, "(%s) creating one partition...\n", _table->name());  
    assert (0); // TODO
    return (0);
}



/****************************************************************** 
 *
 * @fn:     _add_one_part()
 *
 * @brief:  Add one partition
 *
 * @return: 0 on success
 *
 ******************************************************************/

template <typename Partition>
const int part_table_t<Partition>::_add_one_part(Partition* apartition)
{
    assert (apartition);
    CRITICAL_SECTION(conf_cs, _pcgf_lock);
    _ppvec.push_back(apartition);
    ++_pcnt;
    return (0);
}





EXIT_NAMESPACE(dora);

#endif /** __DORA_PART_TABLE_H */

