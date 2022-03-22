#ifndef KNOB_H
#define KNOB_H
#include <Arduino.h>

class knob_decoder
{
private:
    uint8_t rotationValue;
    uint8_t previous;
    uint8_t upper_limit;
    uint8_t lower_limit;

public:
    knob_decoder(uint8_t initial_Val, uint8_t upper, uint8_t lower)
        : rotationValue(initial_Val), upper_limit(upper), lower_limit(lower),previous(0)
    {
    }
    ~knob_decoder();
    void update(const uint8_t current);
    uint8_t get_val();
    void change_val(const uint8_t new_Val);
    void change_pre(const uint8_t pre);
    uint8_t get_pre();

};

uint8_t knob_decoder::get_pre(){
    return previous;
};

void knob_decoder::change_pre(const uint8_t pre){
    previous = pre;
}

uint8_t knob_decoder::get_val()
{
    return rotationValue;
}

void knob_decoder::change_val(const uint8_t new_Val)
{
    rotationValue = new_Val;
}

void knob_decoder::update(const uint8_t current)
{
    // The format of current should be 0b00000011
    // The previous would always be    0b00000011
    uint8_t standardised_current = current;
    uint8_t standardised_previous = previous;
    if (previous > 1)
    {
        standardised_previous = ~(previous | 0xFC);
        standardised_current = ~(current | 0xFC);
    }

    if (standardised_previous==0){
        if(standardised_current==1){
            rotationValue += 1;
        }else if(standardised_current == 2){
            rotationValue -= 1;
        }
        else{}
    }else{
        if (standardised_current ==3){
            rotationValue += 1;
        }else if(standardised_current ==0){
            rotationValue -= 1;
        }else {}
    }
    if (rotationValue > upper_limit)
    {
        rotationValue = upper_limit;
    }
    previous = current;
}

#endif