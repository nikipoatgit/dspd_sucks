#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "booking.h"
#include "coach.h"
#include "passenger.h"
#include "test.h"

#define MAX_PASSENGERS 50
#define NAME_LEN       50
#define DOB_LEN        11

static uint32_t ticketCounter = 1;


static void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


static int readInt(const char *prompt, int lo, int hi) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            flush_stdin();
            if (value >= lo && value <= hi)
                return value;
            printf("  Please enter a number between %d and %d.\n", lo, hi);
        } else {
            flush_stdin();
            printf("  Invalid input – numbers only.\n");
        }
    }
}


static void readString(const char *prompt, char *buf, int maxLen) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, maxLen, stdin)) {
            /* EOF or error – clear and retry */
            clearerr(stdin);
            printf("  Read error, try again.\n");
            continue;
        }
        /* strip trailing newline / carriage-return */
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
            buf[--len] = '\0';

        /* check the string is not all whitespace */
        int nonSpace = 0;
        for (size_t i = 0; i < len; i++)
            if (!isspace((unsigned char)buf[i])) { nonSpace = 1; break; }

        if (len == 0 || !nonSpace) {
            printf("  Input cannot be empty.\n");
            continue;
        }
        /* check for overly-long input (fgets truncates silently) */
        if (len == (size_t)(maxLen - 1) && buf[len-1] != '\n') {
            /* buffer was exactly filled – user may have typed more */
            flush_stdin();
            printf("  Input too long (max %d characters).\n", maxLen - 1);
            continue;
        }
        return;
    }
}

static char readChar(const char *prompt, const char *allowedChars) {
    char buf[64];
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) {
            clearerr(stdin);
            printf("  Read error, try again.\n");
            continue;
        }
        /* strip whitespace */
        size_t len = strlen(buf);
        while (len > 0 && isspace((unsigned char)buf[len-1]))
            buf[--len] = '\0';

        if (len != 1) {
            printf("  Please enter exactly one character (%s).\n", allowedChars);
            continue;
        }
        char c = (char)toupper((unsigned char)buf[0]);
        if (strchr(allowedChars, c)) return c;
        printf("  Invalid choice. Allowed: %s\n", allowedChars);
    }
}


static int isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int daysInMonth(int m, int y) {
    static const int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2) return isLeapYear(y) ? 29 : 28;
    return days[m];
}


static void readDOB(const char *prompt, char *buf) {
    while (1) {
        readString(prompt, buf, DOB_LEN + 8 /* generous for over-long input */);

        /* check exact format DD/MM/YYYY */
        if (strlen(buf) != 10 ||
            !isdigit((unsigned char)buf[0]) || !isdigit((unsigned char)buf[1]) ||
            buf[2] != '/' ||
            !isdigit((unsigned char)buf[3]) || !isdigit((unsigned char)buf[4]) ||
            buf[5] != '/' ||
            !isdigit((unsigned char)buf[6]) || !isdigit((unsigned char)buf[7]) ||
            !isdigit((unsigned char)buf[8]) || !isdigit((unsigned char)buf[9])) {
            printf("  Invalid format. Use DD/MM/YYYY (e.g. 15/08/1990).\n");
            continue;
        }

        int dd = (buf[0]-'0')*10 + (buf[1]-'0');
        int mm = (buf[3]-'0')*10 + (buf[4]-'0');
        int yyyy = (buf[6]-'0')*1000 + (buf[7]-'0')*100 +
                   (buf[8]-'0')*10  + (buf[9]-'0');

        if (mm < 1 || mm > 12) {
            printf("  Month must be between 01 and 12.\n"); continue;
        }
        if (dd < 1 || dd > daysInMonth(mm, yyyy)) {
            printf("  Day %02d is invalid for month %02d/%04d.\n", dd, mm, yyyy);
            continue;
        }
        if (yyyy < 1900 || yyyy > 2026) {
            printf("  Year must be between 1900 and 2026.\n"); continue;
        }
        return; /* valid */
    }
}


static CoachType takeCoachInput(void) {
    int ch = readInt(
        "\nChoose Coach Type:\n"
        "  1. 1AC\n  2. 2AC\n  3. 3AC\n  4. Sleeper\n"
        "Enter choice [1-4]: ",
        1, 4);
    switch (ch) {
        case 1: return COACH_1AC;
        case 2: return COACH_2AC;
        case 3: return COACH_3AC;
        default: return COACH_SL;
    }
}

static BerthType takeBerthInput(void) {
    int b = readInt(
        "  Preferred berth (1-Lower, 2-Middle, 3-Upper, 4-SideLower, 5-SideUpper): ",
        1, 5);
    switch (b) {
        case 1: return BERTH_L;
        case 2: return BERTH_M;
        case 3: return BERTH_U;
        case 4: return BERTH_SL;
        default: return BERTH_SU;
    }
}


static Ticket* createNewTicket(void) {
    Ticket* t = newTicket(ticketCounter++);
    appendTicket(t);
    return t;
}


static void bookFlow(Coach* train) {
    int n = readInt("\nEnter number of passengers: ",1, MAX_PASSENGERS);

    CoachType type = takeCoachInput();

    /* VLAs with validated bound are safe here */
    char     names[n][NAME_LEN];
    char     genders[n];
    uint8_t  ages[n];
    char     dobs[n][DOB_LEN];
    BerthType pref[n];

    for (int i = 0; i < n; i++) {
        printf("\n--- Passenger %d of %d ---\n", i + 1, n);

        readString("  Full name: ", names[i], NAME_LEN);

        genders[i] = readChar("  Gender (M/F/O): ", "MFO");

        ages[i] = (uint8_t)readInt("  Age [1-120]: ", 1, 120);

        readDOB("  Date of birth (DD/MM/YYYY): ", dobs[i]);

        pref[i] = takeBerthInput();
    }

    Ticket*   t    = createNewTicket();
    BerthList* list = createPref(pref, n);
    int booked = bookSeats(train, type, list, t,
                           names, genders, ages, dobs);

    printf("\nSeats booked  : %d\n", booked);
    printf("Ticket ID     : %u\n", t->ticket_id);
}

static void cancelFlow(Coach* train) {
    uint32_t id = (uint32_t)readInt("\nEnter Ticket ID: ", 1, (int)ticketCounter - 1);
    int removed = cancelReservation(train, id, NULL, 0);
    printf("Passengers removed: %d\n", removed);
}

static void displayFlow(Coach* train) {
    int ch = readInt(
        "\nDisplay Menu\n"
        "  1. All passengers (by name)\n"
        "  2. All passengers (by coach)\n"
        "  3. Lower-berth passengers\n"
        "  4. Senior citizens not in lower berth\n"
        "  5. Available seats\n"
        "  6. Train map\n"
        "Choice [1-6]: ",
        1, 6);

    switch (ch) {
        case 1: displayAllByName();           break;
        case 2: displayAllByCoach();          break;
        case 3: displayLowerBerth();          break;
        case 4: displaySeniorNotLower();      break;
        case 5: displayAvailableSeats(train); break;
        case 6: printTrain(train);            break;
    }
}


int main(void) {
    Coach* train = initCoaches();

    printf("\n=== Railway Reservation System ===\n");

    while (1) {
        int choice = readInt(
            "\n-------------------------\n"
            "1. Book Ticket\n"
            "2. Cancel Ticket\n"
            "3. Display\n"
            "4. Reverse Train\n"
            "5. Exit\n"
            "-------------------------\n"
            "Enter choice [1-5]: ",
            1, 5);

        switch (choice) {
            case 1: bookFlow(train);                                  break;
            case 2: cancelFlow(train);                                break;
            case 3: displayFlow(train);                               break;
            case 4: reverseTrainOrder(&train);
                    printf("Train order reversed.\n");                break;
            case 5: printf("Exiting…\n"); return 0;
        }
    }
}