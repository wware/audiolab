#include "foo.h"

int _ready = 0;
int sine_table[TABLE_SIZE];

void _init_table(void)
{
    int i;
    if (!_ready) {
        for (i = 0; i < TABLE_SIZE; i++)
            sine_table[i] = (int) (WAVE_MAX * sin(2 * M_PI * i / TABLE_SIZE));
        _ready = 1;
    }
}

int sinusoid(int *ptr, double freq, double phase,
             double ampl1, double ampl2)
{
    // fill in details later
    return 0;
}

int main(void)
{
    _init_table();
    return 0;
}
