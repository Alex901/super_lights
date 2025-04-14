#ifndef MENU_CONTROL_H
#define MENU_CONTROL_H

#include <stdbool.h> // Include for the bool type

extern bool is_in_special_mode; // Declare the variable as extern
extern bool is_in_special_mode_lr;

void menu_init(void); // Initialize the menu
void menu_scroll_up(void); // Scroll up in the menu
void menu_scroll_down(void); // Scroll down in the menu
void menu_select(void); // Select the current menu item
void menu_back(void); // Go back to the parent menu
void menu_render(void); // Render the current menu
void toggle_sound(void); // Toggle sound setting
void adjust_brightness(void); // Adjust brightness setting
void toggle_light(void); // Toggle light setting
void about_page(void); // Display about page
void select_color(void); // Select color setting
void toggle_ir(void); // Toggle IR setting
void toggle_auto_unplug(void); // Toggle auto unplug setting

#endif // MENU_CONTROL_H