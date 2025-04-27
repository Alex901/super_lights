#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_partition.h"
#include "esp_timer.h"
#include "memory_control.h"
#include "settings_control.h" // To access current settings

#define MAX_EVENT_SIZE 64 // Maximum size of each event (in bytes)
#define MAX_EVENTS 256    // Maximum number of events
#define QUEUE_LENGTH 10   // Maximum number of events in the queue

static const esp_partition_t *memory_partition = NULL;
static int event_index = 0;              // Index for the next event
static QueueHandle_t event_queue = NULL; // Queue for log events

// Local copy of settings to track changes
static int last_light_state = -1;
static int last_brightness = -1;
static int last_color = -1;

void memory_control_init(void)
{
    // Locate the custom partition
    memory_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0x40, "memory");
    if (memory_partition == NULL)
    {
        printf("Error: Memory partition not found\n");
        return;
    }

    printf("Memory partition found: Offset = 0x%lx, Size = 0x%lx\n",
           (unsigned long)memory_partition->address, (unsigned long)memory_partition->size);

    // Create the event queue
    event_queue = xQueueCreate(QUEUE_LENGTH, MAX_EVENT_SIZE);
    if (event_queue == NULL)
    {
        printf("Error: Failed to create event queue\n");
        return;
    }

    printf("Event queue created successfully\n");

    // Write a header to the partition if it doesn't already exist
    const char *header = "Logged time events";
    char existing_header[MAX_EVENT_SIZE] = {0};

    // Read the first bytes of the partition to check for an existing header
    esp_err_t err = esp_partition_read(memory_partition, 0, existing_header, strlen(header) + 1);
    if (err == ESP_OK && strcmp(existing_header, header) == 0)
    {
        printf("Header already exists: %s\n", existing_header);
    }
    else
    {
        // Write the header to the partition
        err = esp_partition_write(memory_partition, 0, header, strlen(header) + 1);
        if (err == ESP_OK)
        {
            printf("Header written: %s\n", header);
        }
        else
        {
            printf("Failed to write header: %s\n", esp_err_to_name(err));
            return;
        }
    }

    // Initialize the event index by scanning the partition
    size_t header_size = strlen(header) + 1;
    size_t offset = MAX_EVENT_SIZE; // Start after the header
    char event[MAX_EVENT_SIZE];
    event_index = 1; // Start with 1 for the header block

    printf("DEBUG header_size: %zu\n", header_size);
    printf("DEBUG initial_offset: %zu\n", offset);
    printf("DEBUG partition_size: %lu\n", memory_partition->size);

    while (offset < memory_partition->size)
    {
        memset(event, 0, sizeof(event));
        err = esp_partition_read(memory_partition, offset, event, sizeof(event));
        if (err != ESP_OK || event[0] == '\0' || event[0] == 0xFF)
        {
            printf("DEBUG encountered empty or invalid block at offset: %zu\n", offset);
            break; // Stop if we encounter an empty or invalid event
        }

        printf("DEBUG valid event found at offset: %zu\n", offset);
        event_index++;            // Increment the event index for each valid event
        offset += MAX_EVENT_SIZE; // Move to the next event block
    }

    printf("DEBUG final_event_index: %d\n", event_index);
}

void memory_log_event(const char *event)
{
    if (event_queue == NULL)
    {
        printf("Error: Event queue not initialized\n");
        return;
    }

    // Get the current timestamp in milliseconds
    uint64_t timestamp_ms = esp_timer_get_time() / 1000;

    // Convert timestamp to hours, minutes, and seconds
    uint64_t total_seconds = timestamp_ms / 1000;
    uint64_t hours = (total_seconds / 3600) % 24;
    uint64_t minutes = (total_seconds / 60) % 60;
    uint64_t seconds = total_seconds % 60;

    // Format the event with the readable timestamp
    char formatted_event[MAX_EVENT_SIZE] = {0};
    printf("Event: %s\n", event);
    printf("Timestamp: %llu:%llu:%llu\n", hours, minutes, seconds);
    snprintf(formatted_event, sizeof(formatted_event), "time: %02llu:%02llu:%02llu - %s", hours, minutes, seconds, event);

    // Calculate the offset for the event
    size_t header_size = MAX_EVENT_SIZE; // Remove
    size_t offset = event_index * MAX_EVENT_SIZE;

    printf("DEBUG header_size: %zu\n", header_size);
    printf("DEBUG calculated_offset: %zu\n", offset);
    printf("DEBUG partition_size: %lu\n", memory_partition->size);

    // Ensure the offset does not exceed the partition size
    if (offset + MAX_EVENT_SIZE > memory_partition->size)
    {
        printf("Error: Memory partition full\n");
        return;
    }

    // Write the event to the partition
    esp_err_t err = esp_partition_write(memory_partition, offset, formatted_event, MAX_EVENT_SIZE);
    if (err == ESP_OK)
    {
        printf("Logged event: %s\n", formatted_event);
        event_index++;
        printf("DEBUG updated_event_index: %d\n", event_index);
    }
    else
    {
        printf("Failed to log event: %s\n", esp_err_to_name(err));
    }
}

void memory_control_task(void *pvParameters)
{
    char event[MAX_EVENT_SIZE];

    while (1)
    {
        // Check for changes in settings
        Settings *settings = settings_get();

        // Check light state
        if (settings->light != last_light_state)
        {
            snprintf(event, sizeof(event), "Light %s", settings->light ? "ON" : "OFF");
            memory_log_event(event);
            last_light_state = settings->light;
        }

        // Check brightness
        if (settings->brightness != last_brightness)
        {
            snprintf(event, sizeof(event), "Brightness set to %d%%", settings->brightness);
            memory_log_event(event);
            last_brightness = settings->brightness;
        }

        // Check color
        if (settings->selected_color != last_color)
        {
            snprintf(event, sizeof(event), "Color changed to %s", settings_get_color().name);
            memory_log_event(event);
            last_color = settings->selected_color;
        }

        // Process events from the queue
        if (xQueueReceive(event_queue, event, pdMS_TO_TICKS(100)) == pdPASS)
        {
            if (memory_partition == NULL)
            {
                printf("Error: Memory partition not initialized\n");
                continue;
            }

            // Calculate the offset for the event
            size_t offset = (event_index * MAX_EVENT_SIZE) + strlen("Logged time events") + 1;
            if (offset + MAX_EVENT_SIZE > memory_partition->size)
            {
                printf("Error: Memory partition full\n");
                continue;
            }

            // Write the event to the partition
            esp_err_t err = esp_partition_write(memory_partition, offset, event, strlen(event) + 1);
            if (err == ESP_OK)
            {
                printf("Logged event: %s\n", event);
                event_index++;
            }
            else
            {
                printf("Failed to log event: %s\n", esp_err_to_name(err));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Check for changes every 500ms
    }
}

void memory_print_all_events(void)
{
    if (memory_partition == NULL)
    {
        printf("Error: Memory partition not initialized\n");
        return;
    }

    printf("Reading all logged events:\n");

    // Start reading after the header
    size_t header_size = strlen("Logged time events") + 1;
    size_t offset = header_size;

    for (int i = 0; i < event_index; i++)
    {
        char event[MAX_EVENT_SIZE];
        esp_err_t err = esp_partition_read(memory_partition, offset, event, sizeof(event));
        if (err == ESP_OK && event[0] != '\0')
        {
            printf("Event %d: %s\n", i + 1, event);
        }
        else if (err == ESP_ERR_INVALID_SIZE || event[0] == '\0')
        {
            break; // No more events
        }
        else
        {
            printf("Failed to read event %d: %s\n", i + 1, esp_err_to_name(err));
        }

        offset += MAX_EVENT_SIZE; // Move to the next event
    }
}

void memory_clear_partition(void)
{
    if (memory_partition == NULL)
    {
        printf("Error: Memory partition not initialized\n");
        return;
    }

    // Erase the entire partition
    esp_err_t err = esp_partition_erase_range(memory_partition, 0, memory_partition->size);
    if (err == ESP_OK)
    {
        printf("Memory partition cleared successfully.\n");
        event_index = 0; // Reset the event index

        // Reinitialize the memory to restore the header
        memory_control_init();
    }
    else
    {
        printf("Failed to clear memory partition: %s\n", esp_err_to_name(err));
    }
}

const char *memory_get_usage(void)
{
    static char usage[32]; // Static buffer to hold the formatted string

    if (memory_partition == NULL)
    {
        snprintf(usage, sizeof(usage), "Error: No partition");
        return usage;
    }

    // Calculate the total memory used
    size_t header_size = strlen("Logged time events") + 1;
    size_t used_memory = header_size + (event_index * MAX_EVENT_SIZE);
    size_t total_memory = memory_partition->size;

    // Format the usage as "x/y kB"
    snprintf(usage, sizeof(usage), "Usage: %zu/%zu kB", used_memory / 1024, total_memory / 1024);

    return usage;
}