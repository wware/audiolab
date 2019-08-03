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

int main(void)
{
    int i;
    Vox v = Vox();
    v.set_freq(3);
    for (i = 0; i < SAMPLES_PER_SECOND; i++) {
        printf("%d %d\n", i, v.sine());
        v.step();
    }
    return 0;
}
