#pragma once

typedef struct {
	uint64_t task_ea;	/* effective address of the task */
	uint32_t id;		/* spu thread id */
	uint32_t rank;		/* rank in SPU thread group (0..count-1) */
	uint32_t count;		/* number of threads in group */
	uint32_t sync;		/* sync value for response */
	uint32_t response;	/* response value */
	uint32_t taskType;
	uint32_t stop;		/* !0 for SPU to stop */
	uint32_t dummy[3];
} spustr_t;
