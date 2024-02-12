# Operacijski_sustavi

Laboratorijske vježbe predmeta Operacijski sustavi koji se izvodi u 4. semestru na Fakultetu elektrotehnike i računarstva u Zagrebu.

**Sadržaj vježbi**

Prvi labos: Signali - Priprema i zadatak (signali.c)

Drugi labos: Višezadaćni rad

- Zadatak 2a: Napisati program koji stvara dva procesa. Svaki proces povećava zajedničku varijablu A za 1 u petlji M puta. Parametar M zadati kao argument iz komandne linije. Sinkronizirati ta dva procesa Dekkerovim algoritmom (Dekker.c)

- Zadatak 2b: Napisati program koji stvara N dretvi. Svaka dretva povećava zajedničku varijablu A za 1 u petlji M puta. Parametre N i M zadati kao argument iz komandne linije. Sinkronizirati dretve Lamportovim algoritmom (Lamport.c) Napomena: Kritični odsječak je povećavanje varijable A za 1, a ne cijela petlja!

Treći labos: Sinkronizacijski mehanizmi

- Zadatak 3a: Riješiti problem pospane frizerke koristeći procese i sinkronizirati ih semaforima (frizerka.c)
- Zadatak 3b: Riješiti problem kanibala i misionara koristeći dretve i sinkronizirati ih monitorom (kanibali.c)

Četvrti labos: Raspoređivanje radnog spremnika straničenjem (stranicenje.c)
