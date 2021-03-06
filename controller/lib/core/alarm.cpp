/* Copyright 2020, Edwin Chiu

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "hal.h"

#include "alarm.h"

namespace {
struct alarm_stack_t {
  alarm_t alarm[ALARM_NODES];
  int8_t top;
};

alarm_stack_t stack;
} // anonymous namespace

static bool stack_full() {
  return (stack.top == (ALARM_NODES - 1)) ? true : false;
}

static bool stack_empty() { return (stack.top == -1) ? true : false; }

static int32_t stack_peek(alarm_t **alarm) {
  int32_t return_status = VC_STATUS_FAILURE;
  if (!stack_empty()) {
    *alarm = &stack.alarm[stack.top];
    return_status = VC_STATUS_SUCCESS;
  }
  return return_status;
}

static int32_t stack_pop(alarm_t **alarm) {
  int32_t return_status = VC_STATUS_FAILURE;
  if (!stack_empty()) {
    *alarm = &stack.alarm[stack.top];
    stack.top--;
    return_status = VC_STATUS_SUCCESS;
  }
  return return_status;
}

static void stack_push(alarm_t alarm) {
  if (!stack_full()) {
    stack.top++;
    stack.alarm[stack.top] = alarm;
  }
}

void alarm_init() { stack.top = -1; }

void alarm_add(enum dataID alarmID, char *data) {
  alarm_t alarm;
  if (!stack_full()) {
    // No point spending time doing these operations if the stack is full
    alarm.alarm = alarmID;
    alarm.timestamp = Hal.millis();

    // Copy alarm data
    for (uint8_t idx = 0; idx < ALARM_DATALEN; idx++) {
      alarm.data[idx] = *(data + idx);
    }

    stack_push(alarm);
  } else {
    // Stack full, ignore alarm
    // TODO Log this
  }
}

bool alarm_available() { return (stack_empty() == false) ? true : false; }

void alarm_remove() {
  alarm_t *alarm;
  stack_pop(&alarm); // Don't need this alarm anymore, remove it
}

int32_t alarm_read(enum dataID *alarmID, uint32_t *timestamp, char *data) {
  int32_t return_status = VC_STATUS_FAILURE;
  alarm_t *alarm;
  return_status = stack_peek(&alarm);

  if (return_status == VC_STATUS_SUCCESS) {
    *alarmID = (enum dataID)alarm->alarm;
    *timestamp = alarm->timestamp;

    for (uint8_t idx = 0; idx < ALARM_DATALEN; idx++) {
      *(data + idx) = alarm->data[idx];
    }
  }

  return return_status;
}
