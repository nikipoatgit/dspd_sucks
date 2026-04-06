//
// Created by nikipo on 4/5/26.
//

#include "passenger.h"
#include "booking.h"
#include "seat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Passenger* gPassengerList = NULL;
Passenger* gPassengerTail = NULL;
Ticket*    gTicketList    = NULL;
Waitlist*  gWaitlist      = NULL;

static uint32_t gNextWaitlistNo = 1;

/* ================= GLOBAL PASSENGER ================= */

void appendPassengerGlobal(Passenger* p) {
    if (!p) return;

    p->gnext = NULL;

    if (!gPassengerList) {
        gPassengerList = gPassengerTail = p;
    } else {
        gPassengerTail->gnext = p;
        gPassengerTail = p;
    }
}

void removePassengerGlobal(Passenger* p) {
    if (!gPassengerList || !p) return;

    if (gPassengerList == p) {
        gPassengerList = p->gnext;
        if (gPassengerTail == p) gPassengerTail = NULL;
        return;
    }

    Passenger* prev = gPassengerList;
    while (prev->gnext && prev->gnext != p)
        prev = prev->gnext;

    if (prev->gnext == p) {
        prev->gnext = p->gnext;
        if (gPassengerTail == p) gPassengerTail = prev;
    }
}

/* ================= TICKET ================= */

Ticket* newTicket(uint32_t ticket_id) {
    Ticket* t = calloc(1, sizeof(Ticket));
    if (!t) return NULL;
    t->ticket_id = ticket_id;
    return t;
}

Ticket* findTicket(uint32_t ticket_id) {
    for (Ticket* t = gTicketList; t; t = t->next)
        if (t->ticket_id == ticket_id) return t;
    return NULL;
}

void appendTicket(Ticket* t) {
    if (!t) return;

    t->next = NULL;

    if (!gTicketList) {
        gTicketList = t;
        return;
    }

    Ticket* cur = gTicketList;
    while (cur->next) cur = cur->next;
    cur->next = t;
}

/* ================= WAITLIST ================= */

static BerthList* cloneBerthList(BerthList* src) {
    BerthList *head = NULL, *tail = NULL;

    while (src) {
        BerthList* n = malloc(sizeof(BerthList));
        if (!n) break;

        *n = (BerthList){ .berth = src->berth, .next = NULL };

        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }

        src = src->next;
    }
    return head;
}

uint32_t addToWaitlist(CoachType type, BerthList* prefs,
                       uint32_t ticket_id, uint8_t numPassengers) {

    Waitlist* w = malloc(sizeof(Waitlist));
    if (!w) return 0;

    *w = (Waitlist){
        .ticket_id = ticket_id,
        .numPassengers = numPassengers,
        .type = type,
        .berthList = cloneBerthList(prefs),
        .next = NULL
    };

    uint32_t wlNo = gNextWaitlistNo++;

    if (!gWaitlist) gWaitlist = w;
    else {
        Waitlist* t = gWaitlist;
        while (t->next) t = t->next;
        t->next = w;
    }

    printf("[WAITLIST] Ticket %u → WL %u\n", ticket_id, wlNo);
    return wlNo;
}

/* ================= MERGE SORT ================= */

static Passenger* split(Passenger* head) {
    Passenger *slow = head, *fast = head->gnext;

    while (fast && fast->gnext) {
        slow = slow->gnext;
        fast = fast->gnext->gnext;
    }

    Passenger* mid = slow->gnext;
    slow->gnext = NULL;
    return mid;
}

static Passenger* merge(Passenger* a, Passenger* b) {
    if (!a) return b;
    if (!b) return a;

    if (strcmp(a->name, b->name) <= 0) {
        a->gnext = merge(a->gnext, b);
        return a;
    } else {
        b->gnext = merge(a, b->gnext);
        return b;
    }
}

Passenger* sortAllByName(Passenger* head) {
    if (!head || !head->gnext) return head;

    Passenger* mid = split(head);

    Passenger* left  = sortAllByName(head);
    Passenger* right = sortAllByName(mid);

    return merge(left, right);
}

/* ================= SORT WRAPPERS ================= */

void sortGlobalByName(void) {
    gPassengerList = sortAllByName(gPassengerList);

    // rebuild tail (O(n), unavoidable)
    gPassengerTail = gPassengerList;
    while (gPassengerTail && gPassengerTail->gnext)
        gPassengerTail = gPassengerTail->gnext;
}

/* ================= DISPLAY ================= */

static const char* berthStr(BerthType b) {
    static const char* map[] = {"L","M","U","SL","SU"};
    return (b < 5) ? map[b] : "?";
}

static void printPassenger(const Passenger* p) {
    printf("  %-20s | %c | %3d | %s | C:%3d | S:%2d | %-2s | T:%u\n",
           p->name, p->gender, p->age, p->dob,
           p->coach_no, p->seat_no, berthStr(p->berth), p->ticket_id);
}

void displayAllByName(void) {
    printf("\n=== Passengers (Name Sorted) ===\n");

    sortGlobalByName();

    for (Passenger* p = gPassengerList; p; p = p->gnext)
        printPassenger(p);
}

/* ================= CANCEL ================= */

static Seat* findSeatByNo(Coach* train, uint16_t coach_no, uint8_t seat_no) {
    for (Coach* c = train; c; c = c->next)
        if (c->coach_no == coach_no)
            for (Seat* s = c->seatList; s; s = s->next)
                if (s->seat_no == seat_no) return s;

    return NULL;
}

int cancelReservation(Coach* train,
                      uint32_t ticket_id,
                      const uint8_t* seat_nos,
                      int count) {

    Ticket* ticket = findTicket(ticket_id);
    if (!ticket) return 0;

    int cancelled = 0;
    Passenger *prev = NULL, *p = ticket->passengerList;

    while (p) {
        int cancel = (!seat_nos || count == 0);

        for (int i = 0; !cancel && i < count; i++)
            if (p->seat_no == seat_nos[i]) cancel = 1;

        if (cancel) {
            Seat* s = findSeatByNo(train, p->coach_no, p->seat_no);
            if (s) { s->isBooked = 0; s->occupant = NULL; }

            Passenger* tmp = p;
            if (prev) prev->next = p->next;
            else ticket->passengerList = p->next;

            p = p->next;

            removePassengerGlobal(tmp);
            free(tmp);
            cancelled++;
        } else {
            prev = p;
            p = p->next;
        }
    }

    ticket->numPassengers -= cancelled;
    advanceWaitlist(train);
    return cancelled;
}


void displayAllByCoach(void) {
    printf("\n=== Passengers (Coach → Seat) ===\n");

    // reuse existing qsort version if needed OR skip sorting
    for (Passenger* p = gPassengerList; p; p = p->gnext)
        printf("%s | C:%d S:%d\n", p->name, p->coach_no, p->seat_no);
}

void displayLowerBerth(void) {
    printf("\n=== Lower Berth ===\n");

    for (Passenger* p = gPassengerList; p; p = p->gnext)
        if (p->berth == BERTH_L || p->berth == BERTH_SL)
            printf("%s | Age:%d | C:%d S:%d\n",
                   p->name, p->age, p->coach_no, p->seat_no);
}

void displaySeniorNotLower(void) {
    printf("\n=== Senior NOT Lower ===\n");

    for (Passenger* p = gPassengerList; p; p = p->gnext)
        if (p->age > 60 &&
            p->berth != BERTH_L && p->berth != BERTH_SL)
            printf("%s | Age:%d | C:%d S:%d\n",
                   p->name, p->age, p->coach_no, p->seat_no);
}

void displayAvailableSeats(Coach* train) {
    printf("\n=== Available Seats ===\n");

    for (Coach* c = train; c; c = c->next) {
        if (c->type == COACH_ENGINE || c->type == COACH_PANTRY)
            continue;

        int count = 0;
        for (Seat* s = c->seatList; s; s = s->next)
            if (!s->isBooked) count++;

        printf("Coach %d → %d free\n", c->coach_no, count);
    }
}


void reverseTrainOrder(Coach** train) {
    if (!train || !*train) return;

    Coach* engine = NULL;
    Coach* rest = NULL;

    for (Coach* c = *train; c;) {
        Coach* next = c->next;
        c->next = NULL;

        if (c->type == COACH_ENGINE) {
            engine = c;
        } else {
            c->next = rest;
            rest = c;
        }
        c = next;
    }

    if (engine) {
        engine->next = rest;
        *train = engine;
    } else {
        *train = rest;
    }
}

void advanceWaitlist(Coach* train) {
    if (!gWaitlist) return;

    Waitlist* w = gWaitlist;
    Ticket* t = findTicket(w->ticket_id);

    if (!t) {
        gWaitlist = w->next;
        free(w);
        return;
    }

    int booked = 0;
    for (Passenger* p = t->passengerList; p; p = p->next)
        booked++;

    int need = w->numPassengers - booked;
    if (need <= 0) {
        gWaitlist = w->next;
        free(w);
        return;
    }
}