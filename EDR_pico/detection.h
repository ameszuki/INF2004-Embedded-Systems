#include <stdio.h>
#include <string.h>

#define NUM_BASELINE_BUFFERS 5
#define BUFFER_SIZE 4

typedef struct {
    unsigned char data[BUFFER_SIZE];
    int is_full;
} BaselineBuffer;

BaselineBuffer baseline_buffers[NUM_BASELINE_BUFFERS];

void printBuffer(const unsigned char *buffer) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

int detectMemoryAnomalies(unsigned char *input_buffer) {
    int i, j;
    int step;

    // Check if any baseline buffer is empty and store the input buffer if found
    for (i = 0; i < NUM_BASELINE_BUFFERS; i++) {
        if (!baseline_buffers[i].is_full) {
            memcpy(baseline_buffers[i].data, input_buffer, BUFFER_SIZE);
            baseline_buffers[i].is_full = 1;
            printf("Stored to baseline data %d\n",i);
            return 0;  // Input buffer stored successfully
        }
    }

    /*
      STATIC BYTES check.
    */
    // Analyze baseline buffers to identify static and changing byte positions
    int static_bytes[BUFFER_SIZE] = {0};  // 0: static, 1: changing
    for (i = 0; i < BUFFER_SIZE; i++) {
        unsigned char reference_byte = baseline_buffers[0].data[i];
        for (j = 1; j < NUM_BASELINE_BUFFERS; j++) {
            if (baseline_buffers[j].data[i] != reference_byte) {
                static_bytes[i] = 1;  // Mark as changing
            }
        }
    }
    printf("\n");
    printf("RULE 1: Static bytes\n");
    // Print the expected "normal" for the buffer
    printf("Expected Normal  : ");
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (!static_bytes[i]) {
            printf("%02X ", baseline_buffers[0].data[i]);
        } else {
            printf("?? ");
        }
    }
    printf("\n");

    // Check the input buffer for anomalies in static bytes
    int anomaly_detected = 0;
    printf("Input Buffer     : ");
    printBuffer(input_buffer);
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (!static_bytes[i] && baseline_buffers[0].data[i] != input_buffer[i]) {
            printf(" [ALERT] --> Anomaly at position %d (Previously STATIC byte %02X has changed to %02X)\n", i, baseline_buffers[0].data[i], input_buffer[i]);
            anomaly_detected = 1;
        }
    }

    /*
      COUNTER CHECK 1
    */
    // An array to store whether each element in baseline_buffers follows a specific pattern.
    // 0: constant, 1: changing
    int counter_check1[BUFFER_SIZE] = {0};

    // Iterate over each element in the arrays
    for (i = 0; i < BUFFER_SIZE; i++) {
        // Set the reference value as the value of the first buffer at the current index
        int reference_value = baseline_buffers[0].data[i];

        // Calculate the step as the difference between the values of the second and first buffers at the current index
        step = baseline_buffers[1].data[i] - baseline_buffers[0].data[i];

        // Assume it's a counter (mark as constant)
        counter_check1[i] = 0;

        // Iterate over additional buffers (starting from the third buffer)
        for (j = 1; j < NUM_BASELINE_BUFFERS; j++) {
            // Check if the value at the current index of the current buffer
            // is not equal to the expected value based on the reference value and step
            if ((baseline_buffers[j].data[i] - baseline_buffers[j-1].data[i] != step) || step == 0) {
                // If there is a mismatch, mark the corresponding element in counter_check1 as 1 (changing)
                counter_check1[i] = 1;
                //printf("\nByte %d is NOT a counter\n",i);
                // Break out of the inner loop since the pattern is not consistent for this element
            }
        }
    }
    printf("RULE 2: Expected counter values\n");
    // Print the expected "normal" for the buffer
    printf("Expected Normal  : ");
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (counter_check1[i]==0) {
            printf("%d  ",baseline_buffers[1].data[i] - baseline_buffers[0].data[i]);
        }else{
          printf("   ");
        }
    }

    printf("\n");

    // Check the input buffer for anomalies in counter check 1
    printf("Input Buffer     : ");
    printBuffer(input_buffer);
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (!counter_check1[i] && (input_buffer[i] - baseline_buffers[0].data[i]) % (baseline_buffers[1].data[i] - baseline_buffers[0].data[i]) != 0) {
            printf(" [ALERT] --> Anomaly at position %d (a known COUNTER with expected change of %d, has changed in an unexpected way)\n", i,baseline_buffers[1].data[i] - baseline_buffers[0].data[i]);
            anomaly_detected = 1;
        }
    }

    printf("\n");
    /*
      End of detection logic, return result
    */
    if (anomaly_detected) {
        return 1;  // Anomaly detected
    } else {
        return 0;  // No anomalies detected
    }
}