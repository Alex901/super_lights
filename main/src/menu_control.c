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
void toggle_us(void);
void adjust_brightness(void);
void adjust_ir_sensitivity(void);
void adjust_us_sensitivity(void);

// Timings submenu
MenuItem timings_menu[] = {
    {"Auto unplug", NULL, toggle_auto_unplug},
    {"IR Timing", NULL, NULL},
    {"UR Timing", NULL, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Sensitivity submenu
MenuItem sensitivity_menu[] = {
    {"IR Sense", NULL, adjust_ir_sensitivity},
    {"US distance", NULL, adjust_us_sensitivity},
    {NULL, NULL, NULL} // End of menu
};

// Light submenu
MenuItem light_menu[] = {
    {"Brightness", NULL, adjust_brightness},
    {"Color", NULL, select_color},
    {"IR", NULL, toggle_ir},
    {"US", NULL, toggle_us},
    {"Sensitivity", sensitivity_menu, NULL},
    {"Timings", timings_menu, NULL},
    {NULL, NULL, NULL} // End of menu
};

// Audio submenu
MenuItem audio_menu[] = {
    {"Sound", NULL, NULL},
    {"Signal", NULL, NULL},
    {"Volume", NULL, NULL},
    {"Mode", NULL, NULL},
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
    {"Light", NULL, toggle_light},
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

    // TODO: This is super-nasty, move to a helper method
    // Check if the first item is "Light" and append its current setting
    if (strcmp(item1_name, "Light") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "Light: %s", settings->light ? "On" : "Off");
    }
    else if (strcmp(item1_name, "Brightness") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "Brightness: %d%%", settings->brightness);
    }
    else if (strcmp(item1_name, "Color") == 0)
    {
        Settings *settings = settings_get();
        const char **color_names = settings_get_color_names();
        snprintf(line1, sizeof(line1), "Color: %s", color_names[settings->selected_color]);
    }
    else if (strcmp(item1_name, "Auto unplug") == 0)
    {
        Settings *settings = settings_get();
        if (settings->light_auto_turn_off == 0)
        {
            snprintf(line1, sizeof(line1), "Auto unplug: Off");
        }
        else
        {
            snprintf(line1, sizeof(line1), "Auto unplug: %ds", settings->light_auto_turn_off);
        }
    }
    else if (strcmp(item1_name, "IR") == 0) // Add IR setting
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "IR: %s", settings->ir ? "Active" : "Disabled");
    }
    else if (strcmp(item1_name, "IR Sense") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "IR Sense: %d%%", settings->sensitivity_ir);
    }
    else if (strcmp(item1_name, "US") == 0) // Add US setting
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "US: %s", settings->us ? "Active" : "Disabled");
    }
    else if (strcmp(item1_name, "US distance") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "US distance: %d cm", settings->sensitivity_ur);
    }
    else if (strcmp(item1_name, "UR Timing") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line1, sizeof(line1), "UR Timing: %dms", settings->timing_ur * 100);
    }
    else
    {
        snprintf(line1, sizeof(line1), "%s", item1_name);
    }

    // Check if the second item is "Light" and append its current setting
    if (strcmp(item2_name, "Light") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "Light: %s", settings->light ? "On" : "Off");
    }
    else if (strcmp(item2_name, "Brightness") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "Brightness: %d%%", settings->brightness);
    }
    else if (strcmp(item2_name, "Color") == 0)
    {
        Settings *settings = settings_get();
        const char **color_names = settings_get_color_names();
        snprintf(line2, sizeof(line2), "Color: %s", color_names[settings->selected_color]);
    }
    else if (strcmp(item2_name, "Auto unplug") == 0)
    {
        Settings *settings = settings_get();
        if (settings->light_auto_turn_off == 0)
        {
            snprintf(line2, sizeof(line2), "Auto unplug: Off");
        }
        else
        {
            snprintf(line2, sizeof(line2), "Auto unplug: %ds", settings->light_auto_turn_off);
        }
    }
    else if (strcmp(item2_name, "IR") == 0) // Add IR setting
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "IR: %s", settings->ir ? "Active" : "Disabled");
    }
    else if (strcmp(item2_name, "IR Sense") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "IR Sense: %d%%", settings->sensitivity_ir);
    }
    else if (strcmp(item2_name, "US") == 0) // Add US setting
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "US: %s", settings->us ? "Active" : "Disabled");
    }
    else if (strcmp(item2_name, "US distance") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "US distance: %d cm", settings->sensitivity_ur);
    }
    else if (strcmp(item2_name, "UR Timing") == 0)
    {
        Settings *settings = settings_get();
        snprintf(line2, sizeof(line2), "UR Timing: %dms", settings->timing_ur * 100);
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
                scroll_offset++; // Scroll down
            }
        }
    }
    else
    {
        printf("Oh noes, you can't go any further down! \n");
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
                scroll_offset--; // Scroll up
            }
        }
    }
    else
    {
        printf("Oh noes, you can't go any further up \n");
    }

    // Re-render the menu
    menu_render();
}

// Action: Toggle light on/off
void toggle_light(void)
{
    Settings *settings = settings_get();
    settings->light = !settings->light; // Toggle the light setting
    menu_render();
}

void toggle_ir(void)
{
    Settings *settings = settings_get();
    settings->ir = !settings->ir; // Toggle the IR setting
    menu_render();
}

void toggle_us(void)
{
    Settings *settings = settings_get();
    settings->us = !settings->us; // Toggle the US setting
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

// Menus, these should be in their own files. But for now, they are here

// Action: Display the about page
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

// Action: Adjust brightness
void adjust_brightness(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to wait for enter button release

    Settings *settings = settings_get();
    int original_brightness = settings->brightness; // Save the original brightness
    int brightness = settings->brightness;          // Current brightness value

    is_in_special_mode_lr = true; // Enable special mode for left-right behavior
    display_disable_cursor();     // Disable the cursor

    // Render the initial brightness adjustment screen
    char slider[18];                                 // 16 characters for the display + 2 for null terminator
    snprintf(slider, sizeof(slider), "[%-14s]", ""); // Empty slider with stoppers
    int slider_position = (brightness * 14) / 100;   // Map brightness to slider position
    for (int i = 0; i < slider_position; i++)
    {
        slider[i + 1] = '|'; // Fill the slider between the stoppers
    }

    display_render("Adjust Brightness", slider);

    while (1)
    {
        // Handle button presses for adjusting brightness
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && brightness < 100) // DOWN acts as RIGHT
        {
            brightness += 100 / 14; // Increment brightness by one tick
            if (brightness > 100)
                brightness = 100; // Cap at 100%

            // Update the slider
            slider_position = (brightness * 14) / 100;
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust Brightness", slider);

            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && brightness > 0) // UP acts as LEFT
        {
            brightness -= 100 / 14; // Decrement brightness by one tick
            if (brightness < 0)
                brightness = 0; // Cap at 0%

            // Update the slider
            slider_position = (brightness * 14) / 100;
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust Brightness", slider);

            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Save the current brightness and exit
            settings->brightness = brightness;
            printf("Brightness set to %d%%\n", brightness);
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Restore the original brightness and exit
            settings->brightness = original_brightness;
            printf("Brightness reverted to %d%%\n", original_brightness);
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }

    is_in_special_mode_lr = false; // Disable special mode
    display_enable_cursor();       // Re-enable the cursor
    menu_render();                 // Re-render the menu after exiting
    printf("Exiting brightness adjustment\n");
}

void select_color(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to wait for enter button release

    Settings *settings = settings_get();
    int original_color_index = settings->selected_color; // Save the original color index
    int color_index = settings->selected_color;          // Current color index

    // Fetch the list of colors from settings
    const char **colors = settings_get_color_names();

    is_in_special_mode_lr = true; // Enable special mode for left-right behavior
    display_disable_cursor();     // Disable the cursor

    // Render the initial color selection screen
    char display_line[20];
    snprintf(display_line, sizeof(display_line), "< %s >", colors[color_index]);
    display_render("Select Color", display_line);

    while (1)
    {
        // Handle button presses for cycling through colors
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && colors[color_index + 1] != NULL) // DOWN acts as RIGHT
        {
            color_index++; // Move to the next color

            // Update the display
            snprintf(display_line, sizeof(display_line), "< %s >", colors[color_index]);
            display_render("Select Color", display_line);

            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && color_index > 0) // UP acts as LEFT
        {
            color_index--; // Move to the previous color

            // Update the display
            snprintf(display_line, sizeof(display_line), "< %s >", colors[color_index]);
            display_render("Select Color", display_line);

            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Save the selected color and exit
            settings->selected_color = color_index;
            printf("Color set to: %s\n", colors[color_index]);
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Restore the original color and exit
            settings->selected_color = original_color_index;
            printf("Color reverted to: %s\n", colors[original_color_index]);
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }

    is_in_special_mode_lr = false; // Disable special mode
    display_enable_cursor();       // Re-enable the cursor
    menu_render();                 // Re-render the menu after exiting
    printf("Exiting color selection\n");
}

// Action: Toggle auto unplug setting
void toggle_auto_unplug(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    Settings *settings = settings_get();

    // Define the available options
    const int options[] = {0, 5, 10, 15, 30, 60, 120, 300, 500, 600};
    const int num_options = sizeof(options) / sizeof(options[0]);

    int original_index = 0;
    int current_index = 0;

    // Find the current index based on the saved value
    for (int i = 0; i < num_options; i++)
    {
        if (options[i] == settings->light_auto_turn_off)
        {
            original_index = i;
            current_index = i;
            break;
        }
    }

    is_in_special_mode_lr = true; // Enable special mode for left-right behavior
    display_disable_cursor();     // Disable the cursor

    // Function to render the visible range
    void render_view(void)
    {
        // Calculate the start index for the visible range
        int start_index = current_index - 2;
        if (start_index < 0)
            start_index = 0;
        if (start_index > num_options - 5)
            start_index = num_options - 5;

        // Render the visible range
        char visible[20] = "";
        for (int i = start_index; i < start_index + 5 && i < num_options; i++)
        {
            char temp[6];
            if (i == current_index)
                snprintf(temp, sizeof(temp), "[%d]", options[i]);
            else
                snprintf(temp, sizeof(temp), " %d ", options[i]);
            strncat(visible, temp, sizeof(visible) - strlen(visible) - 1);
        }

        char display_line[20];
        snprintf(display_line, sizeof(display_line), "%s", visible);
        display_render("Auto Unplug", display_line);
    }

    // Initial render
    render_view();

    while (1)
    {
        bool button_pressed = false;

        // Handle button presses
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && current_index < num_options - 1) // DOWN acts as RIGHT
        {
            current_index++; // Move to the next option
            button_pressed = true;
            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && current_index > 0) // UP acts as LEFT
        {
            current_index--; // Move to the previous option
            button_pressed = true;
            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Save the selected value and exit
            settings->light_auto_turn_off = options[current_index];
            printf("Auto unplug set to: %d\n", options[current_index]);
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Restore the original value and exit
            settings->light_auto_turn_off = options[original_index];
            printf("Auto unplug reverted to: %d\n", options[original_index]);
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        // Re-render only if a button was pressed
        if (button_pressed)
        {
            render_view();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }

    is_in_special_mode_lr = false; // Disable special mode
    display_enable_cursor();       // Re-enable the cursor
    menu_render();                 // Re-render the menu after exiting
    printf("Exiting auto unplug selection\n");
}

void adjust_ir_sensitivity(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to wait for enter button release

    Settings *settings = settings_get();
    int original_sensitivity = settings->sensitivity_ir; // Save the original sensitivity
    int sensitivity = settings->sensitivity_ir;          // Current sensitivity value

    is_in_special_mode_lr = true; // Enable special mode for left-right behavior
    display_disable_cursor();     // Disable the cursor

    // Render the initial sensitivity adjustment screen
    char slider[18];                                 // 16 characters for the display + 2 for null terminator
    snprintf(slider, sizeof(slider), "[%-14s]", ""); // Empty slider with stoppers
    int slider_position = (sensitivity * 14) / 100;  // Map sensitivity to slider position
    for (int i = 0; i < slider_position; i++)
    {
        slider[i + 1] = '|'; // Fill the slider between the stoppers
    }

    display_render("Adjust IR Sens.", slider);

    while (1)
    {
        // Handle button presses for adjusting sensitivity
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && sensitivity < 100) // DOWN acts as RIGHT
        {
            sensitivity += 100 / 14; // Increment sensitivity by one tick
            if (sensitivity > 100)
                sensitivity = 100; // Cap at 100%

            // Update the slider
            slider_position = (sensitivity * 14) / 100;
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust IR Sens.", slider);

            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && sensitivity > 0) // UP acts as LEFT
        {
            sensitivity -= 100 / 14; // Decrement sensitivity by one tick
            if (sensitivity < 0)
                sensitivity = 0; // Cap at 0%

            // Update the slider
            slider_position = (sensitivity * 14) / 100;
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust IR Sens.", slider);

            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Save the current sensitivity and exit
            settings->sensitivity_ir = sensitivity;
            printf("IR Sensitivity set to %d%%\n", sensitivity);
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Restore the original sensitivity and exit
            settings->sensitivity_ir = original_sensitivity;
            printf("IR Sensitivity reverted to %d%%\n", original_sensitivity);
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }

    is_in_special_mode_lr = false; // Disable special mode
    display_enable_cursor();       // Re-enable the cursor
    menu_render();                 // Re-render the menu after exiting
    printf("Exiting IR sensitivity adjustment\n");
}

void adjust_us_sensitivity(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to wait for enter button release

    Settings *settings = settings_get();
    int original_sensitivity = settings->sensitivity_ur; // Save the original sensitivity
    int sensitivity = settings->sensitivity_ur;          // Current sensitivity value

    is_in_special_mode_lr = true; // Enable special mode for left-right behavior
    display_disable_cursor();     // Disable the cursor

    // Render the initial sensitivity adjustment screen
    char slider[18];                                 // 16 characters for the display + 2 for null terminator
    snprintf(slider, sizeof(slider), "[%-14s]", ""); // Empty slider with stoppers
    int slider_position = ((sensitivity - 2) * 14) / (400 - 2); // Map sensitivity to slider position
    for (int i = 0; i < slider_position; i++)
    {
        slider[i + 1] = '|'; // Fill the slider between the stoppers
    }

    display_render("Adjust US Sens.", slider);

    while (1)
    {
        // Handle button presses for adjusting sensitivity
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0 && sensitivity < 400) // DOWN acts as RIGHT
        {
            sensitivity += (400 - 2) / 14; // Increment sensitivity by one tick
            if (sensitivity > 400)
                sensitivity = 400; // Cap at 400 cm

            // Update the slider
            slider_position = ((sensitivity - 2) * 14) / (400 - 2);
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust US Sens.", slider);

            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(UP_BUTTON_GPIO) == 0 && sensitivity > 2) // UP acts as LEFT
        {
            sensitivity -= (400 - 2) / 14; // Decrement sensitivity by one tick
            if (sensitivity < 2)
                sensitivity = 2; // Cap at 2 cm

            // Update the slider
            slider_position = ((sensitivity - 2) * 14) / (400 - 2);
            snprintf(slider, sizeof(slider), "[%-14s]", "");
            for (int i = 0; i < slider_position; i++)
            {
                slider[i + 1] = '|';
            }

            display_render("Adjust US Sens.", slider);

            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                ; // Wait for button release
        }
        else if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            // Save the current sensitivity and exit
            settings->sensitivity_ur = sensitivity;
            printf("US Sensitivity set to %d cm\n", sensitivity);
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }
        else if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            // Restore the original sensitivity and exit
            settings->sensitivity_ur = original_sensitivity;
            printf("US Sensitivity reverted to %d cm\n", original_sensitivity);
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                ; // Wait for button release
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to debounce buttons
    }

    is_in_special_mode_lr = false; // Disable special mode
    display_enable_cursor();       // Re-enable the cursor
    menu_render();                 // Re-render the menu after exiting
    printf("Exiting US sensitivity adjustment\n");
}