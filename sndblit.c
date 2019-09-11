/***************************************************************
NAME
       sndblit - sound blitter for making electronic music

SYNOPSIS
       sndblit [-d] [-o outputfile] [-s samplingfrequency]

DESCRIPTION
       sndblit  accepts  blip descriptors and outputs a RIFF WAVE
       (.wav) file. A "blip" is an instantaneous snapshot  of  an
       audio  spectrum. Like the persistence of vision that makes
       movies work, the human hearing system perceives continuous
       sound when you hear a sound generated by updating an audio
       spectrum about 30 times per second.  sndblit exploits this
       persistence of hearing.

       The  input to sndblit is a blip descriptor file. The first
       line is a floating pointer number, the blip duration,  and
       an  int,  the  total  number of blips in the file. This is
       followed by one descriptor for each blip.

       Each blip descriptor starts with one line, the  number  of
       sinusoids  in this blip. For each sinusoid there is a line
       with five floats, the frequency, left channel cosine  com-
       ponent,  left channel sine component, right channel cosine
       component, and right channel sine component.

OPTIONS
       -i, --input-file
              Specifies an input  file.  If  not  provided,  blip
              descriptors are read from standard input.

       -o, --output-file
              Specifies  an  output  file.  If  not provided, the
              default output file is sndblit_output.wav.

       -v, --verbose
              Run in verbose mode, printing information about the
              sinusoids within each blip.

       -s, --sampling-frequency
              Specifies  a  new  sampling  frequency. The default
              sampling frequency is 44100 Hertz, audio  CD  qual-
              ity.

       -h, -?, --help
              Print helpful(?) usage information.

AUTHOR
       Will  Ware <wware@alum.mit.edu> has placed this program in
       the public domain.
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

#define ASSERT(c) if (!(c)) \
  { fprintf(stderr,"Assertion failed at line %d:\n%s\n", __LINE__, #c); exit(1); }

struct sinusoid {
	double frequency;
	int left_cosine, right_cosine, left_sine, right_sine;
};

struct blip {
	int num_sinusoids;
	struct sinusoid waves[1];	/* growable via malloc */
};

#ifndef M_PI
#define M_PI 3.141592653589793238462
#endif

#define CYCLE 8192		/* this should be a power of 2 */
#define FRACTION 0x1000		/* used for fixed-point math */

static int verbose_flag = 0;

static FILE *infile;
static char outfile_name[80], left_data_name[80], right_data_name[80];
static int num_blips, blipsize, max_value;
static int *leftbuf, *rightbuf;
static double blip_duration, sampling_frequency = 44100.0, t, dt;
static struct blip *current_blip = NULL;
static struct blip *previous_blip = NULL;
static int cos_table[CYCLE], sin_table[CYCLE];

#define GET1(fmt,x)          _get_line(1, fmt, x, NULL, NULL, NULL, NULL)
#define GET2(fmt,x,y)        _get_line(2, fmt, x, y, NULL, NULL, NULL)
#define GET3(fmt,x,y,z)      _get_line(3, fmt, x, y, z, NULL, NULL)
#define GET4(fmt,x,y,z,u)    _get_line(4, fmt, x, y, z, u, NULL)
#define GET5(fmt,x,y,z,u,v)  _get_line(5, fmt, x, y, z, u, v)

static void _get_line(int expected, char *format,
		    void *var1, void *var2, void *var3,
		    void *var4, void *var5)
{
	static int input_line_number = 1;
	static char input_line[2001];
	if (fgets(input_line, 2000, infile) == NULL) {
		fprintf(stderr, "Premature end of input at line %d\n",
			input_line_number);
		exit(1);
	}
	input_line[strlen(input_line) - 1] = '\0';
	if (verbose_flag)
		printf("Line %d: \"%s\"\n", input_line_number, input_line);
	if (sscanf(input_line, format, var1, var2, var3, var4, var5)
	    != expected) {
		fprintf(stderr, "Garbage in input at line %d\n",
			input_line_number);
		exit(1);
	}
	input_line_number++;
}

/* Generate stereo .WAV files with 16-bit data. */
#define NCHANNELS 2
#define SAMPWIDTH 2
#define WAVE_FORMAT_PCM 1

static void put2(int x, FILE * outf)
{
	putc(x & 0xFF, outf);
	putc((x >> 8) & 0xFF, outf);
}

static void put4(int x, FILE * outf)
{
	putc(x & 0xFF, outf);
	putc((x >> 8) & 0xFF, outf);
	putc((x >> 16) & 0xFF, outf);
	putc((x >> 24) & 0xFF, outf);
}

static void wave_file_header(FILE * outf)
{
	int datasize;
	datasize = num_blips * blipsize * NCHANNELS * SAMPWIDTH;
	fprintf(outf, "RIFF");
	put4(datasize + 36, outf);
	fprintf(outf, "WAVEfmt ");
	put4(16, outf);
	put2(WAVE_FORMAT_PCM, outf);
	put2(NCHANNELS, outf);
	put4((int) sampling_frequency, outf);
	put4((int) (sampling_frequency * NCHANNELS * SAMPWIDTH), outf);
	put2(NCHANNELS * SAMPWIDTH, outf);
	put2(8 * SAMPWIDTH, outf);
	fprintf(outf, "data");
	put4(datasize, outf);
}

static int compare_frequencies(const void *arg1, const void *arg2)
{
	struct sinusoid *s1, *s2;
	s1 = (struct sinusoid *) arg1;
	s2 = (struct sinusoid *) arg2;
	if (s1->frequency == s2->frequency)
		return 0;
	if (s1->frequency < s2->frequency)
		return -1;
	return 1;
}


#define TESTMALLOC(x) \
  if ((x) == NULL) { \
    fprintf(stderr, "Line %d: out of memory\n", __LINE__); \
    exit(1); }

static void read_in_blip(void)
{
	int j, num_sinusoids;
	double lc, ls, rc, rs;
	GET1("%d", &num_sinusoids);
	if (verbose_flag)
		printf("reading blip, %d sinusoids\n", num_sinusoids);
	current_blip = malloc(sizeof(struct blip) +
			      (num_sinusoids -
			       1) * sizeof(struct sinusoid));
	TESTMALLOC(current_blip);
	current_blip->num_sinusoids = num_sinusoids;
	for (j = 0; j < num_sinusoids; j++) {
		GET5("%lf %lf %lf %lf %lf",
		     &(current_blip->waves[j].frequency),
		     &lc, &ls, &rc, &rs);
		current_blip->waves[j].left_cosine = (int) (FRACTION * lc);
		current_blip->waves[j].right_cosine =
		    (int) (FRACTION * rc);
		current_blip->waves[j].left_sine = (int) (FRACTION * ls);
		current_blip->waves[j].right_sine = (int) (FRACTION * rs);
		if (verbose_flag)
			printf("f=%f lc=%d rc=%d ls=%d rs=%d\n",
			       current_blip->waves[j].frequency,
			       current_blip->waves[j].left_cosine,
			       current_blip->waves[j].right_cosine,
			       current_blip->waves[j].left_sine,
			       current_blip->waves[j].right_sine);
	}
	qsort(current_blip->waves, num_sinusoids,
	      sizeof(struct sinusoid), compare_frequencies);
}

static void render_sinusoid(int *buf, double f,
			    int cos_start, int cos_finish,
			    int sin_start, int sin_finish)
{
	int i;
	int c0, c1, dc0, dc1;
	int s0, s1, ds0, ds1;
	int ph1, ph0, dph1, dph0;

	{
		double cos_delta =
		    ((double) (cos_finish - cos_start)) / blipsize;
		double sin_delta =
		    ((double) (sin_finish - sin_start)) / blipsize;
		c1 = cos_start;
		c0 = 0;
		dc1 = (int) cos_delta;
		dc0 = (int) (FRACTION * (cos_delta - dc1));
		s1 = sin_start;
		s0 = 0;
		ds1 = (int) sin_delta;
		ds0 = (int) (FRACTION * (sin_delta - ds1));
	}

	{
		double phase, phase_delta;
		phase = CYCLE * f * t;
		phase_delta = CYCLE * f * blip_duration / blipsize;
		ph1 = (int) phase;
		ph0 = (int) (FRACTION * (phase - ph1));
		dph1 = (int) phase_delta;
		dph0 = (int) (FRACTION * (phase_delta - dph1));
	}

	for (i = 0; i < blipsize; i++) {
		buf[i] +=
		    c1 * cos_table[ph1 & (CYCLE - 1)] +
		    s1 * sin_table[ph1 & (CYCLE - 1)];

		c0 += dc0;
		c1 += dc1;
		while (c0 >= FRACTION) {
			c1++;
			c0 -= FRACTION;
		}
		while (c0 < -FRACTION) {
			c1--;
			c0 += FRACTION;
		}

		s0 += ds0;
		s1 += ds1;
		while (s0 >= FRACTION) {
			s1++;
			s0 -= FRACTION;
		}
		while (s0 < -FRACTION) {
			s1--;
			s0 += FRACTION;
		}

		ph0 += dph0;
		ph1 += dph1;
		while (ph0 >= FRACTION) {
			ph1++;
			ph0 -= FRACTION;
		}
		while (ph0 < -FRACTION) {
			ph1--;
			ph0 += FRACTION;
		}
	}
}

static void merge_with_previous_blip(void)
{
	int i, j, b_abs;
	i = j = 0;
	while (1) {
		if (i < current_blip->num_sinusoids &&
		    j < previous_blip->num_sinusoids &&
		    current_blip->waves[i].frequency ==
		    previous_blip->waves[j].frequency) {
			/* this is the same sinusoid spanning two blips */
			if (verbose_flag)
				printf
				    ("continue existing sinusoid, f=%f\n",
				     current_blip->waves[i].frequency);
			render_sinusoid(leftbuf,
					current_blip->waves[i].frequency,
					previous_blip->waves[j].
					left_cosine,
					current_blip->waves[i].left_cosine,
					previous_blip->waves[j].left_sine,
					current_blip->waves[i].left_sine);
			render_sinusoid(rightbuf,
					current_blip->waves[i].frequency,
					previous_blip->waves[j].
					right_cosine,
					current_blip->waves[i].
					right_cosine,
					previous_blip->waves[j].right_sine,
					current_blip->waves[i].right_sine);
			i++;
			j++;
		} else if (i < current_blip->num_sinusoids &&
			   (j == previous_blip->num_sinusoids ||
			    current_blip->waves[i].frequency <
			    previous_blip->waves[j].frequency)) {
			/* this is the beginning of a new sinusoid */
			if (verbose_flag)
				printf("begin new sinusoid, f=%f\n",
				       current_blip->waves[i].frequency);
			render_sinusoid(leftbuf,
					current_blip->waves[i].frequency,
					0,
					current_blip->waves[i].left_cosine,
					0,
					current_blip->waves[i].left_sine);
			render_sinusoid(rightbuf,
					current_blip->waves[i].frequency,
					0,
					current_blip->waves[i].
					right_cosine, 0,
					current_blip->waves[i].right_sine);
			i++;
		} else if (j < previous_blip->num_sinusoids &&
			   (i == current_blip->num_sinusoids ||
			    previous_blip->waves[j].frequency <
			    current_blip->waves[i].frequency)) {
			/* this is the end of an old sinusoid */
			if (verbose_flag)
				printf("end old sinusoid, f=%f\n",
				       previous_blip->waves[j].frequency);
			render_sinusoid(leftbuf,
					previous_blip->waves[j].frequency,
					previous_blip->waves[j].
					left_cosine, 0,
					previous_blip->waves[j].left_sine,
					0);
			render_sinusoid(rightbuf,
					previous_blip->waves[j].frequency,
					previous_blip->waves[j].
					right_cosine, 0,
					previous_blip->waves[j].right_sine,
					0);
			j++;
		} else
			break;
	}
	for (i = b_abs = 0; i < blipsize; i++) {
		if (leftbuf[i] < -b_abs)
			b_abs = -leftbuf[i];
		else if (leftbuf[i] > b_abs)
			b_abs = leftbuf[i];
		if (rightbuf[i] < -b_abs)
			b_abs = -rightbuf[i];
		else if (rightbuf[i] > b_abs)
			b_abs = rightbuf[i];
	}
	if (max_value < b_abs)
		max_value = b_abs;
	if (verbose_flag)
		printf("max value = %08x\n", max_value);
}

static void help(void)
{
	printf("NAME\n"
	       "       sndblit - sound blitter for making electronic music\n"
	       "\n"
	       "SYNOPSIS\n"
	       "       sndblit [-d] [-o outputfile] [-s samplingfrequency]\n"
	       "\n"
	       "DESCRIPTION\n"
	       "       sndblit  accepts  blip descriptors and outputs a RIFF WAVE\n"
	       "       (.wav) file. A \"blip\" is an instantaneous snapshot  of  an\n"
	       "       audio  spectrum. Like the persistence of vision that makes\n"
	       "       movies work, the human hearing system perceives continuous\n"
	       "       sound when you hear a sound generated by updating an audio\n"
	       "       spectrum about 30 times per second.  sndblit exploits this\n"
	       "       persistence of hearing.\n"
	       "\n"
	       "       The  input to sndblit is a blip descriptor file. The first\n"
	       "       line is a floating pointer number, the blip duration,  and\n"
	       "       an  int,  the  total  number of blips in the file. This is\n"
	       "       followed by one descriptor for each blip.\n"
	       "\n"
	       "       Each blip descriptor starts with one line, the  number  of\n"
	       "       sinusoids  in this blip. For each sinusoid there is a line\n"
	       "       with five floats, the frequency, left channel cosine  com-\n"
	       "       ponent,  left channel sine component, right channel cosine\n"
	       "       component, and right channel sine component.\n"
	       "\n"
	       "OPTIONS\n"
	       "       -i, --input-file\n"
	       "              Specifies an input  file.  If  not  provided,  blip\n"
	       "              descriptors are read from standard input.\n"
	       "\n"
	       "       -o, --output-file\n"
	       "              Specifies  an  output  file.  If  not provided, the\n"
	       "              default output file is sndblit_output.wav.\n"
	       "\n"
	       "       -v, --verbose\n"
	       "              Run in verbose mode, printing information about the\n"
	       "              sinusoids within each blip.\n"
	       "\n"
	       "       -s, --sampling-frequency\n"
	       "              Specifies  a  new  sampling  frequency. The default\n"
	       "              sampling frequency is 44100 Hertz, audio  CD  qual­\n"
	       "              ity.\n"
	       "\n"
	       "       -h, -?, --help\n"
	       "              Print helpful(?) usage information.\n"
	       "\n"
	       "AUTHOR\n"
	       "       Will  Ware <wware@alum.mit.edu> has placed this program in\n"
	       "       the public domain.\n");
}


static const struct option long_options[] = {
	{"verbose", no_argument, NULL, 'v'},
	{"input-file", required_argument, NULL, 'i'},
	{"output-file", required_argument, NULL, 'o'},
	{"left-data", optional_argument, NULL, 'L'},
	{"right-data", optional_argument, NULL, 'R'},
	{"sampling-frequency", required_argument, NULL, 's'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};

int main(int argc, char *argv[])
{
	FILE *outf, *leftf, *rightf;
	int c, i, j, k, mult;
	unsigned char *outbuf;
	static struct blip *zero_blip;

	infile = stdin;
	strcpy(outfile_name, "sndblit_output.wav");
	left_data_name[0] = '\0';
	right_data_name[0] = '\0';
	while ((c = getopt_long(argc, argv, "s:i:o:L:R:vh?",
				long_options, (int *) 0)) != EOF)
		switch (c) {
		case 's':
			sampling_frequency = atof(optarg);
			break;
		case 'i':
			infile = fopen(optarg, "r");
			if (infile == NULL) {
				fprintf(stderr,
					"cannot open \"%s\" for reading\n",
					optarg);
				return 1;
			}
			break;
		case 'o':
			strcpy(outfile_name, optarg);
			break;
		case 'L':
			strcpy(left_data_name, optarg);
			break;
		case 'R':
			strcpy(right_data_name, optarg);
			break;
		case 'v':
			verbose_flag = 1;
			break;
		default:
			help();
			return 0;
		}
	for (i = 0; i < CYCLE; i++) {
		double angle = (2.0 * M_PI / CYCLE) * i;
		cos_table[i] = (int) (FRACTION * cos(angle));
		sin_table[i] = (int) (FRACTION * sin(angle));
	}

	GET2("%lf %d", &blip_duration, &num_blips);
	num_blips += 2;		/* we'll add extra blips at beginning and end */
	dt = 1. / sampling_frequency;
	blipsize = (int) (blip_duration / dt);
	/* At the cost of a slight inaccuracy in tempo but with a vast
	 * simplification of the code, coerce blip_duration to be an exact
	 * multiple of dt.
	 */
	blip_duration = dt * blipsize;
	leftbuf = malloc(blipsize * sizeof(int));
	TESTMALLOC(leftbuf);
	rightbuf = malloc(blipsize * sizeof(int));
	TESTMALLOC(rightbuf);

	/* make a blip with zero amplitudes */
	zero_blip = malloc(sizeof(struct blip) - sizeof(struct sinusoid));
	TESTMALLOC(zero_blip);
	zero_blip->num_sinusoids = 0;
	current_blip = zero_blip;

	if (left_data_name[0] == '\0')
		leftf = fopen("left.tmp", "wb");
	else
		leftf = fopen(left_data_name, "wb");
	if (leftf == NULL) {
		fprintf(stderr, "can't open left.tmp for writing\n");
		return 1;
	}
	if (right_data_name[0] == '\0')
		rightf = fopen("right.tmp", "wb");
	else
		rightf = fopen(right_data_name, "wb");
	if (rightf == NULL) {
		fprintf(stderr, "can't open right.tmp for reading\n");
		return 1;
	}
	max_value = 0;

	/* step thru all the blips in the input */
	for (i = 0, t = 0.0; i < num_blips; i++, t += blipsize * dt) {
		if (verbose_flag)
			printf("starting loop for blip %d\n", i - 1);
		memset(leftbuf, 0, blipsize * sizeof(int));
		memset(rightbuf, 0, blipsize * sizeof(int));
		if (previous_blip != NULL && previous_blip != zero_blip)
			free(previous_blip);
		previous_blip = current_blip;
		if (i == 0 || i == num_blips - 1)
			/* at the beginning, ramp up from the zero blip */
			/* at the end, ramp down to the zero blip */
			current_blip = zero_blip;
		else
			read_in_blip();
		merge_with_previous_blip();
		if (left_data_name[0] || right_data_name[0]) {
			for (j = 0; j < blipsize; j++) {
				fprintf(leftf, "%d %d\n", j + i * blipsize, leftbuf[j]);
				fprintf(rightf, "%d %d\n", j + i * blipsize, rightbuf[j]);
			}
		} else {
			fwrite(leftbuf, sizeof(int), blipsize, leftf);
			fwrite(rightbuf, sizeof(int), blipsize, rightf);
		}
		if (verbose_flag)
			printf("finishing loop for blip %d\n", i - 1);
	}
	fclose(leftf);
	fclose(rightf);
	fclose(infile);
	if (previous_blip != NULL && previous_blip != zero_blip)
		free(previous_blip);
	free(zero_blip);
	mult = (int) ((32700.0 * 65536.0) / max_value);
	if (verbose_flag)
		printf("max_value = %d, mult = %d\n", max_value, mult);

	outf = fopen(outfile_name, "wb");
	if (outf == NULL) {
		fprintf(stderr, "can't open \"%s\" for writing\n",
			outfile_name);
		return 1;
	}
	wave_file_header(outf);
	outbuf = malloc(4 * blipsize);
	TESTMALLOC(outbuf);
	if (left_data_name[0] == '\0')
		leftf = fopen("left.tmp", "rb");
	else
		leftf = fopen(left_data_name, "rb");
	if (leftf == NULL) {
		fprintf(stderr, "can't open left.tmp for reading\n");
		return 1;
	}
	if (right_data_name[0] == '\0')
		rightf = fopen("right.tmp", "rb");
	else
		rightf = fopen(right_data_name, "rb");
	if (rightf == NULL) {
		fprintf(stderr, "can't open right.tmp for reading\n");
		return 1;
	}
	for (i = 0; i < num_blips; i++) {
		if (left_data_name[0] || right_data_name[0]) {
			for (j = 0; j < blipsize; j++) {
				ASSERT(fscanf(leftf, "%d %d\n", &k, &leftbuf[j]) == 2);
				ASSERT(fscanf(rightf, "%d %d\n", &k, &rightbuf[j]) == 2);
			}
		} else {
			ASSERT(fread(leftbuf, sizeof(int), blipsize, leftf) ==
			       blipsize);
			ASSERT(fread(rightbuf, sizeof(int), blipsize, rightf) ==
			       blipsize);
		}
		for (j = k = 0; j < blipsize; j++) {
			int x, y;
			x = (mult * leftbuf[j]) >> 16;
			y = (mult * rightbuf[j]) >> 16;
			outbuf[k++] = x & 0xFF;
			outbuf[k++] = (x >> 8) & 0xFF;
			outbuf[k++] = y & 0xFF;
			outbuf[k++] = (y >> 8) & 0xFF;
		}
		ASSERT(k == 4 * blipsize);
		ASSERT(fwrite(outbuf, 4, blipsize, outf) == blipsize);
	}
	ASSERT(fclose(leftf) == 0);
	ASSERT(fclose(rightf) == 0);
	ASSERT(fclose(outf) == 0);
	free(leftbuf);
	free(rightbuf);
	free(outbuf);
	system("rm -f left.tmp right.tmp");
	return 0;
}
