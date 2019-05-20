
#include "gpio.h"

/* Function defined in gpioFunctions.s. */
extern void SetGpioFunction(unsigned int gpio_register, unsigned int function);

/* Function defined in gpioFunctions.s. */
extern void SetGpio(unsigned int gpio_register, unsigned int value);


/* Sets the function of the GPIO register. */
void gpio_enable_function(unsigned int gpio_register, unsigned int function) {
  SetGpioFunction(gpio_register, function);
}

/* Sets the GPIO pin addressed to high if value is different than zero and
 * low otherwise.*/
void gpio_set_register(unsigned int gpio_register, unsigned int value) {
  SetGpio(gpio_register, value);
}

