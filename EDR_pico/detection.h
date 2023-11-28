#include <stdio.h>
#include <string.h>
#include <math.h>

#define NUM_BASELINE_BUFFERS 5
#define BUFFER_SIZE 4
double avg_entropy = 0;
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

double calculate_entropy(const unsigned char *input_buffer, size_t buffer_size) {
    if (input_buffer == NULL || buffer_size == 0) {
        return 0.0;  // Invalid input
    }

    // Count the occurrences of each byte value
    int byte_count[256] = {0};
    for (size_t i = 0; i < buffer_size; i++) {
        byte_count[input_buffer[i]]++;
    }

    // Calculate the probability of each byte value
    double probability[256];
    for (int i = 0; i < 256; i++) {
        probability[i] = (double)byte_count[i] / buffer_size;
    }

    // Calculate entropy
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (probability[i] > 0) {
            entropy += probability[i] * log2(1.0 / probability[i]);
        }
    }

    return entropy;
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


/* ENTROPY CALC */
    double total_entropy = 0;
    for (i = 0; i < NUM_BASELINE_BUFFERS; i++) {
      total_entropy += calculate_entropy(baseline_buffers[i].data,BUFFER_SIZE);
      // printf("total entropy %f\n",total_entropy);
    }
    avg_entropy = total_entropy/NUM_BASELINE_BUFFERS;
    // printf("avg entropy %f\n",avg_entropy);

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
            printf(" [ALERT] --> Anomaly at position %d (a known COUNTER with expected change of %d, has changed in an unexpected way, off by %d)\n", i,baseline_buffers[1].data[i] - baseline_buffers[0].data[i],(input_buffer[i] - baseline_buffers[0].data[i]) % (baseline_buffers[1].data[i] - baseline_buffers[0].data[i]));
            anomaly_detected = 1;
        }
    }

    printf("RULE 3: Expected entropy range\n");
    double current_entropy = calculate_entropy(input_buffer,BUFFER_SIZE);
    printf("Expected Normal  : %f\n",avg_entropy);
    printf("Input Buffer     : %f\n",current_entropy);

    if(current_entropy > 1.5 * avg_entropy || calculate_entropy(input_buffer,BUFFER_SIZE) < 0.5 * avg_entropy){
      printf(" [ALERT] --> Anomaly - entropy change to %f from average of %f\n", current_entropy, avg_entropy);
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