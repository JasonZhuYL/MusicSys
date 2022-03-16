#ifndef KNOB_H
#define KNOB_H

class knob_decoder
{
private:
    uint8_t rotationValue;
    uint8_t previous;
    uint8_t upper_limit;
    uint8_t lower_limit;

public:
    knob_decoder(uint8_t initial_Val, uint8_t upper, uint8_t lower)
        : rotationValue(initial_Val), upper_limit(upper), lower_limit(lower)
    {
    }
    ~knob_decoder();
    void update(const uint8_t current);
    uint8_t get_val();
    void change_val(const uint8_t new_Val);
};

uint8_t knob_decoder::get_val()
{
    return rotationValue;
}

void knob_decoder::change_val(const uint8_t new_Val)
{
    self.roationValue = new_Val;
}

void knob_decoder::update(const uint8_t current)
{
    // The format of current should be 0b00000011
    // The previous would always be    0b00000011
    uint8_t standardised_current = current;
    if (self.previous > 1)
    {
        self.previous = ~(self.previous | 0xFC);
        standardised_current = ~(current | 0xFC);
    }

    switch (self.previous)
    {
    case 0:
        switch (standardised_current)
        {
        case 1:
            rotationValue += 1;
        case 2:
            rotationValue -= 1;
        case 3:
        default:
        }
    case 1:
        switch (standardised_current)
        {
        case 3:
            rotationValue += 1;
        case 0:
            rotationValue -= 1;
        case 2:
        default:
        }
    }
    if (rotationValue > upper_limit)
    {
        rotationValue = upper_limit;
    }
    // Currently using the uint8_t thus 255+1 = 0
    //  Lower limit function could not be implemented.
    //  if (rotationValue  upper_limit){
    //      rotationValue = upper_limit;
    //  }
}

#endif