/*
 * Sample program to illustrate a parallel algorithm manipulating a shared
 * array.
 *
 * 6 threads are created
 *
 *
 * The PPU performs the following tasks:
 *  - initialize a 4x6 array
 *  - create 6 threads. Each thread is assigned a different rank from 0 to 5.
 *    The threads also get the address of the array. They put themselves in
 *    a blocking waiting mode for a signal notification.
 *  - The PPU fills the array with the consecutive elements { 1, 2, .., 24 }.
 *  - The PPU sends a signal to the threads to unblock them.
 *  - Each thread reads a different vector of 4 integers from the array (using
 *    dma), multiplies all elements by 2 and sends the vector back to main
 *    storage.
 *
 * The original array contains the integers : { 1, 2, ... 24 } so the result
 * is the values { 2, 4, ..., 48 }.
 *
 * All 24 multiplications are done in parallel:
 *  - Each SPU perform a vector multiplication of the { 2, 2, 2, 2 } vector with
 *    the read value, hence 4 multiplications per SPU.
 *  - All 6 SPUs do the same job in parallel, hence 4*6 for all SPUs.
 */

#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <sys/spu.h>

#include "spu_bin.h"
#include "spustr.h"

#include "nclogger.h"
#include "tasks/sum.h"
#include "thespumanager.h"


int main(int argc, const char* argv[])
{
	// NCLogger log("192.168.40.201", 18194);
	NCLogger log("192.168.1.122", 18194);
	log.send("\nTEST!\nIf you see this you've set up NCLogger correctly.\n");

	TheSPUManager m(log);
	log.send("spuman\n");
	std::vector<Sum*> v(6);
	for (size_t i=0; i<v.size(); i++) {
		v[i] = (Sum*)memalign(16, sizeof(Sum));
		v[i]->init(i+1,i+1);
	}
	log.send("sum loop\n");
	m.run(v);
	log.send("run\n");

	for (size_t i=0; i<v.size(); i++) {
		Sum *s = v[i];
		log << "s.res: " << s->res << std::endl;
		log.send();
	}

	m.stop();

	for (size_t i=0; i<v.size(); i++) {
		free(v[i]);
	}

	// log.send("Joining SPU thread group... ");
	// printf("Joining SPU thread group... ");
	// printf("%08x\n", sysSpuThreadGroupJoin(group_id, &cause, &status));
	// log << "cause=" << cause << ", status=" << status << std::endl;
	// log.send();
	// printf("cause=%d status=%d\n", cause, status);

	// printf("Closing image... %08x\n", sysSpuImageClose(&image));

	log.send("Bye\n");
	return 0;
}
