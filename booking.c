//
// Created by nikipo on 4/4/26.
//

#include "booking.h"
#include "string.h"
#include "seat.h"

Seat* findSeatInAnyCoach(Coach* train, CoachType type, BerthType berth, Coach** outCoach) {
    Coach* curr = train;

    while (curr) {
        if (curr->type == type) {
            Seat* s = findAvailableSeat(curr, berth);
            if (s) {
                if (outCoach) *outCoach = curr;
                return s;
            }
        }
        curr = curr->next;
    }

    return NULL;
}

int bookSeats(Coach* train,
              const CoachType type,
              BerthList* prefs,
              Ticket* ticket,
              const char names[][50],
              const char genders[],
              const uint8_t ages[],
              const char dobs[][11]) {

    if (!train || !prefs || !ticket) return 0;

    int count = 0;

    // SAME COACH
    Coach* curr = train;
    while (curr) {
        if (curr->type == type) {
            if (canCoachSatisfy(curr, prefs)) {
                return allocateSeatsInCoach(curr, prefs, ticket,
                                            names, genders, ages, dobs);
            }
        }
        curr = curr->next;
    }

    // ---------- STEP 2: MULTI-COACH (STRICT PREFS) ----------
    BerthList* p = prefs;

    while (p) {
        Coach* assignedCoach = NULL;
        Seat* s = findSeatInAnyCoach(train, type, p->berth, &assignedCoach);

        if (!s) break;

        // allocate immediately
        s->isBooked = 1;

        Passenger* newP = (Passenger*)malloc(sizeof(Passenger));
        if (!newP) break;

        strcpy(newP->name, names[count]);
        newP->gender = genders[count];
        newP->age = ages[count];
        strcpy(newP->dob, dobs[count]);

        newP->berth = s->berth;
        newP->coach_no = assignedCoach->coach_no;
        newP->seat_no = s->seat_no;
        newP->ticket_id = ticket->ticket_id;
        newP->next = NULL;

        s->occupant = newP;

        // append to ticket
        if (!ticket->passengerList) {
            ticket->passengerList = newP;
        } else {
            Passenger* temp = ticket->passengerList;
            while (temp->next) temp = temp->next;
            temp->next = newP;
        }

        count++;
        p = p->next;
    }

    // full success
    if (!p) return count;

    // ---------- STEP 3: PARTIAL ALLOCATION ----------
    // continue for remaining prefs but ignore berth strictness

    while (p) {
        Coach* curr2 = train;
        Seat* s = NULL;
        Coach* assignedCoach = NULL;

        // find ANY free seat (ignore berth)
        while (curr2) {
            if (curr2->type == type) {
                Seat* tempSeat = curr2->seatList;
                while (tempSeat) {
                    if (!tempSeat->isBooked) {
                        s = tempSeat;
                        assignedCoach = curr2;
                        break;
                    }
                    tempSeat = tempSeat->next;
                }
            }
            if (s) break;
            curr2 = curr2->next;
        }

        if (!s) break;

        // allocate
        s->isBooked = 1;

        Passenger* newP = (Passenger*)malloc(sizeof(Passenger));
        if (!newP) break;

        strcpy(newP->name, names[count]);
        newP->gender = genders[count];
        newP->age = ages[count];
        strcpy(newP->dob, dobs[count]);

        newP->berth = s->berth;
        newP->coach_no = assignedCoach->coach_no;
        newP->seat_no = s->seat_no;
        newP->ticket_id = ticket->ticket_id;
        newP->next = NULL;

        s->occupant = newP;

        if (!ticket->passengerList) {
            ticket->passengerList = newP;
        } else {
            Passenger* temp = ticket->passengerList;
            while (temp->next) temp = temp->next;
            temp->next = newP;
        }

        count++;
        p = p->next;
    }

    if (count > 0) return count;

    // ---------- STEP 4: WAITLIST ----------
    // (basic placeholder)

    // addToWaitlist(type, prefs, ticket_id, count);
    // implement later

    return 0;
}