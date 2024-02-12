#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_MIS 10
#define KAN 10
#define MIS 10
#define N 7

struct arg_struct
{
	int obala;
	char naz[3];
};

int C[2] = {0, 1};
char LO[100][3], DO[100][3], Ca[7][3];
char poruka[80];
int l0 = 0, d0 = 0, c0 = 0; // broj putnika na obalama koji čekaju ukrcaj
int o_mis[2] = {0, 0};
int o_kan[2] = {0, 0};
pthread_mutex_t m;
pthread_t dretva[32];
pthread_cond_t uvjet[2][2];
pthread_cond_t camac[3];
int mlc = 0, mdc = 0, klc = 0, kdc = 0; // # BROJEVI PUTNIKA koji čekaju na obali;
int mub = 0, kub = 0;					// broj putnika u brodu
int vrs[2] = {0, 1};
int br[2] = {0, 0};
int ceka[2] = {0, 0};
int siti[2] = {0, 0};
int c_obala = 1;

char slovo;

int c_mis = 0;
int c_kan = 0;

void ispis()
{
	int i;
	printf("%s\n", poruka);
	printf(" C[");

	if (c_obala == 0)
		slovo = 'L';

	else
		slovo = 'D';

	printf("%c] {", slovo);

	for (i = 0; i < c0; i++)
		printf(" %s", Ca[i]);

	printf("}");
	printf(" LO {");

	for (i = 0; i < l0; i++)
		printf(" %s", LO[i]);

	printf("}");
	printf(" DO {");

	for (i = 0; i < d0; i++)
		printf(" %s", DO[i]);

	printf("}");
	printf("\n");
}

void Camac()
{
	int i;
	strcpy(poruka, "C: Prazan na desnoj obali");
	ispis();

	while (1)
	{
		pthread_mutex_lock(&m);
		pthread_cond_wait(&camac[1], &m);
		strcpy(poruka, "C:tri putnika ukrcana, polazim za jednu sekundu");
		ispis();
		sleep(1);

		if (c_obala == 1)
		{
			printf("C: prevozim s desne na lijevu obalu");
		}

		else
			printf("C: prevozim s lijeve na desnu obalu");
		for (i = 0; i < c0; i++)
			printf(" %s ", Ca[i]);
		printf("\n");

		for (i = 0; i < c0; i++)
			strcpy(Ca[i], "");
		c0 = 0;

		sleep(1);

		c_obala = 1 - c_obala;

		pthread_cond_broadcast(&camac[2]);
		pthread_mutex_unlock(&m);
	}
}

void premj_ob_c(char *naz, int obala)
{
	int i, ind = 0;
	if (obala == 0)
	{
		for (i = 0; i < l0; i++)
			if (!strcmp(LO[i], naz))
				ind = i;
		strcpy(Ca[c0], LO[ind]);
		c0++;

		for (i = ind; i < l0 - 1; i++)
			strcpy(LO[i], LO[i + 1]);
		l0--;
	}

	else
	{
		for (i = 0; i < d0; i++)
			if (!strcmp(DO[i], naz))
			{
				ind = i;
			}

		strcpy(Ca[c0], DO[ind]);
		c0++;
		// printf ("%d\n",ind);
		for (i = ind; i < d0 - 1; i++)
			strcpy(DO[i], DO[i + 1]);
		d0--;
	}
}

void Misionari(int vr)
{
	int obala, usao;
	char naz[3];
	pthread_mutex_lock(&m);
	sprintf(naz, "M%d", vr);

	obala = rand() % 2;

	if (obala == 0)
	{
		sprintf(poruka, "%s: Došao na lijevu obalu", naz);
		strcpy(LO[l0], naz);
		l0++;
	}

	else
	{
		sprintf(poruka, "%s: Došao na desnu obalu", naz);
		strcpy(DO[d0], naz);
		d0++;
	}

	ispis();
	usao = 0;
	o_mis[obala]++;

	while (!usao)
	{

		if (c_obala == obala && (c_kan + c_mis) < 7 && c_kan <= (c_mis + 1))
		{
			c_mis++;
			o_mis[obala]--;
			usao = 1;

			// premjesti s OBALE u camac
			premj_ob_c(naz, obala);
			sprintf(poruka, "%s: Ušao u čamac", naz);
			ispis();
			if ((c_mis + c_kan) >= 3)
			{
				pthread_cond_signal(&camac[1]);
			}

			if ((c_mis + c_kan) < 7 && (o_kan[obala]) > 0 && c_kan == c_mis)
				pthread_cond_signal(&uvjet[obala][1]);
		}
		else
		{
			pthread_cond_wait(&uvjet[obala][0], &m);
		}
	}

	pthread_cond_wait(&camac[2], &m);
	c_mis--;
	if ((c_kan + c_mis) == 0)
	{
		pthread_cond_signal(&camac[0]);
		pthread_cond_broadcast(&uvjet[c_obala][0]);
		pthread_cond_broadcast(&uvjet[c_obala][1]);
	}

	pthread_mutex_unlock(&m);
}

void Kanibali(int vr)
{
	int obala, usao;
	char naz[3];
	pthread_mutex_lock(&m);
	sprintf(naz, "K%d", vr);
	obala = rand() % 2;
	if (obala == 0)
	{

		sprintf(poruka, "%s: Došao na lijevu obalu", naz);
		strcpy(LO[l0], naz);
		l0++;
	}
	else
	{
		sprintf(poruka, "%s: Došao na desnu obalu", naz);
		strcpy(DO[d0], naz);
		d0++;
	}

	ispis();
	usao = 0;
	o_kan[obala]++;
	while (!usao)
	{
		if ((obala == c_obala) && ((c_kan + c_mis) < 7) && (((c_kan + 1) <= c_mis) || (c_mis == 0 && c_kan >= 0)))
		{
			c_kan++;
			o_kan[obala]--;
			usao = 1;

			// premjesti s OBALE u camac
			premj_ob_c(naz, obala);
			sprintf(poruka, "%s: Ušao u čamac", naz);
			ispis();
			if ((c_kan + c_mis) >= 3)
			{
				pthread_cond_signal(&camac[1]);
			}

			if ((c_mis + c_kan) < 7 && o_mis[obala] > 0)
				pthread_cond_signal(&uvjet[obala][0]);

			if ((c_mis + c_kan) < 7 && (o_kan[obala]) > 0 && (c_mis == 0))
				pthread_cond_signal(&uvjet[obala][1]);
		}
		else
		{
			pthread_cond_wait(&uvjet[obala][1], &m);
		}
	}

	pthread_cond_wait(&camac[2], &m);
	c_kan--;

	if ((c_kan + c_mis) == 0)
	{
		pthread_cond_signal(&camac[0]);
		pthread_cond_broadcast(&uvjet[c_obala][0]);
		pthread_cond_broadcast(&uvjet[c_obala][1]);
	}

	pthread_mutex_unlock(&m);
}

void Create_thr()
{
	int j, i;
	for (j = 0; j < MAX_MIS; j++)
	{
		pthread_create(&dretva[j * 3 + 2], NULL, Kanibali, 2 * j + 1);
		sleep(1);
		pthread_create(&dretva[j * 3 + 3], NULL, Misionari, j + 1);
		pthread_create(&dretva[j * 3 + 4], NULL, Kanibali, 2 * j + 2);
		sleep(1);
	}

	for (j = 0; j < 3 * MAX_MIS; j++)
		pthread_join(dretva[j + 2], NULL);
}

int main()
{
	pthread_create(&dretva[0], NULL, Camac, NULL);
	pthread_create(&dretva[1], NULL, Create_thr, NULL);
	pthread_join(dretva[1], NULL);
	pthread_join(dretva[0], NULL);

	return 0;
}