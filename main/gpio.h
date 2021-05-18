#ifndef GPIO_H
#define GPIO_H

#define LED 2
#define BUTTON 0

void input_handler(void *params);
void set_up_gpio();
void turn_on_led();
void turn_off_led();

#endif
