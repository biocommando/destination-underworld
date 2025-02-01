unsigned synth_random()
{
    static unsigned state = 123;
    state = (state >> 5) ^ state;
    state = (state << 7) ^ state;
    return state;
}