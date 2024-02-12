#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdatomic.h>

atomic_int m, n, k;
atomic_int ZV;
atomic_int *broj, *ulaz;

atomic_int maxbroj()
{
	atomic_int k, max;

	max = broj[0];

	for (k = 1; k < n; k++)

		if (broj[k] > max)

			max = broj[k];

	return max;
}

void Radna(void *x)
{
	atomic_int i1, j;
	atomic_int i;

	i = *((int *)x);

	printf("thread %d rez:%d\n", i, ZV);

	sleep(10);

	for (i1 = 0; i1 < m; i1++)
	{
		*(ulaz + i) = 1;
		*(broj + i) = maxbroj() + 1;
		*(ulaz + i) = 0;

		for (j = 0; j < n; j++)
		{

			while (*(ulaz + j) != 0)
			{
			};

			while ((*(broj + j) != 0) && ((*(broj + j) < *(broj + i)) | ((*(broj + j) == *(broj + i)) && (j < i))))
			{
			};
		}

		ZV++;

		printf("thread %d i1 %d rez:%d\n", i, i1, ZV);

		*(broj + i) = 0;
	}
}

int main(int argc, char *argv[])
{

	pthread_t *thr_id;

	int i;

	n = atoi(argv[1]);
	m = atoi(argv[2]);

	broj = malloc(sizeof(atomic_int) * n);
	ulaz = malloc(sizeof(atomic_int) * n);
	thr_id = malloc(sizeof(pthread_t) * n);

	ZV = 0;

	if (argc != 3)
	{

		printf("Use: %s #1 #2\n", argv[0]);
		exit(1);
	}

	printf("m%d n%d \n", m, n);

	for (i = 0; i < n; i++)

		if (pthread_create(&thr_id[i], NULL, Radna, &i) != 0)
		{

			printf("Greska pri stvaranju dretve!\n");
			exit(1);
		}

	for (i = 0; i < n; i++)

		pthread_join(thr_id[i], NULL);

	printf("kon rez:%d\n", ZV);

	return 0;
}