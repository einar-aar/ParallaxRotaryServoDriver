
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 12800000UL

// Program for ATmega162 clocked with 12.8 MHz crystal

volatile uint8_t cycle_time = 0
volatile uint16_t overflow = 0;
volatile bool rising = true;

void timer1Init(void) {

    cli();

    DDRE |= (1 << DDE2);
    DDRE &= ~(1 << DDE0);
    PORTE |= (1 << DDE0);

    TCCR1A |= (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);

    TCCR1B |= (1 << ICNC1) | (1 << ICES1) | (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    TIFR |= (1 << ICF1); // SET TO CLEAR

    TIMSK |= (1 << TISIE1) | (1 << TOIE1);

    OCR1A = 31999;
    OCR1B = 2399;

    sei();
}

ISR (TIMER1_CAPT_vect) {

    int time_temp = ICR1;

    if (rising) {

        rising = false;
        int start_time = time_temp;
        overflow = 0;
        TCCR1B &= ~(1 << ICES1);

    } else {

        cycle_time = time_temp + overflow * OCR1B - start_time;
        rising = true;
        TCCR1B |= (1 << ICES1);
    }
}

ISR (TIMER1_OVF_vect) {

    overflow++;
}

void setServoSpeed(int16_t servospeed) {

    if (servospeed < 2047 || servospeed > 2751) return;

    OCR1B = servospeed;
}

float getServoAngle(void) {

    // 51 is minimum timerticks per duty cycle
    // 1706 is max timerticks per duty cycle
    return (cycle_time - 51) / (1706 - 51) * 360;
}

void setServoMidPos(void) {

    // Angle from 90 to 270 degrees
    float angle = getServoAngle();

    if (angle > 180) setServoSpeed(-50);
    else if (angle < 180) setServoSpeed(50);

    while (getServoAngle() < 179.9 && getServoAngle() > 180.1);
    
    setServoSpeed((2751 - 2047) / 2);
}

void main() {

    timer1Init();

    while (1);
}