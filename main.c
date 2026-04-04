#include <stdio.h>

#include "booking.h"
#include "coach.h"
#include "test.h"



int main(void) {
    Coach* train = initCoaches();

    // ticket
    Ticket ticket = {0};
    ticket.ticket_id = 1;
    ticket.passengerList = NULL;



    // preferences: L, M, SL
    BerthType prefsArr[] = {BERTH_L, BERTH_M, BERTH_SL};
    BerthList* prefs = createPref(prefsArr, 3);

    // passenger data
    const char names[3][50] = {"+++", "---", "###"};
    const char genders[3] = {'M', 'F', 'M'};
    const uint8_t ages[3] = {20, 22, 25};
    const char dobs[3][11] = {"01/01/2005", "02/02/2003", "03/03/2000"};

    int booked = bookSeats(train, COACH_2AC, prefs, &ticket, names, genders, ages, dobs);

    printf("Booked seats: %d\n", booked);



    // print passengers
    Passenger* p = ticket.passengerList;
    while (p) {
        printf("Name: %s | Coach: %d | Seat: %d | Berth: %d\n",
               p->name, p->coach_no, p->seat_no, p->berth);
        p = p->next;
    }


    printTrain(train);

    return 0;
}
