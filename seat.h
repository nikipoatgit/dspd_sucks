//
// Created by nikipo on 4/4/26.
//

#ifndef SEAT_H
#define SEAT_H
#include "struct.h"
#include <stdint.h>
#include <stdlib.h>
#include "types.h"
#include "coach.h"

Seat* createSeat(uint8_t seat_no, BerthType berth);
Seat* generateSeats(uint8_t capacity);

boolean canCoachSatisfy(Coach* coach, BerthList* prefs);
Seat* findAvailableSeat(Coach* coach, BerthType berth);
int allocateSeatsInCoach(Coach* coach,
                         BerthList* prefs,
                         Ticket* ticket,
                         const char names[][50],
                         const char genders[],
                         const uint8_t ages[],
                         const char dobs[][11]);

#endif // SEAT_H