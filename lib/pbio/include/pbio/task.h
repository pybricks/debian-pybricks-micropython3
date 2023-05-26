// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2023 The Pybricks Authors

/**
 * @addtogroup Task pbio/task: Tasks
 *
 * Framework for scheduling asynchronous tasks.
 *
 * @{
 */
#ifndef _PBIO_TASK_H_
#define _PBIO_TASK_H_

#include <stdbool.h>

#include <contiki.h>

#include <pbio/error.h>

/** Task data structure. */
typedef struct _pbio_task_t pbio_task_t;

/** Task protothread function. */
typedef PT_THREAD((*pbio_task_thread_t)(struct pt *pt, pbio_task_t *task));

/** Task data structure fields. */
struct _pbio_task_t {
    /** Linked list node (internal use). */
    pbio_task_t *next;
    /** Task protothread. */
    pbio_task_thread_t thread;
    /** Caller-defined context data structure. */
    void *context;
    /** Protothread state (internal use). */
    struct pt pt;
    /**
     * Task status. ::PBIO_ERROR_AGAIN indicates not done, ::PBIO_SUCCESS indicates
     * that the task completed successfully, ::PBIO_ERROR_CANCELED indicates that
     * task was canceled, other errors indicate that the task failed.
     */
    pbio_error_t status;
    /** Flag for requesting cancellation. */
    bool cancel;
};

void pbio_task_init(pbio_task_t *task, pbio_task_thread_t thread, void *context);
bool pbio_task_run_once(pbio_task_t *task);
void pbio_task_cancel(pbio_task_t *task);

#endif // _PBIO_TASK_H_

/** @} */
