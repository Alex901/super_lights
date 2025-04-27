#include "display_control.h"
#include "driver/i2c.h"
#include <string.h>

#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
#define I2C_MASTER_SDA_IO 22     // GPIO for SDA
#define I2C_MASTER_SCL_IO 23     // GPIO for SCL
#define I2C_MASTER_FREQ_HZ 100000 // I2C clock frequency

// I2C address of the 1602IIC display
#define LCD_ADDR 0x27

// LCD commands
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_RETURN_HOME 0x02
#define LCD_CMD_FUNCTION_SET 0x28 // 4-bit mode, 2 lines, 5x8 dots
#define LCD_CMD_DISPLAY_ON 0x0C   // Display ON, cursor OFF, blink OFF
#define LCD_CMD_ENTRY_MODE 0x06   // Increment cursor, no display shift

// Control bits for PCF8574
#define LCD_BACKLIGHT 0x08 // Backlight ON
#define LCD_ENABLE    0x04 // Enable bit
#define LCD_RW        0x02 // Read/Write bit (0 = Write)
#define LCD_RS        0x01 // Register Select bit (0 = Command, 1 = Data)

// Helper function to send a nibble (4 bits) to the LCD via PCF8574
static esp_err_t lcd_send_nibble(uint8_t nibble, uint8_t control)
{
    uint8_t data = (nibble & 0xF0) | control | LCD_BACKLIGHT; // Combine nibble, control bits, and backlight
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true); // Send data with Enable LOW
    i2c_master_write_byte(cmd, data | LCD_ENABLE, true); // Pulse Enable HIGH
    i2c_master_write_byte(cmd, data, true); // Pulse Enable LOW
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Helper function to send a command to the LCD
static esp_err_t lcd_send_command(uint8_t command)
{
    // Send the high nibble (upper 4 bits)
    lcd_send_nibble(command & 0xF0, 0x00); // RS = 0, RW = 0 (Command mode)

    // Send the low nibble (lower 4 bits)
    lcd_send_nibble((command << 4) & 0xF0, 0x00); // RS = 0, RW = 0 (Command mode)

    vTaskDelay(pdMS_TO_TICKS(2)); // Wait for the command to complete
    return ESP_OK;
}

// Helper function to send data (characters) to the LCD
static esp_err_t lcd_send_data(uint8_t data)
{
    // Send the high nibble (upper 4 bits)
    lcd_send_nibble(data & 0xF0, LCD_RS); // RS = 1, RW = 0 (Data mode)

    // Send the low nibble (lower 4 bits)
    lcd_send_nibble((data << 4) & 0xF0, LCD_RS); // RS = 1, RW = 0 (Data mode)

    vTaskDelay(pdMS_TO_TICKS(2)); // Wait for the data to be written
    return ESP_OK;
}

void display_init(void)
{
    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Wait for the LCD to power up
    vTaskDelay(pdMS_TO_TICKS(50));

    // Initialize the LCD in 4-bit mode
    lcd_send_nibble(0x30, 0x00); // Function set (8-bit mode)
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_send_nibble(0x30, 0x00); // Function set (8-bit mode)
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_send_nibble(0x20, 0x00); // Function set (4-bit mode)

    // Send initialization commands
    lcd_send_command(LCD_CMD_FUNCTION_SET); // 4-bit mode, 2 lines, 5x8 dots
    lcd_send_command(LCD_CMD_DISPLAY_ON);   // Display ON, cursor OFF, blink OFF
    lcd_send_command(LCD_CMD_CLEAR_DISPLAY); // Clear the display
    lcd_send_command(LCD_CMD_ENTRY_MODE);   // Increment cursor, no display shift
}

void display_clear(void)
{
    // Send the clear display command
    lcd_send_command(LCD_CMD_CLEAR_DISPLAY);
    vTaskDelay(pdMS_TO_TICKS(2)); // Wait for the command to complete
}

void display_render(const char *line1, const char *line2)
{
    // Clear the display first
    display_clear();

    // Set cursor to the beginning of the first line
    lcd_send_command(0x80); // DDRAM address 0x00
    for (size_t i = 0; i < strlen(line1); i++)
    {
        lcd_send_data(line1[i]);
    }

    // Set cursor to the beginning of the second line
    lcd_send_command(0xC0); // DDRAM address 0x40
    for (size_t i = 0; i < strlen(line2); i++)
    {
        lcd_send_data(line2[i]);
    }
}


// These two should be connected to special mode(s), but not today! :) 
void display_enable_cursor(void)
{
    lcd_send_command(0x0E); // Display ON, Cursor ON, Blink OFF
}

void display_disable_cursor(void)
{
    lcd_send_command(0x0C); // Display ON, Cursor OFF, Blink OFF
    vTaskDelay(pdMS_TO_TICKS(2));
}

void display_highlight_row(int row)
{
    if (row == 1)
    {
        lcd_send_command(0x80); // Move cursor to the first row (DDRAM address 0x00)
    }
    else if (row == 2)
    {
        lcd_send_command(0xC0); // Move cursor to the second row (DDRAM address 0x40)
    }

    // Enable blinking for the selected row
    lcd_send_command(0x0F); // Display ON, Cursor ON, Blink ON
}

void display_loading_animation(const char *message)
{
    display_clear();
    display_render(message, ""); // Display the loading message

    char loading_bar[17] = ""; // 16 characters max for the second line
    for (int i = 0; i < 16; i++)
    {
        loading_bar[i] = '#'; // Add a block to the loading bar
        loading_bar[i + 1] = '\0'; // Null-terminate the string
        display_render(message, loading_bar); // Update the display
        vTaskDelay(pdMS_TO_TICKS(125)); // Wait for 125ms (16 steps * 125ms = 2 seconds)
    }
    vTaskDelay(pdMS_TO_TICKS(1500)); // Wait for 500ms before clearing the display
    display_clear(); // Clear the display after the loading animation
    display_render("All ready", "to go!!"); // Display completion message
    vTaskDelay(pdMS_TO_TICKS(2500)); 
}