//
// Created by nikipo on 4/4/26.
//

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
    COACH_1AC,
    COACH_2AC,
    COACH_3AC,
    COACH_SL,
    COACH_ENGINE,
    COACH_PANTRY
} CoachType;

typedef enum {
    BERTH_L,
    BERTH_M,
    BERTH_U,
    BERTH_SL,
    BERTH_SU
} BerthType;

// note here order matters
typedef enum {
    false,
    true
} boolean;

#endif // TYPES_H