#include <stdio.h>
#include <stdint.h>
#include "booking.h"
#include "coach.h"
#include "passenger.h"
#include "test.h"

static uint32_t gNextTicketId = 1;

static Ticket* issueTicket(void) {
    Ticket* t = newTicket(gNextTicketId++);
    appendTicket(t);
    return t;
}

static void separator(const char* title) {
    printf("\n ----------------\n");
    printf("  %s\n", title);
    printf(" ----------------\n");
}

int main(void) {
    printf(" RAILWAY RESERVATION SYSTEM \n");

    Coach* train = initCoaches();

    /* ── BOOKING 1: 3 pax, 2AC, same coach (L M SL) ─────── */
    separator("Booking 1: 3 pax in 2AC [L, M, SL] — same-coach path");
    {
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_L, BERTH_M, BERTH_SL};
        BerthList* prefs = createPref(ba, 3);
        const char names[3][50] = {"Alice", "Bob", "Charlie"};
        const char genders[3]   = {'F', 'M', 'M'};
        const uint8_t ages[3]   = {28, 34, 65};
        const char dobs[3][11]  = {"15/06/1997","20/03/1991","10/01/1960"};

        int n = bookSeats(train, COACH_2AC, prefs, t, names, genders, ages, dobs);

        printf("  Booked: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ── BOOKING 2: 2 pax, 3AC, U berths ────────────────── */
    separator("Booking 2: 2 pax in 3AC [U, U]");
    {
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_U, BERTH_U};
        BerthList* prefs = createPref(ba, 2);
        const char names[2][50] = {"Diana", "Eve"};
        const char genders[2]   = {'F', 'F'};
        const uint8_t ages[2]   = {45, 72};
        const char dobs[2][11]  = {"05/09/1980","22/11/1953"};

        int n = bookSeats(train, COACH_3AC, prefs, t, names, genders, ages, dobs);

        printf("  Booked: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ── BOOKING 3: 2 pax, SL, lower berths ─────────────── */
    separator("Booking 3: 2 pax in SL [L, SL] — includes senior citizen");
    {
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_L, BERTH_SL};
        BerthList* prefs = createPref(ba, 2);
        const char names[2][50] = {"Frank", "Grace"};
        const char genders[2]   = {'M', 'F'};
        const uint8_t ages[2]   = {62, 55};
        const char dobs[2][11]  = {"01/01/1963","14/07/1970"};
        int n = bookSeats(train, COACH_SL, prefs, t, names, genders, ages, dobs);
        printf("  Booked: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ── BOOKING 4: 1 pax, 1AC, M berth ─────────────────── */
    separator("Booking 4: 1 pax in 1AC [M]");
    {
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_M};
        BerthList* prefs = createPref(ba, 1);
        const char names[1][50] = {"Hank"};
        const char genders[1]   = {'M'};
        const uint8_t ages[1]   = {40};
        const char dobs[1][11]  = {"30/08/1985"};
        int n = bookSeats(train, COACH_1AC, prefs, t, names, genders, ages, dobs);
        printf("  Booked: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ── BOOKING 5: 3AC, senior on upper berth ───────────── */
    separator("Booking 5: 3AC [U, SU] — senior Eve gets upper");
    {
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_U, BERTH_SU};
        BerthList* prefs = createPref(ba, 2);
        const char names[2][50] = {"Ivan", "Judy"};
        const char genders[2]   = {'M', 'F'};
        const uint8_t ages[2]   = {68, 29};
        const char dobs[2][11]  = {"03/03/1957","11/11/1996"};
        int n = bookSeats(train, COACH_3AC, prefs, t, names, genders, ages, dobs);
        printf("  Booked: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ══ DISPLAY FUNCTIONS ═════════════════════════════════ */
    displayAllByName();
    displayAllByCoach();
    displayLowerBerth();
    displaySeniorNotLower();
    displayAvailableSeats(train);

    /* ══ SORT: passengers in coach 2AC by name ════════════ */
    separator("Coach 2AC passengers — sorted by name");
    {
        Passenger* sorted = sortByNameInCoach(NULL, 101);
        Passenger* p = sorted;
        while (p) {
            printf("  %-20s | Seat:%2d | Berth:%s\n",
                   p->name, p->seat_no,
                   p->berth==BERTH_L?"L": p->berth==BERTH_M?"M":
                   p->berth==BERTH_U?"U": p->berth==BERTH_SL?"SL":"SU");
            p = p->gnext;
        }
        restoreGlobalAfterCoachSort();  /* repair gnext chain */
    }

    /* ══ CANCELLATION: partial — remove Bob from ticket 1 ═ */
    separator("Partial cancellation: remove Bob (seat 2) from ticket #1");
    {
        uint8_t seats[] = {2};
        int n = cancelReservation(train, 1, seats, 1);
        printf("  Passengers removed: %d\n", n);
    }

    /* ══ CANCELLATION: entire ticket 3 ════════════════════ */
    separator("Full cancellation: cancel entire ticket #3");
    {
        int n = cancelReservation(train, 3, NULL, 0);
        printf("  Passengers removed: %d\n", n);
    }

    displayAllByCoach();
    displayAvailableSeats(train);

    /* ══ WAITLIST: fill 1AC completely then try to book ═══ */
    separator("Waitlist demo: fill all 1AC seats, then attempt booking");
    {
        Coach* c = train;
        while (c && c->type != COACH_1AC) c = c->next;
        if (c) {
            Seat* s = c->seatList;
            while (s) { if (!s->isBooked) s->isBooked = 1; s = s->next; }
            printf("  (All 1AC seats manually filled)\n");
        }
        Ticket* t = issueTicket();
        BerthType ba[] = {BERTH_L};
        BerthList* prefs = createPref(ba, 1);
        const char names[1][50] = {"Zara"};
        const char genders[1]   = {'F'};
        const uint8_t ages[1]   = {27};
        const char dobs[1][11]  = {"15/05/1999"};
        int n = bookSeats(train, COACH_1AC, prefs, t, names, genders, ages, dobs);
        printf("  Seats booked immediately: %d  (Ticket #%u)\n", n, t->ticket_id);
    }

    /* ══ REVERSE COACH ORDER ══════════════════════════════ */
    separator("Reversing coach order");
    reverseTrainOrder(&train);

    printf("\n══ Train after reversal ════════════════════════════════\n");
    printf("  Expected: ENGINE → SL → 3AC → 2AC → PANTRY → 1AC\n\n");
    {
        Coach* c = train;
        while (c) {
            printf("  [%s #%d]\n",
                   c->type==COACH_ENGINE?"ENGINE":
                   c->type==COACH_1AC?"1AC":
                   c->type==COACH_2AC?"2AC":
                   c->type==COACH_3AC?"3AC":
                   c->type==COACH_SL?"SL":"PANTRY",
                   c->coach_no);
            c = c->next;
        }
    }

    printf("\n══ Full train seat map ═════════════════════════════════\n");
    printTrain(train);

    printf("\n[DONE]\n");
    return 0;
}
