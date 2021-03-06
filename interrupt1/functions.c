#include "functions.h"
#include <inttypes.h>

uint8_t read(volatile uint8_t *port, uint8_t bit){
    return (*port & (1 << bit));
}

void high(volatile uint8_t *port , uint8_t bit){
    *port |= (1 << bit);
}

void low(volatile uint8_t *port , uint8_t bit){
    *port &= ~(1 << bit);
}
