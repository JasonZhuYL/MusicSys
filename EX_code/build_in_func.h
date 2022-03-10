volatile uint32_t Serialkeypressed[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
// build-in functions; do not modify
const char *state_to_note(uint8_t id)
{
    switch (id)
    {
    case 0:
        return "None";
        break;
    case 1:
        return "C";
        break;
    case 2:
        return "C#";
        break;
    case 3:
        return "D";
        break;
    case 4:
        return "D#";
        break;
    case 5:
        return "E";
        break;
    case 6:
        return "F";
        break;
    case 7:
        return "F#";
        break;
    case 8:
        return "G";
        break;
    case 9:
        return "G#";
        break;
    case 10:
        return "A";
        break;
    case 11:
        return "A#";
        break;
    case 12:
        return "B";
        break;
    }
}
int char_to_int(char id)
{
    switch (id)
    {
    case '0':
        return 0;
        break;
    case '1':
        return 1;
        break;
    case '2':
        return 2;
        break;
    case '3':
        return 3;
        break;
    case '4':
        return 4;
        break;
    case '5':
        return 5;
        break;
    case '6':
        return 6;
        break;
    case '7':
        return 7;
        break;
    case '8':
        return 8;
        break;
    case '9':
        return 9;
        break;
    case 'A':
        return 10;
        break;
    case 'B':
        return 11;
        break;
    }
}
char int_to_char(int id)
{
    switch (id)
    {
    case 0:
        return '0';
        break;
    case 1:
        return '1';
        break;
    case 2:
        return '2';
        break;
    case 3:
        return '3';
        break;
    case 4:
        return '4';
        break;
    case 5:
        return '5';
        break;
    case 6:
        return '6';
        break;
    case 7:
        return '7';
        break;
    case 8:
        return '8';
        break;
    case 9:
        return '9';
        break;
    }
}

void setRow(uint8_t rowIdx)
{
    digitalWrite(A5, LOW);
    switch (rowIdx)
    {
    case 0:
        digitalWrite(D3, LOW);
        digitalWrite(D6, LOW);
        digitalWrite(D12, LOW);
        break;
    case 1:
        digitalWrite(D3, HIGH);
        digitalWrite(D6, LOW);
        digitalWrite(D12, LOW);
        break;
    case 2:
        digitalWrite(D3, LOW);
        digitalWrite(D6, HIGH);
        digitalWrite(D12, LOW);
        break;
    case 3:
        digitalWrite(D3, HIGH);
        digitalWrite(D6, HIGH);
        digitalWrite(D12, LOW);
        break;
    case 4:
        digitalWrite(D3, LOW);
        digitalWrite(D6, LOW);
        digitalWrite(D12, HIGH);
        break;
    case 5:
        digitalWrite(D3, HIGH);
        digitalWrite(D6, LOW);
        digitalWrite(D12, HIGH);
        break;
    case 6:
        digitalWrite(D3, LOW);
        digitalWrite(D6, HIGH);
        digitalWrite(D12, HIGH);
        break;
    case 7:
        digitalWrite(D3, HIGH);
        digitalWrite(D6, HIGH);
        digitalWrite(D12, HIGH);
        break;
    }
    digitalWrite(A5, HIGH);
}

uint8_t readCols()
{
    uint8_t C0 = digitalRead(A2);
    uint8_t C1 = digitalRead(D9);
    uint8_t C2 = digitalRead(A6);
    uint8_t C3 = digitalRead(D1);
    return (C0 << 3) | (C1 << 2) | (C2 << 1) | C3;
}

void keypressupdate(uint32_t *keypressed, const uint8_t key, const int i)
{
    if (key == 15)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 14)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 13)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 12)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 11)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 10)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 9)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 8)
    {
        __atomic_store_n(&keypressed[4 * i], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 7)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 6)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 5)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 4)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 3)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 2)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 1, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
    else if (key == 1)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 1, __ATOMIC_RELAXED);
    }
    else if (key == 0)
    {
        __atomic_store_n(&keypressed[4 * i], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 1], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 2], 0, __ATOMIC_RELAXED);
        __atomic_store_n(&keypressed[4 * i + 3], 0, __ATOMIC_RELAXED);
    }
}

void play_Mary_has_a_little_lamb()
{
    for (int i = 0; i < 25; i++)
    {
        if (i == 6 || i == 9 || i == 12 || i == 19 || i == 24)
        { // longest
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(450000);
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(150000);
        }
        else if (i == 0 || i == 13)
        {
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(330000);
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(120000);
        }
        else if (i == 1 || i == 14)
        {
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(100000);
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(50000);
        }
        else
        {
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(200000);
            __atomic_store_n(&Serialkeypressed[Mary_has_a_little_lamb[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(100000);
        }
    }
}

void play_Twinkle_star()
{
    for (int i = 0; i < 42; i++)
    {
        if ((i + 1) % 7 == 0)
        {
            __atomic_store_n(&Serialkeypressed[Twinkle_star[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(450000);
            __atomic_store_n(&Serialkeypressed[Twinkle_star[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(150000);
        }
        else
        {
            __atomic_store_n(&Serialkeypressed[Twinkle_star[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(200000);
            __atomic_store_n(&Serialkeypressed[Twinkle_star[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(100000);
        }
    }
}
void play_Ode_an_die_Freude()
{
    for (int i = 0; i < 62; i++)
    {
        if (i == 14 || i == 29 || i == 47)
        { // longest
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(450000);
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(150000);
        }
        else if (i == 12 || i == 27 || i == 59)
        {
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(330000);
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(120000);
        }
        else if (i == 13 || i == 28 || i == 35 || i == 36 || i == 40 || i == 41 || i == 60)
        {
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(100000);
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(50000);
        }
        else
        {
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 0, __ATOMIC_RELAXED);
            delayMicroseconds(225000);
            __atomic_store_n(&Serialkeypressed[Ode_an_die_Freude[i]], 1, __ATOMIC_RELAXED);
            delayMicroseconds(75000);
        }
    }
}
void play_London_Bridge_Is_Falling_Down()
{
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[0]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[0]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(330000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[0]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(120000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[1]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(100000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[1]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(50000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[2]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[2]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[3]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[3]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[4]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[4]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[5]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[5]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[6]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[6]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[0]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);

    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[1]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[7]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[7]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[8]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[8]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[9]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[9]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[1]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);

    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[2]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[10]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[10]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[11]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[11]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[12]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[12]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[2]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);

    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[3]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[13]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(330000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[13]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(120000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[14]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(100000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[14]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(50000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[15]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[15]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[16]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[16]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[17]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[17]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[18]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[18]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[19]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[19]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[3]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);

    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[4]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[20]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[20]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[21]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(450000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[21]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[4]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(150000);

    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[5]], 0, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[22]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[22]], 1, __ATOMIC_RELAXED);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Right[5]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[23]], 0, __ATOMIC_RELAXED);
    delayMicroseconds(225000);
    __atomic_store_n(&Serialkeypressed[London_Bridge_Is_Falling_Down_Left[23]], 1, __ATOMIC_RELAXED);
    delayMicroseconds(75000);
}