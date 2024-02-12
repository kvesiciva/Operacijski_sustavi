#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
   unsigned short *tablica, x, i, i1, j, j1, k, l, trok, t, mint, mini, minj, pomtab, fizadr;
   unsigned long long int *okvir, *disk;

   int N, M;

   N = atoi(argv[1]);
   M = atoi(argv[2]);

   srand(time(0));

   tablica = (unsigned short *)malloc(N * 16 * sizeof(unsigned short));
   okvir = (unsigned long long int *)malloc(M * sizeof(unsigned long long int));
   disk = (unsigned long long int *)malloc(N * 16 * sizeof(unsigned long long int));

   for (i = 0; i < N; ++i)
      for (j = 0; j < 16; ++j)
         *(tablica + i * N + j) = 0;
   for (i = 0; i < M; ++i)
      *(okvir + i) = 0;
   for (i = 0; i < N; ++i)
      for (j = 0; j < 16; ++j)
         *(disk + i * 16 + j) = 0;
   t = 0;

   trok = 0;

   while (1 == 1)
   {
      for (i = 0; i < N; ++i)
      {
         printf("---------------------------\n");
         printf("proces: %d\n", i);
         printf("t: %d\n", t);
         x = (rand() % 0x3fe) & 0x3fe;
         printf("log. adresa: 0x%04x\n", x);
         j = x >> 6;
         if (!(*(tablica + i * N + j) & 0x0020))
         {
            printf("Promasaj!\n");
            if (trok < M)
            {
               okvir[trok] = *(disk + i * 16 + j); // prebaciti s diska u okvir
               *(tablica + i * N + j) |= 0x0020;
               *(tablica + i * N + j) |= t;
               l = trok << 6;
               *(tablica + i * N + j) |= l;
               printf("dodijeljen okvir : 0x%04x\n", trok);
               trok++;
               printf("zapis tablice: 0x%04x\n", *(tablica + i * N + j));
               printf("sadržaj adrese: 0x%04llx\n", okvir[trok]);
               okvir[trok]++;
               *(disk + i * 16 + j) = okvir[trok];
            }
            else
            { // nađi najstariji lru jer su okviri popunjeni
               mint = 32;
               for (i1 = 0; i1 < N; i1++)
                  for (j1 = 0; j1 < 16; j1++)
                     if ((*(tablica + i1 * N + j1) & 0x0020))
                     {
                        if (*(tablica + i1 * N + j1) & 0x001f < mint)
                           mint = *(tablica + i1 * N + j1) & 0x001f;
                        mini = i1;
                        minj = j1;
                        pomtab = *(tablica + i1 * N + j1);
                     }
               *(tablica + mini * N + minj) = 0; // izbriši sadržaj tablice sa najstarijim lru
               l = pomtab >> 6;
               okvir[l] = *(disk + i * 16 + j);                                        // prebaciti s diska u okvir
               printf("Izbacujem stranicu : 0x%04x iz procesa %d\n", minj << 5, mini); //?? minj
               printf("lru izbacene stranice : 0x%04x\n", pomtab & 0x001f);
               printf("dodijeljen okvir : 0x%04x\n", l);

               // ažuriraj tablicu procesa sa novim okvirom
               *(tablica + i * N + j) |= 0x0020;
               *(tablica + i * N + j) |= t;
               l = l << 6;
               *(tablica + i * N + j) |= l;
               printf("zapis tablice: 0x%04x\n", *(tablica + i * N + j));
               printf("sadržaj adrese: 0x%04llx\n", okvir[minj]);

               // povećaj sadržaj okvir za 1 i sinkroniziraj s diskom
               okvir[minj]++;
               *(disk + i * 16 + j) = okvir[minj];
            }
            fizadr = 0x003f & x;
            l >> 1;
            fizadr |= l;
            printf("fizička adresa: 0x%04x\n", fizadr);
         }
         else
         { // ako je stranica već u memoriji
            printf("2zapis tablice: 0x%04x\n", *(tablica + i * N + j));
            l = *(tablica + i * N + j) >> 6;
            printf("2sadržaj adrese: 0x%04llx\n", okvir[l]);
            // povećaj sadržaj okvir za 1 i sinkroniziraj s diskom
            okvir[l]++;
            *(disk + i * 16 + j) = okvir[l];
            fizadr = 0x003f & x;
            l << 6;
            fizadr |= l;
            printf("2fizička adresa: 0x%04x\n", fizadr);
         }

         printf("\n");
         t++;
         if (t == 32)
         {
            t = 0;
            // po svim tablicama stavi lru na 0
            for (i1 = 0; i1 < N; i1++)
               for (j1 = 0; j1 < 16; j1++)
                  *(tablica + i1 * N + j1) &= 0xffe0;
         }
         sleep(2);
      }
   }
   return 0;
}