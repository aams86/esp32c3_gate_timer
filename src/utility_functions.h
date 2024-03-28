#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H
#include "Arduino.h"

// Structure to hold running totals
typedef struct {
    int n; // Number of data points
    double sum_x; // Sum of all data points
    double sum_x_squared; // Sum of the squares of all data points
    double mean;
    double sd;
    uint8_t max;
    uint8_t min;
} real_time_sd_calc;

void init_sd(real_time_sd_calc* stats);
// Add a new data point
void addDataPoint(real_time_sd_calc* stats, uint8_t x);
// Calculate the standard deviation
void calculateStdDev(real_time_sd_calc* stats);

int64_t differenceMicroseconds(int64_t start, int64_t end);
// Calculate the difference in ms
float differenceMilliseconds(int64_t start, int64_t end);

// Calculate the difference in ms
float differenceSeconds(int64_t start, int64_t end);

float microSecondsToSeconds(int64_t microseconds);

#endif