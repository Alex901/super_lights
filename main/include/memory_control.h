#ifndef MEMORY_CONTROL_H
#define MEMORY_CONTROL_H

#include <stdbool.h>

// Initialize the memory control module
void memory_control_init(void);

// Log an event to memory (enqueue the event)
void memory_log_event(const char *event);

// Task function for memory control
void memory_control_task(void *pvParameters);

// Print all logged events from memory
void memory_print_all_events(void);

// Clear the memory partition
void memory_clear_partition(void); 

// Get current memory usage
const char *memory_get_usage(void);

#endif // MEMORY_CONTROL_H