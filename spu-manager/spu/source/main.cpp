#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <sys/spu_thread.h>

#define TAG 1

#include "spustr.h"
#include "tasks/sum.h"
#include "tasks/tasktypes.h"

extern void spu_thread_exit(uint32_t);

/* The effective address of the input structure */
uint64_t spu_ea;
/* A copy of the structure sent by ppu */
spustr_t spu __attribute__((aligned(16)));

uint64_t task_ea;
Sum sumObj __attribute__((aligned(16)));

/* wait for dma transfer to be finished */
static void wait_for_completion(void) {
	mfc_write_tag_mask(1<<TAG);
	spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

static void send_response(uint32_t x) {
	spu.response = x;
	spu.sync = 1;
	/* send response to ppu variable */
	uint64_t ea = spu_ea + ((uint32_t)&spu.response) - ((uint32_t)&spu);
	mfc_put(&spu.response, ea, 4, TAG, 0, 0);
	/* send sync to ppu variable with fence (this ensures sync is written AFTER response) */
	ea = spu_ea + ((uint32_t)&spu.sync) - ((uint32_t)&spu);
	mfc_putf(&spu.sync, ea, 4, TAG, 0, 0);
}

template <typename T>
void run(const uint64_t t_ea, T &t) {
	mfc_get(&t, t_ea, sizeof(t), TAG, 0, 0);
	wait_for_completion();

	t.run();
	uint64_t ea = t_ea + ((uint32_t)&t.res) - ((uint32_t)&t);
	mfc_put(&t.res, ea, sizeof(t.res), TAG, 0, 0);
}

int main(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {

	/* get spu data structure */
	spu_ea = arg1;

	while (true) {
		/* blocking wait for signal notification */
		spu_read_signal1();

		mfc_get(&spu, spu_ea, sizeof(spustr_t), TAG, 0, 0);
		wait_for_completion();

		switch (spu.taskType) {
			case SUM_TYPE:
			{
				task_ea = spu.task_ea;
				run(task_ea, sumObj);
				break;
			}
			case STOP_TYPE:
			default:
				spu.stop = 1;
		}

		if (spu.stop != 0) {
			send_response(1);
			wait_for_completion();
			break;
		}

		/* no wait for completion here, as it's done after response message,
		  and the sync element is send with a fence so we're sure our array
		  share is written back before sync */

		/* send the response message */
		send_response(1);
		wait_for_completion();
	}

	/* properly exit the thread */
	spu_thread_exit(0);
	return 0;
}
