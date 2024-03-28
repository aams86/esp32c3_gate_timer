#include "utility_functions.h"
#include "Arduino.h"

// Calculate the difference in microseconds
int64_t differenceMicroseconds(int64_t start, int64_t end) {
  return end - start;
}

// Calculate the difference in ms
float differenceMilliseconds(int64_t start, int64_t end) {
  return (float)differenceMicroseconds(start, end) / 1000.0;
}

// Calculate the difference in ms
float differenceSeconds(int64_t start, int64_t end) {
  return differenceMilliseconds(start, end) / 1000.0;
}

float microSecondsToSeconds(int64_t microseconds) {
    return (float)microseconds / 1000.0 / 1000.0;
}


#include <stdio.h>
#include <math.h> // For sqrt function


// Initialize the stats
void init_sd(real_time_sd_calc* stats) {
    stats->n = 0;
    stats->sum_x = 0;
    stats->sum_x_squared = 0;
    stats->mean = 0;
    stats->sd = 0;
    stats->max = 0;
    stats->min = 255;
}

// Add a new data point
void addDataPoint(real_time_sd_calc* stats, uint8_t x) {
    stats->n += 1;
    stats->sum_x += x;
    stats->sum_x_squared += x * x;
    if(x > stats->max) {
        stats->max = x;
    } else if(x < stats->min) {
        stats->min = x;
    }
}

// Calculate the standard deviation
void calculateStdDev(real_time_sd_calc* stats) {
    if (stats->n == 0) {
        return;
    }
    double mean = stats->sum_x / stats->n;
    double variance = (stats->sum_x_squared / stats->n) - (mean * mean);
    stats->mean = mean;
    stats->sd = sqrt(variance);
}
