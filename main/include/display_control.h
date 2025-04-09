#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

void display_init(void); // Initialize the display
void display_clear(void); // Clear the display
void display_render(const char *line1, const char *line2); // Render two lines of text
void display_enable_cursor(void); // Enable the cursor
void display_disable_cursor(void); // Disable the cursor
void display_highlight_row(int row); // Highlight a specific row
void display_loading_animation(const char *message);


#endif // DISPLAY_CONTROL_H