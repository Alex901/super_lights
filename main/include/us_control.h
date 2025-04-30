#ifndef US_CONTROL_H
#define US_CONTROL_H

// Initialize the ultrasonic sensor
void us_sensor_init(void);

// Measure distance using the ultrasonic sensor
float us_sensor_get_distance(void);

// Control the ultrasonic sensor
void us_sensor_control(void);

// The task 
void us_sensor_task(void *pvParameters);

#endif // US_CONTROL_H