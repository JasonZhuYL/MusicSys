#ifndef KNOB_H
#define KNOB_H

class knob_decoder{
    private:
        volatile uint8_t previous
        volatile uint8_t rotationValue
    public:
        int knobDecode(volatile uint8_t current)
}


volatile uint8_t knobDecode(volatile uint8_t previous, volatile uint8_t current)
{
    // only two bits are used in each param
    if (previous > 1)
    {
        previous = ~(previous | 0xFC);
        current = ~(current | 0xFC);
    }
    switch (previous)
    {
    case 0:
        switch (current)
        {
        case 1:
            return 1;
        case 2:
            return -1;
        case 3:
            return 0;
        default:
            return 0;
        }
    case 1:
        switch (current)
        {
        case 3:
            return 1;
        case 0:
            return -1;
        case 2:
            return 0;
        default:
            return 0;
        }
    }
    return 0;

volatile uint8_t knobState(volatile uint8_t keyArray, int knobNumber){
    switch (knobNumber)
    {
    case 0:
        return ((keyArray & 0b00001100)) >> 2 | (knobState & 0b11111100)
        break;
    
    default:
        break;
    }
}

}



#endif