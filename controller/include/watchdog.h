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

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <avr/wdt.h>
#include <stdint.h>

#define WDT_1SECOND 1000

void watchdog_init();
void watchdog_handler();
void watchdog_reboot();

#endif // WATCHDOG_H
