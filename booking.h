//
// Created by nikipo on 4/4/26.
//

#ifndef BOOKING_H
#define BOOKING_H

#include "coach.h"

Seat* findSeatInAnyCoach(Coach* train, CoachType type,
                          BerthType berth, Coach** outCoach);


int bookSeats(Coach* train,
              CoachType type,
              BerthList* prefs,
              Ticket* ticket,
              const char names[][50],
              const char genders[],
              const uint8_t ages[],
              const char dobs[][11]);


#endif // BOOKING_H