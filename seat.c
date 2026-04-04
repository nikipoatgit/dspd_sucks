//
// Created by nikipo on 4/4/26.
//

#include "seat.h"

#include <stdio.h>

#include "string.h"

Seat* createSeat(const uint8_t seat_no, const BerthType berth) {
    Seat* s = ( Seat*)malloc(sizeof(Seat));
    if (!s) return NULL;

    s->seat_no = seat_no;
    s->berth = berth;
    s->isBooked = false;
    s->occupant = NULL;
    s->next = NULL;

    return s;
}

Seat* generateSeats(const uint8_t capacity) {
    if (capacity == 0) return NULL;

    static const BerthType berth_map[8] = {
        BERTH_L, BERTH_M, BERTH_U,
        BERTH_L, BERTH_M, BERTH_U,
        BERTH_SL, BERTH_SU
    };

    Seat* head = createSeat(1, berth_map[0]);
    if (!head) return NULL;

    Seat* tail = head;

    uint8_t idx = 1;

    for (uint8_t i = 2; i <= capacity; i++) {

        Seat* newSeat = createSeat(i, berth_map[idx]);
        if (!newSeat) return head;

        tail->next = newSeat;
        tail = newSeat;

        idx++;
        if (idx == 8) idx = 0;
    }

    return head;
}

boolean canCoachSatisfy(Coach* coach, BerthList* prefs) {
    if (!coach || !coach->seatList) return false;

    int available[5] = {0};  // L, M, U, SL, SU

    const  Seat* curr = coach->seatList;

    while (curr) {
        if (!curr->isBooked) {
            available[curr->berth]++;
        }
        curr = curr->next;
    }

    if (!prefs) return false;

    while (prefs) {
        if (available[prefs->berth] <= 0) {
            return false;
        }
        available[prefs->berth]--;
        prefs = prefs->next;
    }

    return true;
}

int allocateSeatsInCoach(Coach* restrict coach,
                             BerthList* restrict prefs,
                             Ticket* restrict ticket,
                             const char names[][50],
                             const char genders[],
                             const uint8_t ages[],
                             const char dobs[][11]) {

    if (!coach || !prefs || !ticket) return 0;

    Passenger* tail = ticket->passengerList;
    // todo insert O(1) operation
    if (tail) {
        while (tail->next) tail = tail->next;
    }

    int count = 0;

    while (prefs) {

        Seat* s = findAvailableSeat(coach, prefs->berth);
        if (!s) break;

        Passenger* p = malloc(sizeof(Passenger));
        if (!p) break;

        memcpy(p->name, names[count], 50);
        memcpy(p->dob,  dobs[count], 11);

        p->gender = genders[count];
        p->age    = ages[count];

        // Direct assignments
        p->berth    = s->berth;
        p->coach_no = coach->coach_no;
        p->seat_no  = s->seat_no;
        p->ticket_id = ticket->ticket_id;

        p->next = NULL;

        // Mark seat AFTER allocation success (safer ordering)
        s->isBooked = true;
        s->occupant = p;

        // O(1) append
        if (!ticket->passengerList) {
            ticket->passengerList = p;
            tail = p;
        } else {
            tail->next = p;
            tail = p;
        }

        count++;
        prefs = prefs->next;
    }

    return count;
}

Seat* findAvailableSeat(Coach* coach, BerthType berth) {
    if (!coach || !coach->seatList) return NULL;

    Seat* curr = coach->seatList;

    while (curr) {
        if (curr->isBooked == false && curr->berth == berth) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}