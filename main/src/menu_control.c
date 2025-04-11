#include "menu_control.h"
#include "display_control.h"
#include "settings_control.h"
#include "gpio_control.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Track the scroll offset and cursor position
static int scroll_offset = 0;   // Index of the first visible menu item
static int cursor_position = 1; // 1 = Row 1, 2 = Row 2

// Menu item structure
typedef struct MenuItem
{
    const char *name;         // Name of the menu item
    struct MenuItem *submenu; // Pointer to the submenu (if any)
    void (*action)(void);     // Callback function for actions (if any)
} MenuItem;

// Current menu state
static MenuItem *current_menu = NULL;
static int current_selection = 0;
bool is_in_special_mode = false; // Flag to turn off normal key presses
bool is_in_special_mode_lr = false;

// Parent menu stack
#define MENU_STACK_SIZE 10
static MenuItem *menu_stack[MENU_STACK_SIZE];
static int menu_stack_index = -1;

// Forward declarations for actions
void toggle_light(void);
void adjust_brightness(void);

// Timings submenu
MenuItem timings_menu[] = {
    {"Turn Off Light", NULL, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Sensitivity submenu
MenuItem sensitivity_menu[] = {
    {"LR Sensitivity", NULL, NULL},
    {"UR Sensitivity", NULL, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Light submenu
MenuItem light_menu[] = {
    {"IR active", NULL, NULL},
    {"UR active", NULL, NULL},
    {"Brightness", NULL, adjust_brightness},
    {"Color", NULL, NULL},
    {"Light", NULL, toggle_light},
    {"Sensitivity", sensitivity_menu, NULL},
    {"Timings", timings_menu, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Audio submenu
MenuItem audio_menu[] = {
    {"Sound", NULL, NULL},
    {"Signal", NULL, NULL},
    {"Volume", NULL, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Settings submenu
MenuItem settings_menu[] = {
    {"Audio settings", audio_menu, NULL},
    {"Light settings", light_menu, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Top-level menu
MenuItem main_menu[] = {
    {"Settings", settings_menu, NULL},
    {"About", NULL, about_page},
    {NULL, NULL, NULL} // End of menu
};

// Initialize the menu
void menu_init(void)
{
    current_menu = main_menu;
    current_selection = 0;
    menu_stack_index = -1;
    scroll_offset = 0;   // Index of the first visible menu item
    cursor_position = 1; // 1 = Row 1, 2 = Row 2
    menu_render();
    display_highlight_row(1); // Highlight the first row
}

// Select the current menu item i.e enter the submenu or execute the action
void menu_select(void)
{
    MenuItem *selected_item = &current_menu[current_selection];
    printf("Selected item: %s\n", selected_item->name);
    printf("Current_selection: %d\n", current_selection);
    if (selected_item->submenu != NULL)
    {
        // Push the current menu onto the stack
        if (menu_stack_index < MENU_STACK_SIZE - 1)
        {
            menu_stack[++menu_stack_index] = current_menu;
        }

        // Enter the submenu
        current_menu = selected_item->submenu;
        current_selection = 0;
        scroll_offset = 0;
        cursor_position = 1; 
        menu_render();
    }
    else if (selected_item->action != NULL)
    {
        // Execute the action
        selected_item->action();
    }
}

// Go back to the parent menu
void menu_back(void)
{
    if (menu_stack_index >= 0)
    {
        // Pop the previous menu from the stack
        current_menu = menu_stack[menu_stack_index--];

        // Restore the previous selection (if needed, you can track it separately)
        current_selection = 0; 
        scroll_offset = 0;
        cursor_position = 1; 
        // Re-render the previous menu
        menu_render();
    }
    else
    {
        // Optional: Add feedback if already at the top-level menu
        printf("Already at the top-level menu, cannot go back further.\n");
    }
}

// Render the current menu
void menu_render(void)
{
    char line1[20];
    char line2[20];

    // Get the names of the two visible menu items
    const char *item1_name = current_menu[scroll_offset].name;
    const char *item2_name = (current_menu[scroll_offset + 1].name != NULL)
                                 ? current_menu[scroll_offset + 1].name
                                 : "";

    printf("Rendering :\n");
    printf("Line 1: %s\n", item1_name);
    printf("Line 2: %s\n", item2_name);

    // Check if the first item is "Light" and append its current setting
    if (strcmp(item1_name, "Light") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "Light: %s", settings->sound_on ? "On" : "Off");
    }
    else
    {
        snprintf(line1, sizeof(line1), "%s", item1_name);
    }

    // Check if the second item is "Light" and append its current setting
    if (strcmp(item2_name, "Light") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "Light: %s", settings->sound_on ? "On" : "Off");
    }
    else
    {
        snprintf(line2, sizeof(line2), "%s", item2_name);
    }

    // Render the menu with the selected row highlighted
    display_render(line1, line2);
    display_highlight_row(cursor_position); // Highlight the selected row (1 or 2)
}

void menu_scroll_down(void)
{
    // Check if there is a next menu item
    if (current_menu[current_selection + 1].name != NULL)
    {
        current_selection++; // Move to the next item

        // Adjust scroll offset if needed
        if (cursor_position == 1)
        {
            cursor_position = 2; // Move cursor to the second row
        }
        else if (cursor_position == 2)
        {
            // Scroll down if there are more items below
            if (current_menu[scroll_offset + 2].name != NULL)
            {
                scroll_offset++;     // Scroll down
            }
        }
    }
    else
    {
        printf("Already at the bottom of the menu\n");
    }

    // Re-render the menu
    menu_render();
}

void menu_scroll_up(void)
{
    // Check if there is a previous menu item
    if (current_selection > 0)
    {
        current_selection--; // Move to the previous item

        // Adjust scroll offset if needed
        if (cursor_position == 2)
        {
            cursor_position = 1; // Move cursor to the first row
        }
        else if (cursor_position == 1)
        {
            // Scroll up if there are more items above
            if (scroll_offset > 0)
            {
                scroll_offset--;     // Scroll up
            }
        }
    }
    else
    {
        printf("Already at the top of the menu\n");
    }

    // Re-render the menu
    menu_render();
}

// Action: Toggle light on/off
void toggle_light(void)
{
    Settings *settings = settings_get();
    settings->sound_on = !settings->sound_on; // Toggle the light setting
    menu_render();
}

// Action: Adjust brightness
void adjust_brightness(void)
{
    Settings *settings = settings_get();
    int brightness = settings->brightness;

    // Simulate a slider for brightness adjustment
    for (int i = 0; i <= 100; i += 10)
    {
        brightness = i;
        settings->brightness = brightness;
        char buffer[20]; // Increase buffer size to avoid truncation
        snprintf(buffer, sizeof(buffer), "Brightness: %d%%", brightness);
        display_render("Adjusting", buffer);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    menu_render();
}

void toggle_sound(void)
{
    Settings *settings = settings_get();
    settings->sound_on = !settings->sound_on; // Toggle the sound setting

    // Display the updated value
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Sound: %s", settings->sound_on ? "On" : "Off");
    display_render(buffer, "");
    vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second

    // Return to the menu
    menu_render();
}

void about_page(void)
{
    const char *about_text[] = {
        "Super Lights v1.0",
        "By: Alzner",
        "Embedded Systems",
        "Project Showcase",
        "Thank you !",
        "for using this",
        "  amazing app!",
        NULL // End of text
    };
    printf("entering about Page\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    is_in_special_mode = true; // Set the flag to disable normal key presses
    int scroll_offset = 0;
    display_disable_cursor();

    // Render the current and next lines of the about text
    const char *line1 = about_text[scroll_offset];
    const char *line2 = (about_text[scroll_offset + 1] != NULL)
                            ? about_text[scroll_offset + 1]
                            : "";

    // Debug: Log the about page rendering
    printf("About Page:\n");
    printf("Line 1: %s\n", line1);
    printf("Line 2: %s\n", line2);

    // Render the lines without highlighting
    display_render(line1, line2);

    while (1)
    {
        // Render the current and next lines of the about text
        const char *line1 = about_text[scroll_offset];
        const char *line2 = (about_text[scroll_offset + 1] != NULL)
                                ? about_text[scroll_offset + 1]
                                : "";

        // Debug: Log the about page rendering
        printf("About Page:\n");
        printf("Line 1: %s\n", line1);
        printf("Line 2: %s\n", line2);

        // Render the lines without highlighting

        // Handle button presses for scrolling
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && about_text[scroll_offset + 1] != NULL)
        {
            printf("Scrolling down\n");
            scroll_offset++; // Scroll down
            display_render(line1, line2);
            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && scroll_offset > 0)
        {
            printf("Scrolling up\n");
            scroll_offset--; // Scroll up
            display_render(line1, line2);
            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Exit the about page
            printf("Enter button pressed, should exit about page\n");
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Exit the about page
            printf("Exiting about page\n");
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }
    is_in_special_mode = false; // Reset the flag to enable normal key presses
    display_enable_cursor();    // Re-enable the cursor
    scroll_offset = 0;
    menu_render(); // Re-render the menu after exiting the about page 
}
