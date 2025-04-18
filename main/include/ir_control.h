#ifndef IR_CONTROL_H
#define IR_CONTROL_H

// Initialize the IR sensor
void ir_sensor_init(void);

// Control the IR sensor (check for motion and activate light if needed)
void ir_sensor_control(void);


#endif // IR_CONTROL_H