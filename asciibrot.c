#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <signal.h>

struct
Options
{
	int iterations;
	double real, imag;
	double jreal, jimag;
	double zoom;
	double fontaspect;
	int width, height;
	int (*fun)(double, double, struct Options *, double *);
	char *charset, inchar;
	int animate;
	int bounce;
	double delay;
};

int
mandelbrot_loop(double x, double y, struct Options *o, double *v)
{
	int nmax = o->iterations, n;
	double re_c = x, im_c = y;
	double re_z = 0, im_z = 0;
	double re_z_now = 0, im_z_now = 0;
	double sqr_abs_z = 0;

	for (n = 0; n < nmax; n++)
	{
		re_z_now = re_z * re_z - im_z * im_z + re_c;
		im_z_now = 2 * re_z * im_z + im_c;
		re_z = re_z_now;
		im_z = im_z_now;
		sqr_abs_z = re_z * re_z + im_z * im_z;

		if (sqr_abs_z > 4)
		{
			*v = (double)n / nmax;
			return 0;
		}
	}

	return 1;
}

int
julia_loop(double x, double y, struct Options *o, double *v)
{
	int nmax = o->iterations, n;
	double re_c = o->jreal, im_c = o->jimag;
	double re_z = x, im_z = y;
	double re_z_now = 0, im_z_now = 0;
	double sqr_abs_z = 0;

	for (n = 0; n < nmax; n++)
	{
		re_z_now = re_z * re_z - im_z * im_z + re_c;
		im_z_now = 2 * re_z * im_z + im_c;
		re_z = re_z_now;
		im_z = im_z_now;
		sqr_abs_z = re_z * re_z + im_z * im_z;

		if (sqr_abs_z > 4)
		{
			*v = (double)n / nmax;
			return 0;
		}
	}

	return 1;
}

void
draw(struct Options *o)
{
	double x_add = o->real, y_add = o->imag;
	double x_mul = o->fontaspect * o->zoom;
	double y_mul = o->zoom;
	int width = o->width, height = o->height;
	double c = fmin(width * o->fontaspect, height);
	int x_term, y_term, is_in;
	double x, y;
	double v;
	size_t chari;

	for (y_term = 0; y_term < height; y_term++)
	{
		y = (double)(2 * y_term - height) / c;
		y *= y_mul;
		y -= y_add;

		for (x_term = 0; x_term < width; x_term++)
		{
			x = (double)(2 * x_term - width) / c;
			x *= x_mul;
			x += x_add;

			is_in = (*o->fun)(x, y, o, &v);
			if (is_in)
				printf("%c", o->inchar);
			else
			{
				chari = (int)(v * strlen(o->charset));
				if (chari >= strlen(o->charset))
					chari = strlen(o->charset) - 1;
				printf("%c", o->charset[chari]);
			}
		}

		if (y_term < height - 1)
			printf("\n");
	}
}

void
animate_end(int dummy)
{
	(void)dummy;

	char *cnorm = "\e[?12l\e[?25h";
	printf("%s\n", cnorm);
	exit(EXIT_SUCCESS);
}

void
animate(struct Options *o)
{
	int radius = rand() % 3600;
	int rot = rand() % 3600;
	int radius_add_choices[] = {-6, -4, -2, 2, 4, 6};
	int radius_add = radius_add_choices[rand() % 6];
	int rot_add_choices[] = {-6, -4, -2, 2, 4, 6};
	int rot_add = rot_add_choices[rand() % 6];
	double radius_span_choices[] = {0.5, 0.3};
	double radius_off_choices[] = {0.3, 0.7};
	int radius_choice = rand() % 2;
	double radius_span = radius_span_choices[radius_choice];
	double radius_off = radius_off_choices[radius_choice];;
	double radius_bounce, rot_rad, jx, jy;
	double zoom_d_bounce, zoom_r_rad;

	int zoom_d = rand() % 3600;
	int zoom_r = rand() % 36000;

	char *topleft = "\e[1;1H";
	char *civis = "\e[?25l";

	o->fun = julia_loop;
	signal(SIGINT, animate_end);

	printf("%s", civis);

	while (1)
	{
		printf("%s", topleft);

		radius_bounce = sin((radius / 10.0) * M_PI / 180) * radius_span + radius_off;
		rot_rad = (rot / 10.0) * M_PI / 180;

		jx = 0;
		jy = radius_bounce;

		o->jreal = jx * cos(rot_rad) - jy * sin(rot_rad);
		o->jimag = jx * sin(rot_rad) + jy * cos(rot_rad);

		if (o->bounce)
		{
			zoom_d_bounce = sin((zoom_d / 10.0) * M_PI / 180) * 0.25 + 1;
			zoom_r_rad = (zoom_r / 100.0) * M_PI / 180;
			o->real = 0.5 * sin(zoom_r_rad);
			o->imag = 0.5 * cos(zoom_r_rad);
			o->zoom = zoom_d_bounce;
		}

		draw(o);

		radius += radius_add;
		radius %= 3600;
		rot += rot_add;
		rot %= 3600;

		zoom_d += 15;
		zoom_d %= 3600;
		zoom_r += 50;
		zoom_r %= 36000;

		usleep(o->delay * 1e6);
	}
}

void
split(char *from, char delim, double *a, double *b)
{
	char *copy, *sec;
	copy = malloc(BUFSIZ);
	strncpy(copy, from, BUFSIZ);
	sec = strchr(copy, delim);
	if (copy == NULL)
	{
		fprintf(stderr, "Malformed argument.\n");
		exit(EXIT_FAILURE);
	}
	*sec = 0;
	sec++;
	*a = atof(copy);
	*b = atof(sec);
	free(copy);
}

void
spliti(char *from, char delim, int *a, int *b)
{
	double ta, tb;
	split(from, delim, &ta, &tb);
	*a = (int)ta;
	*b = (int)tb;
}

int
main(int argc, char **argv)
{
	int opt;
	struct winsize w;
	struct Options o;
	char *buf;

	usleep(0.25 * 1e6);
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	o.iterations = 15;
	o.real = 0;
	o.imag = 0;
	o.jreal = -0.46;
	o.jimag = 0.58;
	o.zoom = 1;
	o.fontaspect = 0.5;
	o.width = w.ws_col;
	o.height = w.ws_row;
	o.fun = mandelbrot_loop;
	o.charset = " .:-+=oO0#";
	o.inchar = '`';
	o.animate = 0;
	o.bounce = 1;
	o.delay = 0.03;

	while ((opt = getopt(argc, argv, "c:C:i:z:p:J:jf:abd:s:")) != -1)
	{
		switch (opt)
		{
			case 'c':
				buf = malloc(BUFSIZ);
				strncpy(buf, optarg, BUFSIZ);
				o.charset = buf;
				break;
			case 'C':
				o.inchar = optarg[0];
				break;
			case 'i':
				o.iterations = atoi(optarg);
				break;
			case 'z':
				o.zoom = atof(optarg);
				break;
			case 'p':
				split(optarg, ':', &o.real, &o.imag);
				break;
			case 'J':
				split(optarg, ':', &o.jreal, &o.jimag);
				break;
			case 'j':
				o.fun = julia_loop;
				break;
			case 'f':
				o.fontaspect = atof(optarg);
				break;
			case 'a':
				o.animate = 1;
				break;
			case 'b':
				o.bounce = 0;
				break;
			case 'd':
				o.delay = atof(optarg);
				break;
			case 's':
				spliti(optarg, 'x', &o.width, &o.height);
				break;
		}
	}

	srand(time(NULL));

	if (o.animate)
		animate(&o);
	else
		draw(&o);

	exit(EXIT_SUCCESS);
}
