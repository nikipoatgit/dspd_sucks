//
// Created by nikipo on 4/4/26.
//

#ifndef COACH_H
#define COACH_H

#include <stdint.h>
#include "types.h"
#include "struct.h"

struct Coach* createCoach(uint16_t coach_no, CoachType type, uint8_t capacity);
void appendCoach(struct Coach** head, struct Coach* newCoach);
struct Coach* initCoaches();

#endif // COACH_H