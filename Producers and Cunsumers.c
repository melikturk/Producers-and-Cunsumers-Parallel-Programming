#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define THREADNUMBER 8//8 thread var
#define MESSAGESAYISI 2//her biri 2 mesaj gönderecek

int doneSending;

typedef struct message {
	int *basMesP;//mesaj baslangýcý
	int *bitMesP;//mesaj bitisi
	int ekMesSay, alMesSay;//eklenen ve alýnan mesaj sayýsý
	omp_lock_t lock;
} msg;

int main(int argc, char *argv[]) {
	srand(time(NULL));
	int tid;
	int threadNumber = THREADNUMBER; //strtol(argv[1], NULL, 10);
	msg mesajlar[THREADNUMBER];//Her mesaj bir threade ait olacak.
	int count = 0;

#pragma omp parallel num_threads(threadNumber) private(tid)
 {
		tid = omp_get_thread_num();
		mesajlar[tid].basMesP = (int*) (malloc(sizeof(int) * THREADNUMBER * MESSAGESAYISI * 2));//Daha fazla yer olsun diye 2 ile çarptýk.
		mesajlar[tid].bitMesP = (int*) mesajlar[tid].basMesP;
		mesajlar[tid].alMesSay = 0;
		mesajlar[tid].ekMesSay = 0;
		omp_init_lock(&mesajlar[tid].lock);//Her bir thread kendi sahip olduðu threadi initilision etmesi lazým.Bu kod bize bunu saðlýyor.
		int mesg;

#pragma omp barrier//Bunun amacý herbir thread buraya gelene kadar bekleyecek. Yani 8 threadin hepsi buraya gelmeli.
        //Barier olmadan lock yaparsak hata alýrýz.
		for (int i = 0; i < MESSAGESAYISI; i++) { //Burada threadlere random mesaj atýyoruz.
			int dest = rand() % THREADNUMBER;
			mesg= rand();

			omp_set_lock(&mesajlar[dest].lock);//critical sectionu saðlýyor. Amacýmýz bir threade bir yeri kullanýrken baþka bir thread orayý kullanamsýný saðlýyoruz.

					// printf("Random Uretilen Mesaj %d\n",mesg);
					*(mesajlar[dest].bitMesP) = mesg;
					*(mesajlar[dest].bitMesP + 1) = tid;
					mesajlar[dest].bitMesP += 2;//Mesaj için thread id aldýðýmýz için 2 ile topladýk.
					mesajlar[dest].ekMesSay++;

			omp_unset_lock(&mesajlar[dest].lock);
			int queueSize = mesajlar[tid].ekMesSay - mesajlar[tid].alMesSay;

			while (queueSize > 0) {
				count++;
				printf("%d. Ben Thread:%d %d. Threaddden Gelen Mesaj %d \n",count, tid, *(mesajlar[tid].basMesP + 1),*(mesajlar[tid].basMesP));
				mesajlar[tid].basMesP += 2;//Her mesaj 2 yer kaplayacak
				mesajlar[tid].alMesSay++;
				doneSending++;
				queueSize = mesajlar[tid].ekMesSay - mesajlar[tid].alMesSay;
			}
		}
			while (doneSending < THREADNUMBER * MESSAGESAYISI) {//Gelen mesajý ekrena yazýyoruz.
				int queueSize = mesajlar[tid].ekMesSay - mesajlar[tid].alMesSay;
				while (queueSize > 0) {
					count++;
					printf("%d. Ben Thread:%d %d. Threaddden Gelen Mesaj %d \n",count, tid, *(mesajlar[tid].basMesP + 1),*(mesajlar[tid].basMesP));
					mesajlar[tid].basMesP += 2;
					mesajlar[tid].alMesSay++;
					doneSending++;
					queueSize = mesajlar[tid].ekMesSay - mesajlar[tid].alMesSay;
				}
			}
// printf("Hello World from thread number %d\n", tid);
//
// if (tid == 0) {
// threadNumber = omp_get_num_threads();
// printf("Number of threads is %d\n", threadNumber);
// }
#pragma omp barrier
omp_destroy_lock(&mesajlar[tid].lock);
}
return 0;
}
