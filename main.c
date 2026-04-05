#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "booking.h"
#include "coach.h"
#include "passenger.h"
#include "test.h"

static uint32_t ticketCounter = 1;

/* create ticket */
Ticket* createNewTicket() {
    Ticket* t = newTicket(ticketCounter++);
    appendTicket(t);
    return t;
}

/* take coach input */
CoachType takeCoachInput() {
    int ch;
    printf("\nChoose Coach Type:\n");
    printf("1. 1AC\n2. 2AC\n3. 3AC\n4. Sleeper\n");
    printf("Enter choice: ");
    scanf("%d", &ch);

    if (ch == 1) return COACH_1AC;
    if (ch == 2) return COACH_2AC;
    if (ch == 3) return COACH_3AC;
    if (ch == 4) return COACH_SL;

    printf("Invalid choice, defaulting to 2AC\n");
    return COACH_2AC;
}

/* take berth input */
BerthType takeBerthInput() {
    int b;
    printf("Berth (1-L, 2-M, 3-U, 4-SL, 5-SU): ");
    scanf("%d", &b);

    if (b == 1) return BERTH_L;
    if (b == 2) return BERTH_M;
    if (b == 3) return BERTH_U;
    if (b == 4) return BERTH_SL;
    if (b == 5) return BERTH_SU;

    return BERTH_L;
}

/* booking */
void bookFlow(Coach* train) {
    int n;
    printf("\nEnter number of passengers: ");
    scanf("%d", &n);

    CoachType type = takeCoachInput();

    char names[n][50];
    char genders[n];
    uint8_t ages[n];
    char dobs[n][11];
    BerthType pref[n];

    for (int i = 0; i < n; i++) {
        printf("\nPassenger %d\n", i + 1);

        printf("Name: ");
        scanf("%s", names[i]);

        printf("Gender: ");
        scanf(" %c", &genders[i]);

        printf("Age: ");
        scanf("%hhu", &ages[i]);

        printf("DOB: ");
        scanf("%s", dobs[i]);

        pref[i] = takeBerthInput();
    }

    Ticket* t = createNewTicket();
    BerthList* list = createPref(pref, n);

    int booked = bookSeats(train, type, list, t,
                           names, genders, ages, dobs);

    printf("\nBooked seats: %d\n", booked);
    printf("Ticket ID: %u\n", t->ticket_id);
}

/* cancel */
void cancelFlow(Coach* train) {
    uint32_t id;
    printf("\nEnter Ticket ID: ");
    scanf("%u", &id);

    int removed = cancelReservation(train, id, NULL, 0);
    printf("Passengers removed: %d\n", removed);
}

/* display */
void displayFlow(Coach* train) {
    int ch;
    printf("\nDisplay Menu\n");
    printf("1. All passengers (name)\n");
    printf("2. All passengers (coach)\n");
    printf("3. Lower berth\n");
    printf("4. Senior citizens not in lower\n");
    printf("5. Available seats\n");
    printf("6. Train map\n");
    printf("Choice: ");
    scanf("%d", &ch);

    if (ch == 1) displayAllByName();
    else if (ch == 2) displayAllByCoach();
    else if (ch == 3) displayLowerBerth();
    else if (ch == 4) displaySeniorNotLower();
    else if (ch == 5) displayAvailableSeats(train);
    else if (ch == 6) printTrain(train);
    else printf("Invalid option\n");
}

/* main */
int main() {
    Coach* train = initCoaches();
    int choice;

    printf("\nRailway Reservation System\n");

    while (1) {
        printf("\n-------------------------\n");
        printf("1. Book Ticket\n");
        printf("2. Cancel Ticket\n");
        printf("3. Display\n");
        printf("4. Reverse Train\n");
        printf("5. Exit\n");
        printf("-------------------------\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            bookFlow(train);
        }
        else if (choice == 2) {
            cancelFlow(train);
        }
        else if (choice == 3) {
            displayFlow(train);
        }
        else if (choice == 4) {
            reverseTrainOrder(&train);
            printf("Train order reversed\n");
        }
        else if (choice == 5) {
            printf("Exiting...\n");
            break;
        }
        else {
            printf("Wrong choice, try again\n");
        }
    }

    return 0;
}
