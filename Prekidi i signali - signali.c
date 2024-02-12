#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

struct timespec t;
struct timespec t0; /* vrijeme pocetka programa */
int k_z[3] = {0, 0, 0};
int t_p = 0;
int reg[3] = {-1, -1, -1};
int stog = 0;

/* postavlja trenutno vrijeme u t0 */
void postavi_pocetno_vrijeme()
{
	clock_gettime(CLOCK_REALTIME, &t0);
}

/* dohvaca vrijeme proteklo od pokretanja programa */
void vrijeme(void)
{

	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec -= t0.tv_sec;
	t.tv_nsec -= t0.tv_nsec;
	if (t.tv_nsec < 0)
	{
		t.tv_nsec += 1000000000;
		t.tv_sec--;
	}

	printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec / 1000000);
}

/* ispis kao i printf uz dodatak trenutnog vremena na pocetku */
#define PRINTF(format, ...)            \
	do                                 \
	{                                  \
		vrijeme();                     \
		printf(format, ##__VA_ARGS__); \
	} while (0)

void ispis_stanja()
{

	printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec / 1000000);
	printf("K_Z = %d%d%d, T_P = %d", k_z[0], k_z[1], k_z[2], t_p);
	printf(" stog:");
	for (int i = 2; i >= 0; i--)
		if (reg[i] != -1)
			printf("%d,reg[%d],", reg[i], reg[i]);
	printf("\n");
}

/*
 * spava zadani broj sekundi
 * ako se prekine signalom, kasnije nastavlja spavati neprospavano
 */
void spavaj(time_t sekundi, int i)
{
	struct timespec koliko;
	koliko.tv_sec = sekundi;
	koliko.tv_nsec = 0;

	while (nanosleep(&koliko, &koliko) == -1 && errno == EINTR)
	{
		if (i == 0)
			PRINTF("Nastavlja se izvođenje glavnog programa\n");
		else
			PRINTF("Nastavlja se izvođenje prekida razine %d\n", i);
		ispis_stanja();
	}
}

void obradi_sigint()
{
	reg[2] = t_p;
	t_p = 3;
	k_z[2] = 0;

	PRINTF("SIGINT: zapocinjem obradu prekida razine 3\n");
	ispis_stanja();
	spavaj(20, 3);
	PRINTF("SIGINT: obrada prekida razine 3 gotova\n");
	t_p = reg[2];
	reg[2] = -1;
}

void obradi_sigusr1()
{

	while (t_p > 2)
	{
	};
	reg[1] = t_p;
	t_p = 2;
	k_z[1] = 0;
	PRINTF("SIGUSR1: zapocinjem obradu prekida razine 2\n");
	ispis_stanja();
	spavaj(20, 2);
	PRINTF("SIGUSR1: obrada prekida razine 2 gotova\n");
	t_p = reg[1];
	reg[1] = -1;
}

void obradi_sigterm()
{
	while (t_p > 1)
	{
	};
	reg[0] = t_p;
	t_p = 1;
	k_z[0] = 0;

	PRINTF("SIGTERM: zapocinjem obradu prekida razine 1\n");
	ispis_stanja();
	spavaj(20, 1);
	PRINTF("SIGTERM: obrada prekida razine 1 gotova\n");
	t_p = reg[0];
	reg[0] = -1;
}

void obrada_signala(int sig)
{
	if (sig == SIGINT)
	{ // primljen sig raz 3
		k_z[2] = 1;
		if (t_p < 3)
		{
			PRINTF("SKLOP: primljen je signal razine 3 i prosljeđuje se procesoru\n");
			ispis_stanja();
			obradi_sigint();

			if (k_z[1] != 0)
			{
				obradi_sigusr1();
			}
			if (k_z[0] != 0)
			{
				obradi_sigterm();
			}
		}
	}
	else if (sig == SIGUSR1)
	{
		k_z[1] = 1;
		if (t_p < 2)
		{
			PRINTF("SKLOP:primljen je signal razine 2 i prosljeđuje se procesoru\n");
		}
		else
		{
			PRINTF("SKLOP:primljen je signal razine 2, ali se on pamti i ne prosljeđuje procesoru\n");
		}
		ispis_stanja();

		if (t_p < 2)
		{
			obradi_sigusr1();
		}
		if (k_z[0] != 0)
		{
			obradi_sigterm();
		}
	}
	else if (sig == SIGTERM)
	{

		k_z[0] = 1;
		if (t_p < 0)
		{
			PRINTF("SKLOP:primljen je signal razine 1 i prosljeđuje se procesoru\n");
		}
		else
		{
			PRINTF("SKLOP:primljen je signal razine 1, ali se on pamti i ne prosljeđuje procesoru\n");
		}
		ispis_stanja();

		if (t_p < 1)
		{
			obradi_sigterm();
		}
	}
}

void inicijalizacija()
{
	struct sigaction act;

	act.sa_handler = obrada_signala;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR1, &act, NULL); /* maskiranje signala preko sučelja OS-a */
	sigaction(SIGTERM, &act, NULL);

	postavi_pocetno_vrijeme();
	PRINTF("Program s PID=%ld krenuo s radom\n", (long)getpid());
}

int main()
{
	inicijalizacija();
	ispis_stanja();
	spavaj(100, 0);
	PRINTF("G: Kraj glavnog programa\n");

	return 0;
}