/*
  g++ -o foo foo.cpp ; ./foo > F; gnuplot
*/

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>

#define DT  (1. / 40000.)
// #define MODULO   (1 << 24)
#define MODULO   (1 << 16)

int sine_table[MODULO];

class Vox
{
private:
    int phase, dphase;
public:
    void init(void) {
        phase = 0;
    }
    void set_freq(double f) {
        // dphase = 2.0 * M_PI * f;
        dphase = f;
    }
    void step(void) {
        phase = (phase + dphase) & (MODULO - 1);
    }
    int sine(void) {
        return sine_table[phase];
    }
    int square(void) {
        if (phase < (MODULO << 1))
            return (int) (sqrt(0.5) * MODULO);
        else
            return -(int) (sqrt(0.5) * MODULO);
    }
    int ramp(void) {
        return (phase << 1) - MODULO;
    }
    int triangle(void) {
        if (phase < (MODULO << 1))
            return (phase << 2) - MODULO;
        else
            return -(phase << 2) + 3 * MODULO;
    }
};

int main(void)
{
    int i;
    for (i = 0; i < MODULO; i++)
        sine_table[i] = (int) (MODULO * sin(2 * M_PI * i / MODULO));
    // printf("hello, world\n");
    Vox v = Vox();
    v.set_freq(3);
    for (i = 0; i < MODULO; i++) {
        printf("%d %d %d\n", i, v.sine(), v.ramp());
        v.step();
    }
    return 0;
}
