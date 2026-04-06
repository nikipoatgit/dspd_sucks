//
// Created by nikipo on 4/4/26.
//

#include "booking.h"
#include "passenger.h"
#include "seat.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ── internal: find a seat of given type anywhere in train ── */
Seat* findSeatInAnyCoach(Coach* train, CoachType type,
                          BerthType berth, Coach** outCoach) {
    Coach* curr = train;
    while (curr) {
        if (curr->type == type) {
            Seat* s = findAvailableSeat(curr, berth);
            if (s) { if (outCoach) *outCoach = curr; return s; }
        }
        curr = curr->next;
    }
    return NULL;
}

/* ── internal: make a Passenger node ─────────────────────── */
static Passenger* makePassenger(const char* name, char gender,
                                uint8_t age, const char* dob,
                                BerthType berth, uint16_t coach_no,
                                uint8_t seat_no, uint32_t ticket_id) {
    Passenger* p = malloc(sizeof(Passenger));
    if (!p) return NULL;
    strncpy(p->name, name, 49); p->name[49] = '\0';
    strncpy(p->dob,  dob,  10); p->dob[10]  = '\0';
    p->gender    = gender;
    p->age       = age;
    p->berth     = berth;
    p->coach_no  = coach_no;
    p->seat_no   = seat_no;
    p->ticket_id = ticket_id;
    p->next      = NULL;
    return p;
}

/* ── internal: append passenger to ticket AND global list ── */
static void attachPassenger(Ticket* ticket, Passenger* p) {
    if (!ticket->passengerList) ticket->passengerList = p;
    else {
        Passenger* t = ticket->passengerList;
        while (t->next) t = t->next;
        t->next = p;
    }
    ticket->numPassengers++;
    appendPassengerGlobal(p);
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

    // same-coach
    {
        Coach* curr = train;
        while (curr) {
            //chk for coach satisfying
            if (curr->type == type && canCoachSatisfy(curr, prefs)) {
                int got = allocateSeatsInCoach(curr, prefs, ticket,
                                               names, genders, ages, dobs);

                Passenger* cur2 = ticket->passengerList;
                // int skip = count;
                // for (int i = 0; i < skip && cur2; i++) cur2 = cur2->next;
                while (cur2) { appendPassengerGlobal(cur2); cur2 = cur2->next; }
                return got;
            }
            curr = curr->next;
        }
    }

    // multi-coach
    {
        BerthList* p = prefs;
        while (p) {
            Coach* assignedCoach = NULL;
            Seat* s = findSeatInAnyCoach(train, type, p->berth, &assignedCoach);
            if (!s) break;

            s->isBooked = 1;
            Passenger* newP = makePassenger(names[count], genders[count],
                                            ages[count], dobs[count],
                                            s->berth, assignedCoach->coach_no,
                                            s->seat_no, ticket->ticket_id);
            if (!newP) break;
            s->occupant = newP;
            attachPassenger(ticket, newP);
            count++;
            p = p->next;
        }

        if (!p) return count;

        // ignore berth preference
        while (p) {
            Coach* curr2 = train;
            Seat* s = NULL;
            Coach* assignedCoach = NULL;

            while (curr2) {
                if (curr2->type == type) {
                    Seat* ts = curr2->seatList;
                    while (ts) {
                        if (!ts->isBooked) { s = ts; assignedCoach = curr2; break; }
                        ts = ts->next;
                    }
                }
                if (s) break;
                curr2 = curr2->next;
            }

            if (!s) break;

            s->isBooked = 1;
            Passenger* newP = makePassenger(names[count], genders[count],
                                            ages[count], dobs[count],
                                            s->berth, assignedCoach->coach_no,
                                            s->seat_no, ticket->ticket_id);
            if (!newP) break;
            s->occupant = newP;
            attachPassenger(ticket, newP);
            count++;
            p = p->next;
        }

        if (count > 0) return count;
    }

    // waitlist
    {
        int total = 0;
        BerthList* p = prefs;
        while (p) { total++; p = p->next; }
        addToWaitlist(type, prefs, ticket->ticket_id, (uint8_t)total);
    }

    return 0;
}