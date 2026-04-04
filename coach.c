//
// Created by nikipo on 4/4/26.
//

#include "coach.h"
#include <stdlib.h>
#include "seat.h"

Coach* createCoach(const uint16_t coach_no, const CoachType type, const uint8_t capacity) {
    Coach* c = (Coach*)malloc(sizeof(Coach));
    if (!c) return NULL;

    c->coach_no = coach_no;
    c->type = type;
    c->capacity = capacity;
    c->next = NULL;

    if (type == COACH_ENGINE || type == COACH_PANTRY) {
        c->seatList = NULL;
    } else {
        c->seatList = generateSeats(capacity);
    }

    return c;
}

void appendCoach(Coach** head, Coach* newCoach) {
    if (!(*head)) {
        *head = newCoach;
        return;
    }

    Coach* temp = *head;
    while (temp->next) temp = temp->next;

    temp->next = newCoach;
}

Coach* initCoaches() {
    Coach* train = NULL;

    // Engine
    appendCoach(&train, createCoach(0, COACH_ENGINE, 0));

    // 1AC
    appendCoach(&train, createCoach(1, COACH_1AC, 16));

    // Pantry
    appendCoach(&train, createCoach(0, COACH_PANTRY, 0));

    // 2AC
    appendCoach(&train, createCoach(101, COACH_2AC, 48));

    // 3AC
    appendCoach(&train, createCoach(201, COACH_3AC, 64));

    // Sleeper
    appendCoach(&train, createCoach(301, COACH_SL, 72));

    return train;
}