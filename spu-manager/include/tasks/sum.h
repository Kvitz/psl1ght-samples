#pragma once

#include "tasks/tasktypes.h"

class Sum {
public:

  int res;

  inline static TaskType getTaskType() {
    return SUM_TYPE;
  }

  Sum() {
  }

  void init(const int a, const int b) {
    this->a = a;
    this->b = b;
  }

  void run() {
    res = a + b;
  }

private:
  int a;
  int b;

  int dummy;
};
