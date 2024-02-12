#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int Id, n; /* identifikacijski broj segmenta */
atomic_int *ZV, *pravo, *zast;

void prvi(int br)
{
   int i1;

   for (i1 = 0; i1 < br; i1++)
   {
      zast[0] = 1;
      while (zast[1])
      {
         if (zast[2] == 1)
         { // pravo
            zast[0] = 0;
            while (zast[2] != 2)
            { // pravo
            }
            zast[0] = 1;
         }
      }

      zast[3]++;

      printf("prvi %d\n", zast[3]);

      sleep(1);

      zast[2] = 1; // pravo

      zast[0] = 0;
   }
}

void drugi(int br)
{
   int i1;

   for (i1 = 0; i1 < br; i1++)
   {

      zast[1] = 1;

      while (zast[0])
      {

         if (zast[2] == 2)
         { // pravo

            zast[1] = 0;

            while (zast[2] != 1)
            { // pravo
            }

            zast[1] = 1;
         }
      }

      zast[3]++;

      printf("drugi %d\n", zast[3]);

      sleep(1);

      zast[2] = 2; // pravo

      zast[1] = 0;
   }
}

void brisi(int sig)
{
   /* oslobađanje zajedničke memorije */
   (void)shmdt((char *)zast);
   (void)shmctl(Id, IPC_RMID, NULL);

   exit(0);
}

int main(int argc, char *argv[])
{

   n = atoi(argv[1]);

   if (argc != 2)
   {
      printf("Use: %s #1\n", argv[0]);
      exit(1);
   }

   /* zauzimanje zajedničke memorije */

   Id = shmget(IPC_PRIVATE, 4 * sizeof(int), IPC_CREAT | 0666);

   if (Id == -1)

      exit(1); /* greška - nema zajedničke memorije */

   zast = (int *)shmat(Id, NULL, 0);

   zast[0] = 0;
   zast[1] = 0;
   zast[2] = 1; // pravo

   zast[3] = 0; // brojač ZV

   sigset(SIGINT, brisi); // u slučaju prekida briši memoriju

   /* pokretanje paralelnih procesa */

   if (fork() == 0)
   {
      prvi(n);
      exit(0);
   }

   /* pokretanje paralelnih procesa */

   if (fork() == 0)
   {
      drugi(n);
      exit(0);
   }

   (void)wait(NULL);
   (void)wait(NULL);
   printf("Rezultat:%d\n", zast[3]);
   brisi(0);
   return 0;
}