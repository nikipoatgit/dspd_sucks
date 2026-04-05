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
Ticket*    gTicketList    = NULL;
Waitlist*  gWaitlist      = NULL;

static uint32_t gNextWaitlistNo = 1;

void appendPassengerGlobal(Passenger* p) {
    if (!p) return;
    p->gnext = NULL;
    if (!gPassengerList) { gPassengerList = p; return; }
    Passenger* t = gPassengerList;
    while (t->gnext) t = t->gnext;
    t->gnext = p;
}

void removePassengerGlobal(Passenger* p) {
    if (!gPassengerList || !p) return;
    if (gPassengerList == p) { gPassengerList = p->gnext; return; }
    Passenger* prev = gPassengerList;
    while (prev->gnext && prev->gnext != p) prev = prev->gnext;
    if (prev->gnext == p) prev->gnext = p->gnext;
}



Ticket* newTicket(uint32_t ticket_id) {
    Ticket* t = calloc(1, sizeof(Ticket));
    if (!t) return NULL;
    t->ticket_id = ticket_id;
    return t;
}

Ticket* findTicket(uint32_t ticket_id) {
    Ticket* t = gTicketList;
    while (t) { if (t->ticket_id == ticket_id) return t; t = t->next; }
    return NULL;
}

void appendTicket(Ticket* t) {
    if (!t) return;
    t->next = NULL;
    if (!gTicketList) { gTicketList = t; return; }
    Ticket* cur = gTicketList;
    while (cur->next) cur = cur->next;
    cur->next = t;
}



static BerthList* cloneBerthList(BerthList* src) {
    BerthList* head = NULL, *tail = NULL;
    while (src) {
        BerthList* n = malloc(sizeof(BerthList));
        if (!n) break;
        n->berth = src->berth; n->next = NULL;
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
    w->ticket_id     = ticket_id;
    w->numPassengers = numPassengers;
    w->type          = type;
    w->berthList     = cloneBerthList(prefs);
    w->next          = NULL;

    uint32_t wlNo = gNextWaitlistNo++;

    if (!gWaitlist) { gWaitlist = w; }
    else {
        Waitlist* t = gWaitlist;
        while (t->next) t = t->next;
        t->next = w;
    }
    printf("[WAITLIST] Ticket %u placed on waitlist. Position: %u\n",
           ticket_id, wlNo);
    return wlNo;
}

void advanceWaitlist(Coach* train) {
    if (!gWaitlist) return;
    Waitlist* w = gWaitlist;
    Ticket* ticket = findTicket(w->ticket_id);
    if (!ticket) { gWaitlist = w->next; free(w); return; }

    int alreadyBooked = 0;
    Passenger* p = ticket->passengerList;
    while (p) { alreadyBooked++; p = p->next; }

    int need = (int)w->numPassengers - alreadyBooked;
    if (need <= 0) { gWaitlist = w->next; free(w); return; }

    BerthList* prefs = w->berthList;
    for (int i = 0; i < alreadyBooked && prefs; i++) prefs = prefs->next;
    if (!prefs) { gWaitlist = w->next; free(w); return; }

    const char placeholder[1][50] = {"WL_PASSENGER"};
    const char gender[1] = {'M'};
    const uint8_t age[1] = {0};
    const char dob[1][11] = {"00/00/0000"};

    int got = bookSeats(train, w->type, prefs, ticket,
                        placeholder, gender, age, dob);

    if (got >= need) {
        printf("[WAITLIST] Ticket %u confirmed from waitlist.\n", w->ticket_id);
        gWaitlist = w->next;
        free(w);
    } else {
        printf("[WAITLIST] Ticket %u still waiting (%d/%d seats).\n",
               w->ticket_id, alreadyBooked + got, w->numPassengers);
    }
}




static int globalCount(void) {
    int n = 0;
    Passenger* p = gPassengerList;
    while (p) { n++; p = p->gnext; }
    return n;
}

static Passenger** collectGlobal(int* out_n) {
    int n = globalCount();
    *out_n = n;
    if (n == 0) return NULL;
    Passenger** arr = malloc(n * sizeof(Passenger*));
    if (!arr) return NULL;
    Passenger* p = gPassengerList;
    for (int i = 0; i < n; i++) { arr[i] = p; p = p->gnext; }
    return arr;
}

static void relinkGlobal(Passenger** arr, int n) {
    gPassengerList = (n > 0) ? arr[0] : NULL;
    for (int i = 0; i < n; i++)
        arr[i]->gnext = (i + 1 < n) ? arr[i + 1] : NULL;
}

static int cmpCoachSeatQS(const void* a, const void* b) {
    const Passenger* pa = *(const Passenger**)a;
    const Passenger* pb = *(const Passenger**)b;
    if (pa->coach_no != pb->coach_no) return (int)pa->coach_no - (int)pb->coach_no;
    return (int)pa->seat_no - (int)pb->seat_no;
}

static int cmpNameQS(const void* a, const void* b) {
    const Passenger* pa = *(const Passenger**)a;
    const Passenger* pb = *(const Passenger**)b;
    return strcmp(pa->name, pb->name);
}

Passenger* sortByCoachAndSeat(Passenger* head) {
    (void)head; /* always sorts the global list */
    int n; Passenger** arr = collectGlobal(&n);
    if (!arr) return gPassengerList;
    qsort(arr, n, sizeof(Passenger*), cmpCoachSeatQS);
    relinkGlobal(arr, n);
    free(arr);
    return gPassengerList;
}

Passenger* sortAllByName(Passenger* head) {
    (void)head;
    int n; Passenger** arr = collectGlobal(&n);
    if (!arr) return gPassengerList;
    qsort(arr, n, sizeof(Passenger*), cmpNameQS);
    relinkGlobal(arr, n);
    free(arr);
    return gPassengerList;
}


Passenger* sortByNameInCoach(Passenger* head, uint16_t coach_no) {
    (void)head;
    int total = globalCount();
    if (total == 0) return NULL;
    Passenger** arr = malloc(total * sizeof(Passenger*));
    if (!arr) return NULL;
    int n = 0;
    Passenger* p = gPassengerList;
    while (p) {
        if (p->coach_no == coach_no) arr[n++] = p;
        p = p->gnext;
    }
    if (n == 0) { free(arr); return NULL; }
    qsort(arr, n, sizeof(Passenger*), cmpNameQS);
    for (int i = 0; i < n - 1; i++) arr[i]->gnext = arr[i + 1];
    arr[n - 1]->gnext = NULL;
    Passenger* result = arr[0];
    free(arr);
    // NOTE: this breaks the global gnext chain for these nodes.
    //   Restore the global chain after printing.
    return result;
}


static void restoreGlobalChain(void) {
    Passenger* tail = NULL;
    gPassengerList = NULL;
    Ticket* t = gTicketList;
    while (t) {
        Passenger* p = t->passengerList;
        while (p) {
            p->gnext = NULL;
            if (!gPassengerList) gPassengerList = p;
            else tail->gnext = p;
            tail = p;
            p = p->next;
        }
        t = t->next;
    }
}


static const char* berthStr(BerthType b) {
    switch (b) {
        case BERTH_L:  return "L";
        case BERTH_M:  return "M";
        case BERTH_U:  return "U";
        case BERTH_SL: return "SL";
        case BERTH_SU: return "SU";
        default:       return "?";
    }
}

static void printPassenger(const Passenger* p) {
    printf("  %-20s | %c | Age:%3d | %s | Coach:%3d | Seat:%2d | %-2s | Tkt#%u\n",
           p->name, p->gender, p->age, p->dob,
           p->coach_no, p->seat_no, berthStr(p->berth), p->ticket_id);
}

void displayAllByName(void) {
    printf("\n══ All Passengers — sorted by Name ═══════════════════════════\n");
    sortAllByName(NULL);
    Passenger* p = gPassengerList;
    if (!p) { printf("  (none)\n"); return; }
    while (p) { printPassenger(p); p = p->gnext; }
}

void displayAllByCoach(void) {
    printf("\n══ All Passengers — sorted by Coach → Seat ════════════════════\n");
    sortByCoachAndSeat(NULL);
    Passenger* p = gPassengerList;
    if (!p) { printf("  (none)\n"); return; }
    while (p) { printPassenger(p); p = p->gnext; }
}

void displayLowerBerth(void) {
    printf("\n══ Lower-berth passengers (L / SL) with Ages ══════════════════\n");
    Passenger* p = gPassengerList;
    int found = 0;
    while (p) {
        if (p->berth == BERTH_L || p->berth == BERTH_SL) {
            printf("  %-20s | Age:%3d | Berth:%-2s | Coach:%d | Seat:%d\n",
                   p->name, p->age, berthStr(p->berth), p->coach_no, p->seat_no);
            found = 1;
        }
        p = p->gnext;
    }
    if (!found) printf("  (none)\n");
}

void displaySeniorNotLower(void) {
    printf("\n══ Senior citizens (age > 60) NOT on lower berth ══════════════\n");
    Passenger* p = gPassengerList;
    int found = 0;
    while (p) {
        if (p->age > 60 && p->berth != BERTH_L && p->berth != BERTH_SL) {
            printf("  %-20s | Age:%3d | Berth:%-2s | Coach:%d | Seat:%d\n",
                   p->name, p->age, berthStr(p->berth), p->coach_no, p->seat_no);
            found = 1;
        }
        p = p->gnext;
    }
    if (!found) printf("  (none)\n");
}

static const char* coachTypeStr(CoachType t) {
    switch (t) {
        case COACH_1AC:    return "1AC";
        case COACH_2AC:    return "2AC";
        case COACH_3AC:    return "3AC";
        case COACH_SL:     return "SL";
        case COACH_ENGINE: return "ENGINE";
        case COACH_PANTRY: return "PANTRY";
        default:           return "?";
    }
}

void displayAvailableSeats(Coach* train) {
    printf("\n══ Available seats per coach ═══════════════════════════════════\n");
    Coach* c = train;
    while (c) {
        if (c->type == COACH_ENGINE || c->type == COACH_PANTRY) { c = c->next; continue; }
        int avail[5] = {0}, total = 0;
        Seat* s = c->seatList;
        while (s) {
            if (!s->isBooked) { avail[s->berth]++; total++; }
            s = s->next;
        }
        printf("  %s #%d: Free=%-3d  [L=%d  M=%d  U=%d  SL=%d  SU=%d]\n",
               coachTypeStr(c->type), c->coach_no, total,
               avail[BERTH_L], avail[BERTH_M], avail[BERTH_U],
               avail[BERTH_SL], avail[BERTH_SU]);
        c = c->next;
    }
}

/* ═══════════════════════════════════════════════════════════
   Reverse coach order  (ENGINE stays first)
   Original : ENGINE, 1AC, PANTRY, 2AC, 3AC, SL
   Result   : ENGINE, SL, 3AC, 2AC, PANTRY, 1AC
   ═══════════════════════════════════════════════════════════ */
void reverseTrainOrder(Coach** train) {
    if (!train || !*train) return;

    Coach* engineList = NULL, *engineTail = NULL;
    Coach* rest = NULL;

    Coach* c = *train;
    while (c) {
        Coach* nx = c->next; c->next = NULL;
        if (c->type == COACH_ENGINE) {
            if (!engineList) engineList = engineTail = c;
            else { engineTail->next = c; engineTail = c; }
        } else {
            c->next = rest; rest = c;   /* prepend = reverse */
        }
        c = nx;
    }

    if (engineTail) engineTail->next = rest;
    *train = engineList ? engineList : rest;
    printf("[TRAIN] Coach order reversed.\n");
}

static Seat* findSeatByNo(Coach* train, uint16_t coach_no, uint8_t seat_no) {
    Coach* c = train;
    while (c) {
        if (c->coach_no == coach_no) {
            Seat* s = c->seatList;
            while (s) { if (s->seat_no == seat_no) return s; s = s->next; }
        }
        c = c->next;
    }
    return NULL;
}

int cancelReservation(Coach* train,
                      uint32_t ticket_id,
                      const uint8_t* seat_nos,
                      int count) {
    Ticket* ticket = findTicket(ticket_id);
    if (!ticket) {
        printf("[CANCEL] Ticket %u not found.\n", ticket_id);
        return 0;
    }

    int cancelled = 0;
    Passenger* prev = NULL;
    Passenger* p = ticket->passengerList;

    while (p) {
        int shouldCancel = 0;
        if (!seat_nos || count == 0) {
            shouldCancel = 1;
        } else {
            for (int i = 0; i < count; i++)
                if (p->seat_no == seat_nos[i]) { shouldCancel = 1; break; }
        }

        if (shouldCancel) {
            Seat* s = findSeatByNo(train, p->coach_no, p->seat_no);
            if (s) { s->isBooked = 0; s->occupant = NULL; }

            Passenger* toFree = p;
            if (prev) prev->next = p->next;
            else ticket->passengerList = p->next;
            p = p->next;

            removePassengerGlobal(toFree);

            printf("[CANCEL] Removed '%s' from ticket %u (Coach %d, Seat %d).\n",
                   toFree->name, ticket_id, toFree->coach_no, toFree->seat_no);
            free(toFree);
            cancelled++;
        } else {
            prev = p;
            p = p->next;
        }
    }

    ticket->numPassengers = (uint8_t)(ticket->numPassengers - cancelled);
    advanceWaitlist(train);
    return cancelled;
}

void restoreGlobalAfterCoachSort(void) {
    restoreGlobalChain();
}