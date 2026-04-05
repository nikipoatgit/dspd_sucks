//
// Created by nikipo on 4/5/26.
//

#ifndef PASSENGER_H
#define PASSENGER_H

#include "struct.h"
#include "types.h"

extern Passenger* gPassengerList;   // all booked passengers, insertion order
extern Ticket*    gTicketList;      // all issued tickets
extern Waitlist*  gWaitlist;        // FIFO waitlist head


void   appendPassengerGlobal(Passenger* p);
void   removePassengerGlobal(Passenger* p);


Ticket*  newTicket(uint32_t ticket_id);
Ticket*  findTicket(uint32_t ticket_id);
void     appendTicket(Ticket* t);


uint32_t addToWaitlist(CoachType type, BerthList* prefs,
                       uint32_t ticket_id, uint8_t numPassengers);
void     advanceWaitlist(Coach* train);


Passenger* sortByCoachAndSeat(Passenger* head);
Passenger* sortByNameInCoach(Passenger* head, uint16_t coach_no);
Passenger* sortAllByName(Passenger* head);
void       restoreGlobalAfterCoachSort(void);


void displayAllByName(void);
void displayAllByCoach(void);
void displayLowerBerth(void);
void displaySeniorNotLower(void);
void displayAvailableSeats(Coach* train);


void reverseTrainOrder(Coach** train);


int cancelReservation(Coach* train,
                      uint32_t ticket_id,
                      const uint8_t* seat_nos,
                      int count);

#endif // PASSENGER_H