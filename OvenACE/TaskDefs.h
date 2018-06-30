/*
 * All Task OS Definitions.
 */
#ifndef __TASK_DEFS__H__
#define __TASK_DEFS__H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define DEBUGTASK_NAME       ("DBUG Led")
#define DEBUGTASK_STACK_SIZE (128)
#define DEBUGTASK_PRIORITY   (tskIDLE_PRIORITY + 1)

#define PIEZOTASK_NAME       ("PiezoBuz")
#define PIEZOTASK_STACK_SIZE (128)
#define PIEZOTASK_PRIORITY   (tskIDLE_PRIORITY + 1)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
