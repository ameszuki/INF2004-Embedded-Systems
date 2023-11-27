#include <stdio.h>
#include <string.h>

#define NUM_BASELINE_BUFFERS 10
#define BUFFER_SIZE 16

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

int main() {
    const char *test_cases[] = {
      "\x41\x42\x28\x00\x00\x00\x21\x48\x09\x45\x57\x58\x59\x49\x00",
      "\x41\x42\x2d\x00\x00\x00\x35\x44\x3f\x4f\x57\x58\x59\x0b\x00",
      "\x41\x42\x32\x00\x00\x00\x34\x4d\x26\x71\x57\x58\x59\x34\x00",
      "\x41\x42\x37\x00\x00\x00\x4c\x6b\x76\x47\x57\x58\x59\x0e\x00",
      "\x41\x42\x3c\x00\x00\x00\x09\x6a\x63\x50\x57\x58\x59\x28\x00",
      "\x41\x42\x41\x00\x00\x00\x43\x09\x7b\x68\x57\x58\x59\x47\x00",
      "\x41\x42\x46\x00\x00\x00\x74\x5a\x2e\x3f\x57\x58\x59\x0c\x00",
      "\x41\x42\x4b\x00\x00\x00\x60\x2b\x74\x3b\x57\x58\x59\x25\x00",
      "\x41\x42\x50\x00\x00\x00\x5f\x6c\x23\x6b\x57\x58\x59\x15\x00",
      "\x41\x42\x55\x00\x00\x00\x7b\x58\x32\x75\x57\x58\x59\x2e\x00",
      "\x41\x42\x5a\x00\x00\x00\x7d\x72\x6b\x43\x57\x58\x59\x30\x00",
      "\x41\x42\xa5\x00\x00\x00\x33\x27\x2e\x68\x57\x58\x59\x47\x00",
      "\x41\x42\xaa\x00\x00\x00\x49\x22\x6f\x5f\x57\x58\x59\x12\x00",
      "\x41\x42\xaf\x00\x00\x00\x42\x3d\x4b\x4d\x57\x58\x59\x53\x00",
      "\x41\x42\xb4\x00\x00\x00\x2e\x58\x20\x58\x57\x58\x59\x58\x00",
      "\x41\x42\xb9\x00\x00\x00\x3a\x41\x09\x4b\x57\x58\x59\x07\x00",
      "\x41\x42\xbe\x00\x00\x00\x40\x2f\x33\x35\x57\x58\x59\x3e\x00",
      "\x41\x42\xc3\x00\x00\x00\x7c\x71\x2d\x5a\x57\x58\x59\x22\x00",
      "\x41\x42\xc8\x00\x00\x00\x77\x50\x33\x38\x57\x58\x59\x5d\x00",
      "\x41\x42\xcd\x00\x00\x00\x3b\x4d\x23\x35\x57\x58\x59\x54\x00",
      "\x41\x42\xd2\x00\x00\x00\x2d\x6a\x56\x4b\x57\x58\x59\x2e\x00",
      "\x41\x42\xd7\x00\x00\x00\x74\x4d\x40\x61\x57\x58\x59\x12\x00",
      "\x41\x42\xdc\x00\x00\x00\x3e\x7e\x09\x34\x57\x58\x59\x31\x00",
      "\x41\x42\xe1\x00\x00\x00\x0a\x59\x2d\x0d\x57\x58\x59\x2f\x00",
      "\x41\x42\xe6\x00\x00\x00\x78\x30\x30\x6e\x57\x58\x59\x13\x00",
      "\x41\x42\xeb\x00\x00\x00\x3e\x24\x3a\x73\x57\x58\x59\x51\x00",
      "\x41\x42\xf1\x00\x00\x00\x23\x7b\x24\x5a\x57\x58\x59\x44\x00",
      "\x41\x43\xf5\x00\x00\x00\x3d\x66\x7a\x66\x57\x58\x59\x55\x00",
    };



    // Process the remaining test cases
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        int result = detectMemoryAnomalies((unsigned char *)test_cases[i]);
        if (result) {
            printf("Anomaly detected in test case %d\n", i + 1);
        } else {
            printf("No anomaly detected in test case %d\n", i + 1);
        }
    }

    return 0;
}
