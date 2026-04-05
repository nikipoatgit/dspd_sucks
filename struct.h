//
// Created by nikipo on 4/4/26.
//

#ifndef STRUCT_H
#define STRUCT_H
#include <stdint.h>

#include "types.h"
typedef struct Passenger Passenger;
typedef struct Seat Seat;
typedef struct Coach Coach;
typedef struct Waitlist Waitlist;
typedef struct BerthList BerthList;
typedef struct Ticket Ticket;

struct Seat {
    uint8_t seat_no; // max seat no 64
    BerthType berth;
    uint8_t isBooked;
    Passenger* occupant;
    Seat* next;
};

struct Coach {
    uint16_t coach_no;
    CoachType type;
    uint8_t capacity;
    Seat* seatList;
    Coach* next;
};


struct Passenger {
    char name[50];
    char gender;
    uint8_t age;
    char dob[11];

    BerthType berth;

    uint16_t coach_no;
    uint8_t seat_no;

    uint32_t ticket_id;

    Passenger* next;     // for  ticket->passengerList
    Passenger* gnext;     //for  global passenger list
};


struct Waitlist {
    uint32_t ticket_id;
    uint8_t numPassengers;

    CoachType type;
    BerthList* berthList;

    Waitlist* next;
};


struct BerthList{
    BerthType berth;
    BerthList* next;
};


struct Ticket {
    uint32_t ticket_id;
    uint8_t numPassengers;

    Passenger* passengerList;

    Ticket* next;
};

#endif // STRUCT_H