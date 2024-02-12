#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BR_POS 6 /* broj klijenata */
#define BR_MJ 5  /* ukupan broj mjesta u čekaoni*/

sem_t *KO; /* globalna varijabla => za kazaljku na objekt u zajednickoj */

/*memoriji */

int Id2, n, *got; /* identifikacijski broj segmenta */

void klijent(int i)
{

   int j;

   printf("Klijent(%d): Želim na frizuru\n", i + 1);

   sem_getvalue(KO, &j); /* semafor otvoreno */

   if (j != 0)

      if (!sem_trywait(KO + 1))
      { // čekaonica

         sem_getvalue(KO + 1, &j);
         printf(" Klijent (%d): Ulazim u čekaonicu (%d)\n", i + 1, BR_MJ - j);
         sem_wait(KO + 2); // čeka na stolicu
         *(got + 1) = i + 1;
         sem_post(KO + 1); // oslobađa se mjesto u čekaonici
         sem_post(KO + 3); // budi se frizerka
         printf("Klijent(%d):Frizerka mi radi frizuru\n", i + 1);
         sem_wait(KO + 4); // gotova frizura
         sem_post(KO + 2); // oslobađa se stolicu
      }

      else
         printf("Klijent(%d):Nema mjesta u čekaoni, vratiću se sutra\n", i + 1);

   else

      printf("Klijent(%d):Zatvoren frizerski salon, vratiću se sutra\n", i + 1);
}

void frizerka()
{
   int i, j;
   printf("Frizerka: Otvoram Salon\n");
   sem_post(KO); /* semafor Otvoreno */
   printf("Frizerka: Postavljen znak otvoreno\n");

   while (1)
   {
      // Sleep until someone arrives and wakes you..

      sem_getvalue(KO + 1, &j); // slobodan broj mjesta u čekaonici

      if ((j == BR_MJ) && !*got)

         printf("Frizerka: Spavam dok klijenti ne dođu\n");

      sem_wait(KO + 3); // spava
      sem_getvalue(KO + 1, &j);

      if (!*got || j <= BR_MJ)
      {
         printf("Frizerka: Idem raditi na klijentu %d\n", *(got + 1));
         sleep(1);
         printf("Frizerka: Klijent %d gotov\n", *(got + 1));
         // Release the customer when done cutting...
         sem_post(KO + 4); // seatbelt
      }

      if (*got)
      {
         if (!sem_trywait(KO))
         {
            printf("Frizerka:Postavljam znak Zatvoreno\n");
         } // Postvlja se znak otovoreno. Ako ima još klijenata, ne završava se petlja.

         sem_getvalue(KO + 1, &j);
         if (j == BR_MJ)
         {
            sem_getvalue(KO + 4, &j); // Završavam petlju, ako nema nikog u čekaonici ili ako nema nikog ko je na stolici
            if (j == 1)
               break;
         }
      }
   }

   printf("Frizerka:Zatvaram salon\n");
}

void sat()
{
   int j;
   sleep(15);
   *got = 1;
   sem_getvalue(KO + 1, &j);

   if (j == BR_MJ)
      sem_post(KO + 3); // budim frizerku, ako spava i nema nikog u čekaonici
   printf("Sat:Kraj\n");
}

void brisi(int sig)
{
   /* oslobađanje zajedničke memorije */
   sem_destroy(KO);
   shmdt(KO);
   shmdt(got);
   exit(0);
}

int main()
{
   int i;
   Id2 = shmget(IPC_PRIVATE, sizeof(sem_t) * 5, 0600); /* vraca id segmenta potrebnog za shmat */
   KO = shmat(Id2, NULL, 0);                           /* proces veze segment za svoj adresni prostor */
   shmctl(Id2, IPC_RMID, NULL);                        /* unistavanje segmenta zajednicke memorije */
   sem_init(KO, 1, 0);                                 /* semafor otvoreno */
   sem_init(KO + 1, 1, BR_MJ);                         /* semafor mjesta u čekaonici*/
   sem_init(KO + 2, 1, 1);                             /* semafor smije_izaći */
   sem_init(KO + 3, 1, 0);                             /* semafor izašli */
   sem_init(KO + 4, 1, 0);                             /* semafor vrijeme */

   Id2 = shmget(IPC_PRIVATE, sizeof(int) * 3, 0600); /* vraca id segmenta potrebnog za shmat */
   got = shmat(Id2, NULL, 0);                        /* proces veze segment za svoj adresni prostor */
   shmctl(Id2, IPC_RMID, NULL);                      /* unistavanje segmenta zajednicke memorije */

   *got = 0;
   *(got + 1) = 0;
   *(got + 2) = 0;

   sigset(SIGINT, brisi); // u slučaju prekida briši memoriju

   /* pokretanje paralelnih procesa */

   if (fork() == 0)
   {
      frizerka();
      exit(0);
   }

   if (fork() == 0)
   {
      sat();
      exit(0);
   }

   sleep(1);

   for (i = 0; i < BR_POS * 2; i++)
   {
      if (i > BR_POS)
         sleep(3);

      if (fork() == 0)
      {

         klijent(i);
         exit(0);
      }
   }

   for (i = 0; i < BR_POS * 2; i++)
   {
      (void)wait(NULL); // klijenti
   }

   (void)wait(NULL); // sat
   (void)wait(NULL); // frizerka

   brisi(0);

   return 0;
}