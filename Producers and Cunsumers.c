#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define THREADNUMBER 8//8 thread var
#define MESSAGESAYISI 2//her biri 2 mesaj g�nderecek

int doneSending;

typedef struct message {
	int *basMesP;//mesaj baslang�c�
	int *bitMesP;//mesaj bitisi
	int ekMesSay, alMesSay;//eklenen ve al�nan mesaj say�s�
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
		mesajlar[tid].basMesP = (int*) (malloc(sizeof(int) * THREADNUMBER * MESSAGESAYISI * 2));//Daha fazla yer olsun diye 2 ile �arpt�k.
		mesajlar[tid].bitMesP = (int*) mesajlar[tid].basMesP;
		mesajlar[tid].alMesSay = 0;
		mesajlar[tid].ekMesSay = 0;
		omp_init_lock(&mesajlar[tid].lock);//Her bir thread kendi sahip oldu�u threadi initilision etmesi laz�m.Bu kod bize bunu sa�l�yor.
		int mesg;

#pragma omp barrier//Bunun amac� herbir thread buraya gelene kadar bekleyecek. Yani 8 threadin hepsi buraya gelmeli.
        //Barier olmadan lock yaparsak hata al�r�z.
		for (int i = 0; i < MESSAGESAYISI; i++) { //Burada threadlere random mesaj at�yoruz.
			int dest = rand() % THREADNUMBER;
			mesg= rand();

			omp_set_lock(&mesajlar[dest].lock);//critical sectionu sa�l�yor. Amac�m�z bir threade bir yeri kullan�rken ba�ka bir thread oray� kullanams�n� sa�l�yoruz.

					// printf("Random Uretilen Mesaj %d\n",mesg);
					*(mesajlar[dest].bitMesP) = mesg;
					*(mesajlar[dest].bitMesP + 1) = tid;
					mesajlar[dest].bitMesP += 2;//Mesaj i�in thread id ald���m�z i�in 2 ile toplad�k.
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
			while (doneSending < THREADNUMBER * MESSAGESAYISI) {//Gelen mesaj� ekrena yaz�yoruz.
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
