#pragma once

#include <memory>
#include <vector>

#include "spu_bin.h"
#include "spustr.h"

#include "nclogger.h"
#include "tasks/tasktypes.h"

#define ptr2ea(x) ((u64)(void *)(x))

class TheSPUManager {
public:
  TheSPUManager(NCLogger &log) : log(log) {
    spu = (spustr_t*)memalign(16, 6*sizeof(spustr_t));
    const std::string threadName("mythread");
    sysSpuThreadAttribute attr = { ptr2ea(threadName.c_str()), threadName.size()+1, SPU_THREAD_ATTR_NONE };
    const std::string groupName("mygroup");
    sysSpuThreadGroupAttribute grpattr = { groupName.size()+1, ptr2ea(groupName.c_str()), 0, 0 };

    sysSpuInitialize(6, 0);
    sysSpuImageImport(&image, spu_bin, 0);
    sysSpuThreadGroupCreate(&group_id, 6, 100, &grpattr);

    /* create 6 spu threads */
    for (int i = 0; i < 6; i++) {
      spu[i].rank = i;
      spu[i].count = 6;
      spu[i].sync = 0;
      spu[i].stop = 0;
      log << "&spu[i]: " << &spu[i] << std::endl;
      log.send();
      arg[i].arg0 = ptr2ea(&spu[i]);
      log << "arg[i].arg0: " << arg[i].arg0 << std::endl;
      log.send();

      sysSpuThreadInitialize(&spu[i].id, group_id, i, &image, &attr, &arg[i]);
      sysSpuThreadSetConfiguration(spu[i].id, SPU_SIGNAL1_OVERWRITE|SPU_SIGNAL2_OVERWRITE);
    }

    sysSpuThreadGroupStart(group_id);
    log.send("TheSPUManager ctor\n");
  }

  ~TheSPUManager() {
    sysSpuImageClose(&image);
    free(spu);
  }

  template <typename T>
  void run(const std::vector<T*> &v) {
    for (size_t i=0; i<6 && i<v.size(); i++) {
      T* p = v[i];
      spu[i].taskType = T::getTaskType();
      spu[i].task_ea = ptr2ea(p);
    }
    log.send("TheSPUManager run...\n");
    notifyAndWaitForAll();
    log.send("TheSPUManager run done\n");
  }

  void stop() {
    for (int i=0; i<6; i++) {
      spu[i].stop = 1;
      spu[i].taskType = STOP_TYPE;
    }
    notifyAndWaitForAll();

    u32 cause, status;
    sysSpuThreadGroupJoin(group_id, &cause, &status);
  }

private:
  spustr_t *spu;
  sysSpuImage image;
  sysSpuThreadArgument arg[6];
  u32 group_id;
  NCLogger &log;

  void notifyAndWaitForAll() {
    for (int i=0; i<6; i++) {
      sysSpuThreadWriteSignal(spu[i].id, 0, 1);
    }
    for (int i = 0; i < 6; i++)
  		while (spu[i].sync == 0);
  }

};
