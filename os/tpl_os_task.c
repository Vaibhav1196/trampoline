/**
 * @file tpl_os_task_management.c
 *
 * @section desc File description
 *
 * Trampoline Task Management Calls
 *
 * @section copyright Copyright
 *
 * Trampoline OS
 *
 * Trampoline is copyright (c) IRCCyN 2005+
 * Copyright ESEO for function and data structures documentation
 * Trampoline is protected by the French intellectual property law.
 *
 * This software is distributed under the Lesser GNU Public Licence
 *
 * @section infos File informations
 *
 * $Date$
 * $Rev$
 * $Author$
 * $URL$
 */
 
#include "tpl_os.h"
#include "tpl_os_kernel.h"
#include "tpl_machine_interface.h"
#include "tpl_os_error.h"
#include "tpl_os_hooks.h"

#define OS_START_SEC_CODE
#include "tpl_memmap.h"

/*
 * OSEK/VDX API services
 *
 * see header file documentation or refer to the OSEK/VDX 2.2.2 specification
 */
StatusType ActivateTask(const TaskType task_id)
{
    /*  init the error to no error  */
    StatusType result = E_OK;
    
    /*  lock the task structures    */
    LOCK_WHEN_TASK()
    
    /*  store information for error hook routine    */
    STORE_SERVICE(OSServiceId_ActivateTask)
    STORE_TASK_ID(task_id)
    
    /*  Check a task_id error   */
    CHECK_TASK_ID_ERROR(task_id,result)
    
#ifndef NO_TASK
    IF_NO_EXTENDED_ERROR(result)
        result = tpl_activate_task(tpl_task_table[task_id]);
        if (result == (tpl_status)E_OK_AND_SCHEDULE)
        {
            tpl_schedule(FROM_TASK_LEVEL);
            result &= OSEK_STATUS_MASK;
        }
    IF_NO_EXTENDED_ERROR_END()
#endif
    
    PROCESS_ERROR(result)

    /*  unlock the task structures  */
    UNLOCK_WHEN_TASK()
    
    return result;
}


StatusType TerminateTask(void)
{
    /*  init the error to no error  */
    StatusType result = E_OK;

    /*  lock the task structures    */
    LOCK_WHEN_TASK()
    
    /*  store information for error hook routine    */
    STORE_SERVICE(OSServiceId_TerminateTask)
    
    /*  check we are at the task level  */
    CHECK_TASK_CALL_LEVEL_ERROR(result)
    /*  check the task does not own a resource  */
    CHECK_RUNNING_OWNS_REZ_ERROR(result)

#ifndef NO_TASK
    IF_NO_EXTENDED_ERROR(result)
    
        /*  set the state of the running task to DYING                  */
        tpl_running_obj->state = (tpl_exec_state)DYING;

        /*  and let the scheduler do its job                            */
        tpl_schedule(FROM_TASK_LEVEL);
        
    IF_NO_EXTENDED_ERROR_END()
#endif

    PROCESS_ERROR(result)
    
    /*  unlock the task structures  */
    UNLOCK_WHEN_TASK()

    return result;
}


StatusType ChainTask(const TaskType task_id)
{
    StatusType result = E_OK;

#ifndef NO_TASK
    tpl_exec_common *exec_obj;
#endif

    /*  lock the task system    */
    LOCK_WHEN_TASK()

    /*  store information for error hook routine    */
    STORE_SERVICE(OSServiceId_ChainTask)
    STORE_TASK_ID(task_id)
    
    /*  Check a call level error    */
    CHECK_TASK_CALL_LEVEL_ERROR(result)
    /*  Check a task_id error       */
    CHECK_TASK_ID_ERROR(task_id,result)
    /*  Check no resource is held by the terminating task   */
    CHECK_RUNNING_OWNS_REZ_ERROR(result)

#ifndef NO_TASK
    IF_NO_EXTENDED_ERROR(result)

        exec_obj = &((tpl_task_table[task_id])->exec_desc); 
        
        if (exec_obj == tpl_running_obj)
        {
            /*  The activated task and the currentily running object
                are the same. So the task is put in the RESURRECT state.    */
            tpl_running_obj->state = RESURRECT;

        }
        else
        {
            tpl_activate_counter    count = exec_obj->activate_count;

            if (count < exec_obj->static_desc->max_activate_count)
            {
                if (count == 0)
                {
                    /*  init the task       */
                    exec_obj->state = (tpl_exec_state)READY_AND_NEW;
                }
                /*  put it in the list  */
                tpl_put_exec_object(exec_obj, (u8)NEWLY_ACTIVATED_EXEC_OBJ);
                
                /*  inc the task activation count. When the task
                    will terminate it will dec this count and if
                    not zero it will be reactivated                         */
                exec_obj->activate_count++;
                
                /*  the object that is currently running is put in
                    the DYING state                                         */
                tpl_running_obj->state = DYING;
            }
            else
            {
                /*  The max activation count is reached. So an error occurs */
                result = E_OS_LIMIT;
            }
        }
        
        if (result == E_OK)
        {
            /*  and let the scheduler do its job                            */
            tpl_schedule(FROM_TASK_LEVEL);
        }
        
    IF_NO_EXTENDED_ERROR_END()
#endif
    
    PROCESS_ERROR(result)
    
    /*  unlock the task structures  */
    UNLOCK_WHEN_TASK()
    
    return result;
}


StatusType Schedule(void)
{
    StatusType result = E_OK;
    
    /*  lock the task system    */
    LOCK_WHEN_TASK()
    
    /*  store information for error hook routine    */
    STORE_SERVICE(OSServiceId_Schedule)

    /*  Check a call level error    */
    CHECK_TASK_CALL_LEVEL_ERROR(result)
    /*  Check no resource is held by the calling task   */
    CHECK_RUNNING_OWNS_REZ_ERROR(result)
    
#ifndef NO_TASK
    IF_NO_EXTENDED_ERROR(result)
        /*  release the internal resource   */
        tpl_release_internal_resource(tpl_running_obj);
        /*  does the rescheduling           */
        tpl_schedule(FROM_TASK_LEVEL);
    IF_NO_EXTENDED_ERROR_END()
#endif
    
    PROCESS_ERROR(result)

    /*  unlock the task structures  */
    UNLOCK_WHEN_TASK()
    
    return result;
}


StatusType GetTaskID(TaskRefType task_id)
{
    /*  get the task id from the task descriptor.
        note : the idle task has an id set to INVALID_TASK
        Done when the idle task is running, tpl_get_task_id
        returns naturally INVALID_TASK in task_id   */
    *task_id = tpl_running_obj->static_desc->id;
    
    return E_OK;
}


StatusType GetTaskState(
    const TaskType      task_id,
    TaskStateRefType    state)
{
    StatusType result = E_OK;
    
    LOCK_WHEN_HOOK()
    
    /*  store information for error hook routine    */
    STORE_SERVICE(OSServiceId_GetTaskState)
    STORE_TASK_STATE_REF(state)

    /*  Check a task_id error       */
    CHECK_TASK_ID_ERROR(task_id,result)
    
#ifndef NO_TASK
    IF_NO_EXTENDED_ERROR(result)
        /*  MISRA RULE 45 VIOLATION: the original pointer points to a struct
            that has the same beginning fields as the struct it is casted to
            This allow object oriented design and polymorphism.
        */
        *state = ((tpl_exec_common *)(tpl_task_table[task_id]))->state;
    IF_NO_EXTENDED_ERROR_END()
#endif

    PROCESS_ERROR(result)

    UNLOCK_WHEN_HOOK()
    
    return result;
}

#define OS_STOP_SEC_CODE
#include "tpl_memmap.h"

/* End of file tpl_os_task.c */