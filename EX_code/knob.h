#ifndef KNOB_H
#define KNOB_H
class knob_decoder
{ // all public methods behave as reenterant functions
private:
    int rotation_value;
    int upper_limit;
    int lower_limit;

public:
    knob_decoder(int rotation, int upper, int lower)
        : rotation_value(rotation), upper_limit(upper), lower_limit(lower)
    {
    }
    ~knob_decoder();
    void update(const int previous_state_ab, const int current_state_ab);
    int get_rotation();
    void change_rotation(int changed_value);
    void increase_joy();
    void decrease_joy();
};
void knob_decoder::update(const int previous_state_ab, const int current_state_ab)
{
    if (previous_state_ab == 0)
    {
        if (current_state_ab == 1 && rotation_value < upper_limit)
        {
            rotation_value++;
            return;
        }
        else if (current_state_ab == 2 && rotation_value > lower_limit)
        {
            rotation_value--;
            return;
        }
        else if (current_state_ab == 3 && rotation_value < upper_limit)
        {
            rotation_value += 2;
            return;
        }
        else
        {
            return;
        }
    }
    else if (previous_state_ab == 1)
    {
        if (current_state_ab == 0 && rotation_value > lower_limit)
        {
            rotation_value--;
            return;
        }
        else if (current_state_ab == 3 && rotation_value < upper_limit)
        {
            rotation_value++;
            return;
        }
        else
        {
            return;
        }
    }
    else if (previous_state_ab == 2)
    {
        if (current_state_ab == 0 && rotation_value < upper_limit)
        {
            rotation_value++;
            return;
        }
        else if (current_state_ab == 3 && rotation_value > lower_limit)
        {
            rotation_value--;
            return;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (current_state_ab == 1 && rotation_value > lower_limit)
        {
            rotation_value--;
            return;
        }
        else if (current_state_ab == 2 && rotation_value < upper_limit)
        {
            rotation_value++;
            return;
        }
        else if (current_state_ab == 0 && rotation_value > lower_limit + 1)
        {
            rotation_value -= 2;
            return;
        }
        else
        {
            return;
        }
    }
}
int knob_decoder::get_rotation()
{
    return rotation_value;
}
void knob_decoder::change_rotation(int changed_value)
{
    rotation_value = changed_value;
}
void knob_decoder::increase_joy()
{
    if (rotation_value < upper_limit)
        rotation_value++;
}
void knob_decoder::decrease_joy()
{
    if (rotation_value > lower_limit)
        rotation_value--;
}
knob_decoder::~knob_decoder() {}
#endif
