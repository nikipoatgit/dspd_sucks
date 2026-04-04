//
// Created by nikipo on 4/4/26.
//

#include "test.h"
#include <stdio.h>
#include "seat.h"

const char* coachTypeToStr(const CoachType type) {
    switch (type) {
        case COACH_1AC: return "1AC";
        case COACH_2AC: return "2AC";
        case COACH_3AC: return "3AC";
        case COACH_SL: return "SL";
        case COACH_ENGINE: return "ENGINE";
        case COACH_PANTRY: return "PANTRY";
        default: return "UNKNOWN";
    }
}

const char* berthToStr(BerthType b) {
    switch (b) {
        case BERTH_L: return "L";
        case BERTH_M: return "M";
        case BERTH_U: return "U";
        case BERTH_SL: return "SL";
        case BERTH_SU: return "SU";
        default: return "?";
    }
}

void printSeats(const Seat* head) {
    while (head) {

        if (head->isBooked && head->occupant) {
            printf("%d(%s)[B:%s] ",
                   head->seat_no,
                   berthToStr(head->berth),
                   head->occupant->name);
        } else {
            printf("%d(%s)[F] ",
                   head->seat_no,
                   berthToStr(head->berth));
        }

        head = head->next;
    }
    printf("\n");
}

void printTrain(const Coach* train) {
    while (train) {
        printf("Coach: %s | No: %d | Capacity: %d\n",
               coachTypeToStr(train->type),
               train->coach_no,
               train->capacity);

        if (train->seatList) {
            printSeats(train->seatList);
        }

        printf("------------\n");
        train = train->next;
    }
}

BerthList* createPref(BerthType arr[], int n) {
    BerthList* head = NULL;
    BerthList* tail = NULL;

    for (int i = 0; i < n; i++) {
        BerthList* node = (BerthList*)malloc(sizeof(BerthList));
        node->berth = arr[i];
        node->next = NULL;

        if (!head) head = tail = node;
        else {
            tail->next = node;
            tail = node;
        }
    }
    return head;
}
