#include <stdio.h>
#include <stdlib.h>

// Define a structure to represent time
typedef struct {
    int hours;
    int minutes;
} TIME;

// Define a structure to represent a flight-plan
typedef struct FlightPlan {
    int flightID;
    TIME departureTime;
    TIME ETA;
    struct FlightPlan *next;
} FlightPlan;

// Define a structure to represent a bucket for the digital dashboard
typedef struct Bucket {
    int bucketID;
    TIME startETA;
    TIME endETA;
    FlightPlan *flightPlans; // Linked list of flight-plans sorted by departure time
    struct Bucket *next;
} Bucket;

// Function to calculate the time difference in minutes
int timediff(TIME A, TIME B) {
    return (A.hours * 60 + A.minutes) - (B.hours * 60 + B.minutes);
}

// Function to compare two times
int maxtime(TIME A, TIME B) {
    if (A.hours == B.hours && A.minutes == B.minutes) {
        return 0; // Times are the same
    } else if ((A.hours * 60 + A.minutes) > (B.hours * 60 + B.minutes)) {
        return 1; // A precedes B
    } else {
        return -1; // A succeeds B
    }
}

// Function to insert a new flight-plan in the digital dashboard
void insertFlightPlan(Bucket **dashboard, int bucketID, int flightID, TIME departureTime, TIME ETA) {
    Bucket *currentBucket = *dashboard;

    // Find the appropriate bucket or create a new one
    while (currentBucket != NULL) {
        if (currentBucket->bucketID == bucketID) {
            break;
        }
        currentBucket = currentBucket->next;
    }

    if (currentBucket == NULL) {
        // Create a new bucket if the specified bucket doesn't exist
        Bucket *newBucket = (Bucket *)malloc(sizeof(Bucket));
        if (newBucket != NULL) {
            newBucket->bucketID = bucketID;
            newBucket->startETA = ETA;
            newBucket->endETA = ETA;
            newBucket->flightPlans = NULL;
            newBucket->next = *dashboard;
            *dashboard = newBucket;
        }
        currentBucket = newBucket;
    }

    // Create a new flight-plan
    FlightPlan *newFlightPlan = (FlightPlan *)malloc(sizeof(FlightPlan));
    if (newFlightPlan != NULL) {
        newFlightPlan->flightID = flightID;
        newFlightPlan->departureTime = departureTime;
        newFlightPlan->ETA = ETA;
        newFlightPlan->next = NULL;

        // Insert the flight-plan into the linked list sorted by departure time
        FlightPlan *currentFlightPlan = currentBucket->flightPlans;
        FlightPlan *prevFlightPlan = NULL;

        while (currentFlightPlan != NULL && maxtime(currentFlightPlan->departureTime, departureTime) != 1) {
            prevFlightPlan = currentFlightPlan;
            currentFlightPlan = currentFlightPlan->next;
        }

        if (prevFlightPlan == NULL) {
            newFlightPlan->next = currentBucket->flightPlans;
            currentBucket->flightPlans = newFlightPlan;
        } else {
            prevFlightPlan->next = newFlightPlan;
            newFlightPlan->next = currentFlightPlan;
        }
    }
}

// Function to cancel a flight-plan in the digital dashboard
void cancelFlightPlan(Bucket **dashboard, int flightID) {
    Bucket *currentBucket = *dashboard;
    FlightPlan *prevFlightPlan = NULL;

    // Find the flight-plan to cancel
    while (currentBucket != NULL) {
        FlightPlan *currentFlightPlan = currentBucket->flightPlans;

        while (currentFlightPlan != NULL) {
            if (currentFlightPlan->flightID == flightID) {
                // Remove the flight-plan from the linked list
                if (prevFlightPlan == NULL) {
                    currentBucket->flightPlans = currentFlightPlan->next;
                } else {
                    prevFlightPlan->next = currentFlightPlan->next;
                }

                // Free the memory
                free(currentFlightPlan);

                return;
            }

            prevFlightPlan = currentFlightPlan;
            currentFlightPlan = currentFlightPlan->next;
        }

        currentBucket = currentBucket->next;
    }
}

// Function to show the status of a particular flight-plan
void showFlightPlanStatus(Bucket *dashboard, int flightID) {
    Bucket *currentBucket = dashboard;

    while (currentBucket != NULL) {
        FlightPlan *currentFlightPlan = currentBucket->flightPlans;

        while (currentFlightPlan != NULL) {
            if (currentFlightPlan->flightID == flightID) {
                // Display flight-plan details
                printf("Flight ID: %d\n", currentFlightPlan->flightID);
                printf("Departure Time: %02d:%02d\n", currentFlightPlan->departureTime.hours, currentFlightPlan->departureTime.minutes);
                printf("ETA: %02d:%02d\n", currentFlightPlan->ETA.hours, currentFlightPlan->ETA.minutes);

                return;  // Add this line to exit the function after displaying details
            }

            currentFlightPlan = currentFlightPlan->next;
        }

        currentBucket = currentBucket->next;
    }

    printf("Flight-plan with ID %d not found.\n", flightID);
}


// Function to show flight-plans within a 1-hour time period from a given time
void showFlightPlansInTimePeriod(Bucket *dashboard, TIME currentTime) {
    Bucket *currentBucket = dashboard;

    while (currentBucket != NULL) {
        if (timediff(currentBucket->startETA, currentTime) <= 0 && timediff(currentTime, currentBucket->endETA) <= 60) {
            // Display flight-plans within the 1-hour time period
            FlightPlan *currentFlightPlan = currentBucket->flightPlans;

            while (currentFlightPlan != NULL) {
                printf("Bucket ID: %d\n", currentBucket->bucketID);
                printf("Flight ID: %d\n", currentFlightPlan->flightID);
                printf("Departure Time: %02d:%02d\n", currentFlightPlan->departureTime.hours, currentFlightPlan->departureTime.minutes);
                printf("ETA: %02d:%02d\n", currentFlightPlan->ETA.hours, currentFlightPlan->ETA.minutes);
                printf("\n");

                currentFlightPlan = currentFlightPlan->next;
            }
        }

        currentBucket = currentBucket->next;
    }
}

// Function to free memory for flight-plans and buckets
void freeFlightPlansAndBuckets(Bucket *dashboard) {
    while (dashboard != NULL) {
        FlightPlan *currentFlightPlan = dashboard->flightPlans;
        while (currentFlightPlan != NULL) {
            FlightPlan *temp = currentFlightPlan;
            currentFlightPlan = currentFlightPlan->next;
            free(temp);
        }

        Bucket *tempBucket = dashboard;
        dashboard = dashboard->next;
        free(tempBucket);
    }
}

// Function to write flight data to a file
void writeFlightDataToFile(Bucket *dashboard, const char *filename) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    Bucket *currentBucket = dashboard;

    while (currentBucket != NULL) {
        FlightPlan *currentFlightPlan = currentBucket->flightPlans;

        while (currentFlightPlan != NULL) {
            fprintf(file, "%d %d %d %02d %02d %02d %02d %02d %02d %02d\n",
                    currentBucket->bucketID, currentFlightPlan->flightID,
                    currentFlightPlan->departureTime.hours, currentFlightPlan->departureTime.minutes,
                    currentFlightPlan->ETA.hours, currentFlightPlan->ETA.minutes,
                    currentBucket->startETA.hours, currentBucket->startETA.minutes,
                    currentBucket->endETA.hours, currentBucket->endETA.minutes);

            currentFlightPlan = currentFlightPlan->next;
        }

        currentBucket = currentBucket->next;
    }

    fclose(file);
}

// Function to read flight data from a file
Bucket *readFlightDataFromFile(const char *filename) {
    printf("Attempting to open file: %s\n", filename);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file for reading");
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    Bucket *dashboard = NULL;

    while (!feof(file)) {
        int bucketID, flightID;
        TIME departureTime, ETA, startETA, endETA;

        if (fscanf(file, "%d %d %d %d %d %d %d %d %d %d",
                   &bucketID, &flightID,
                   &departureTime.hours, &departureTime.minutes,
                   &ETA.hours, &ETA.minutes,
                   &startETA.hours, &startETA.minutes,
                   &endETA.hours, &endETA.minutes) == 10) {

            // Print the flight plan details for debugging
            printf("Read Flight Plan: Bucket ID: %d, Flight ID: %d, Departure Time: %02d:%02d, ETA: %02d:%02d\n",
                   bucketID, flightID, departureTime.hours, departureTime.minutes,
                   ETA.hours, ETA.minutes);

            insertFlightPlan(&dashboard, bucketID, flightID, departureTime, ETA);

            Bucket *currentBucket = dashboard;

            while (currentBucket != NULL) {
                if (currentBucket->bucketID == bucketID) {
                    if (timediff(currentBucket->startETA, startETA) > 0) {
                        currentBucket->startETA = startETA;
                    }

                    if (timediff(endETA, currentBucket->endETA) > 0) {
                        currentBucket->endETA = endETA;
                    }

                    break;
                }

                currentBucket = currentBucket->next;
            }
        }
    }

    // Check for any errors during reading
    if (!feof(file)) {
        perror("Error reading from file");
        fclose(file);
        freeFlightPlansAndBuckets(dashboard);
        return NULL;
    }

    fclose(file);
    return dashboard;
}

// Main function
int main() {
    // Initialize an empty digital dashboard
Bucket *dashboard = readFlightDataFromFile("C:/Users/Asus/Desktop/New folder/test.txt");

    int option, bucketID, flightID;
    TIME departureTime, ETA, currentTime;

    do {
        printf("\nMenu:\n");
        printf("1. Insert Flight Plan\n");
        printf("2. Cancel Flight Plan\n");
        printf("3. Show Flight Plan Status\n");
        printf("4. Show Flight Plans in 1-hour Time Period\n");
        printf("0. Exit\n");

        printf("Enter option: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                printf("Enter Bucket ID: ");
                scanf("%d", &bucketID);
                printf("Enter Flight ID: ");
                scanf("%d", &flightID);
                printf("Enter Departure Time (HH MM): ");
                scanf("%d %d", &departureTime.hours, &departureTime.minutes);
                printf("Enter ETA (HH MM): ");
                scanf("%d %d", &ETA.hours, &ETA.minutes);

                insertFlightPlan(&dashboard, bucketID, flightID, departureTime, ETA);
                break;

            case 2:
                printf("Enter Flight ID to Cancel: ");
                scanf("%d", &flightID);
                cancelFlightPlan(&dashboard, flightID);
                break;

            case 3:
                printf("Enter Flight ID to Show Status: ");
                scanf("%d", &flightID);
                showFlightPlanStatus(dashboard, flightID);
                break;

            case 4:
                printf("Enter Current Time (HH MM): ");
                scanf("%d %d", &currentTime.hours, &currentTime.minutes);
                showFlightPlansInTimePeriod(dashboard, currentTime);
                break;

            case 0:
                // Write flight data to file before exiting
                writeFlightDataToFile(dashboard, "C:/Users/Asus/Desktop/New folder/test.txt");
                // Free the memory for flight-plans and buckets before exiting
                freeFlightPlansAndBuckets(dashboard);
                break;

            default:
                printf("Invalid option. Please try again.\n");
        }

    } while (option != 0);

    return 0;
}
