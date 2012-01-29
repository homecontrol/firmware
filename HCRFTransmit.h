#ifndef HCRFTRANSMIT_H_
#define HCRFTRANSMIT_H_

inline void transmit(int pin, int pulse_length, int n_high, int n_low)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(pulse_length * n_high);
    digitalWrite(pin, LOW);
    delayMicroseconds(pulse_length * n_low);
}

inline void send_0(int pin, int pulse_length)
{
    transmit(pin, pulse_length, 1, 3);
    transmit(pin, pulse_length, 1, 3);
}

inline void send_1(int pin, int pulse_length)
{
    transmit(pin, pulse_length, 3, 1);
    transmit(pin, pulse_length, 3, 1);
}

inline void send_F(int pin, int pulse_length)
{
    transmit(pin, pulse_length, 1, 3);
    transmit(pin, pulse_length, 3, 1);
}

inline void sync(int pin, int pulse_length)
{
    transmit(pin, pulse_length, 1, 31);
}

#endif
