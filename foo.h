#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>

#define SAMPLES_PER_SECOND   44100
#define DT  (1. / SAMPLES_PER_SECOND)

#define WAVE_MAX_BIT       24
#define TABLE_SIZE_BIT     13
#define PHASE_MODULO_BIT   30

#define WAVE_MAX       (1 << WAVE_MAX_BIT)
#define TABLE_SIZE     (1 << TABLE_SIZE_BIT)
#define PHASE_MODULO   (1 << PHASE_MODULO_BIT)

extern void _init_table(void);
extern int sine_table[8192]; // TABLE_SIZE
extern int sinusoid(int32_t **ptr, double freq, double phase,
                    double ampl1, double ampl2);

class Vox
{
private:
    int phase, dphase;
    int table_index() {
        return phase >> (PHASE_MODULO_BIT - TABLE_SIZE_BIT);
    }
public:
    Vox() {
        _init_table();
        phase = 0;
    }
    void set_freq(double f) {
        dphase = (int) (PHASE_MODULO * f / SAMPLES_PER_SECOND);
    }
    void step(void) {
        phase = (phase + dphase) & (PHASE_MODULO - 1);
    }
    int sine(void) {
        return sqrt(2.0) * sine_table[table_index()];
    }
    int square(void) {
        if (phase < (PHASE_MODULO >> 1))
            return (int) WAVE_MAX;
        else
            return -(int) WAVE_MAX;
    }
    int ramp(void) {
        return sqrt(3.0) * ((phase >> (PHASE_MODULO_BIT - WAVE_MAX_BIT - 1)) - WAVE_MAX);
    }
    int triangle(void) {
        if (phase < (PHASE_MODULO >> 1))
            return sqrt(3.0) * ((phase >> (PHASE_MODULO_BIT - WAVE_MAX_BIT - 2)) - WAVE_MAX);
        else
            return sqrt(3.0) * ((-phase >> (PHASE_MODULO_BIT - WAVE_MAX_BIT - 2)) + 3 * WAVE_MAX);
    }
};
