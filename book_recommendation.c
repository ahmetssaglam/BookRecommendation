/* 
@file
BLM3021 2020-2021 GUZ Proje

��birlik�i filtre (collaborative filtering) y�ntemi ile bir ki�inin �nceki se�imlerine bakarak
yeni kitap �neren bir sistem tasar�m�.

@author
�sim: Ahmet Said SA�LAM
��renci No: 17011501
Tarih: 20.01.2021
E-Mail: l1117501@std.yildiz.edu.tr
Compiler: TDM-GCC 4.9.2 64 bit-Release
IDE: DEV-C++ (version 5.11)
��letim Sistemi: Windows 10 Pro 64 bit
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <ctype.h>
#include<math.h>
#define INPUT "RecomendationDataSet.csv"
#define BUFFER_SIZE 1000 //txt dosyadan al�nan sat�r�n saklanaca�� bufferin boyutu
#define WORD_SIZE 50
#define NAME_SIZE 20
#define AYRAC ";"	//kelimelerin ayrilacagi delim ifadesi

//her bir kullan�c� i�in kullan�c� bilgilerini tutan structure
typedef struct user_info {
	int *points;	//kitaplara verdigi puanlar� tutan dizi icin pointer
	char name[NAME_SIZE];	//kullan�c� ismi
	double *similarities;	//diger kullan�c�lara benzerligini tutan dizi icin pointer
	int count;	//kullan�c�n�n okudugu kitap say�s�
}user_info;

//double arraydeki daha onceden maks olarak donmemis elemanlar aras�ndaki maks eleman�n indisini donduren fonksiyon
int findMax(double *array, int *flags, int size) {
	int i;
	double temp;
	int x = 0;
	
	
	//maks hesab�na dahil olmam�s ilk degerin indisini bul
	while(flags[x] == 1) {
		x++;
	}
	
	//son elemana gelindiyse kontrol etmeden don
	if(x == (size - 1)) {
		return x;
	}
	
	temp = array[x]; //degeri al
	
	//diziyi x'ten itibaren gez
	for(i = (x + 1); i < size; i++) {
		//gezilen eleman tempten buyukse ve daha once donmediyse tempe al ve indisini sakla
		if(temp < array[i] && flags[i] == 0) {
			temp = array[i];
			x = i;
		}
	}
	flags[x] = 1;	//ilgili eleman donecek, flag degerini 1'e cek
	return x;	//indisi dondur
}

//double dizideki en buyuk eleman�n indisini donduren fonksiyon
int findMaxV2(double *array, int size) {
	int i;
	double temp;
	int x = 0;
	
	temp = array[0];
	
	for(i = 1; i < size; i++) {
		if(temp < array[i]) {
			temp = array[i];
			x = i;
		}
	}
	return x;
}

//icerisine ald�g� similarities dizininin maks k elamn�n�n indislerini donduren fonksiyon
int *findSimilars(int size, double *similarities, int k) {
	int i;
	int *array = (int*) calloc(k, sizeof(int));
	int *flags = (int*) calloc(size, sizeof(int));	//flags dizisine tum elemanlar� 0 olacak sekilde yer ac�l�r
	
	//en benzer k kisi bulunur
	for(i = 0; i < k; i++) {
		array[i] = findMax(similarities, flags, size);	//en benzer i. kisi diziye aktar�l�r
	}
	
	free(flags);
	 
	return array;	//dizi dondurulur
}

//kitap isimlerini tutmak icin yer acan ve isimleri yerlestiren fonksiyon
char **allocateBookNames(char *buffer, int *book_count) {
	char **bookNames;
	char *token;
	char *temp = (char*) calloc(BUFFER_SIZE, sizeof(char));
	int count = 0;
	int i;
	
	//bufferin yedegi al�n�r
	strcpy(temp,buffer);
	
	token = strtok(buffer, AYRAC);
	while(token != NULL) {
		count++;	//kac adet kitap var sayilir
		token = strtok(NULL, AYRAC);
	}
	
	count--; //ilk gozdeki gereksiz kelime counta say�lmaz
	
	bookNames = (char**) calloc(count,sizeof(char*));	//kitap sayisi kadar, matriste satir icin yer acilir
	for(i = 0; i < count; i++) {
		bookNames[i] = (char*) calloc(WORD_SIZE, sizeof(char));	//maksimum kitap ismi uzunlugu WORD_SIZE kadar olacak sekilde matriste sutun yeri acilir
	}
	
	i = 0;	//matriste satir indisi
	
	//kitap isimleri matrise kopyalanir
	token = strtok(temp, AYRAC);	//ilk kelimeyi al isleme sokma
	token = strtok(NULL, AYRAC);	//ikinci kelimeden itibaren kitap isimleri
	while(token != NULL) {
		strcpy(bookNames[i],token);
		i++;
		token = strtok(NULL, AYRAC);
	}
	
	//al�nan son kitab�n son harfi new line karakter ise temizlenir
	if(bookNames[(count - 1)][strlen(bookNames[(count - 1)]) - 1] == '\n') {
		bookNames[(count - 1)][strlen(bookNames[(count - 1)]) - 1] = '\0'; 
	}
	
	*book_count = count;	//kitap sayisi guncellenir
	
	return bookNames;	//kitap matrisi dondurulur
}

//struct array olusturmak icin yer acan fonksiyon
struct user_info *userAllocator(int count) {
	struct user_info *users;
	users = (struct user_info*) calloc(count, sizeof(struct user_info));	//kullanici sayisi kadar struct yeri acilir
	
	return users;	//struct dizisi disari dondurulur
}

//user struct dizisinin icini dolduran fonksiyon
struct user_info fillStruct(int *book_count, int old_user_count, char *buffer) {
	int b_count = *book_count;	//sistemdeki kitap sayisi
	int i;
	char *token;
	struct user_info temp_user;	//gecici struct degiskeni
	
	temp_user.points = (int*) calloc(b_count, sizeof(int));	//kitap sayisi kadar structun icinde points yeri acilir
	temp_user.similarities = (double*) calloc(old_user_count, sizeof(double));	//eski kullanici sayisi kadar similarities dizisi icin yer acilir
	temp_user.count = 0;	//kullanicin okudugu kitap sayisini tutan degisken 0'lanir
	
	//user ismi temizlenir
	for(i = 0; i < NAME_SIZE; i++) {
		temp_user.name[i] = '\0';
	}
	
	token = strtok(buffer,AYRAC);	//kullan�c� ismini al
	//printf("token isim : %s\n",token);
	strcpy(temp_user.name, token);	//structa ata
	
	//kullanicin kitaplar icin verdigi puanlar sirasiyla alinir ve points dizisine yerlestirilir
	token = strtok(NULL,AYRAC);	
	for(i = 0; i < b_count; i++) {
		temp_user.points[i] = atoi(token);
		//printf("FILL KONTROL :string : %s, int : %d\n",token, atoi(token));
		//kullanici kitab� okuduysa count arttirilir
		if(temp_user.points[i] != 0) {
			temp_user.count++;
		}
		token = strtok(NULL,AYRAC);
	}
	return temp_user;	//gecici kullanici disari dondurulur
}

//csv dosyas�n�n ilk sat�r�ndan eski ve yeni kullanc� say�lar�n�, ikinci sat�r�ndan kitap isimlerini ve di�er sat�rlar�ndan kullan�c� verilerini okuyan fonksiyon
int readFile(int *book_count, struct user_info **old_users_main, struct user_info **new_users_main, int *old_count, int *new_count, char ***book_names) {
	int old_user_count;	//eski kullan�c� say�s�n� tutan degisken
	int new_user_count;	//yeni kullan�c� say�s�n� tutan degisken
	FILE *inputFile; //file pointer
	char *buffer = (char*) calloc(BUFFER_SIZE,sizeof(char)); //dosyadan alinan satirin tutuldugu buffer
	char *temp = (char*) calloc(WORD_SIZE,sizeof(char));	//temporary dizi
	char *token;
	char **books;	//kitap isimlerini tutan matris
	int i, j;	//dongu degiskenleri
	struct user_info *old_users, *new_users;	//eski ve yeni kullan�c� bilgilerini tutan diziler
	
	//dosya ac�lamazsa hata verilir
	if((inputFile = fopen(INPUT,"r")) == NULL) {
		printf("Dosya okunmak icin acilamadi!\n");
		return 1;
	}
	//dosya acilirsa
	else {
		//dosyadaki ilk sat�r okunur
		fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile); //ilk sat�r� dosyadan buffer'a al
		//printf("BUFFER : %s\n",buffer);
		
		//eski kullan�c� say�s� okunur
		token = strtok(buffer,AYRAC);
		strcpy(temp,token);
		old_user_count = atoi(temp);
		old_users = userAllocator(old_user_count);	//eski kullanicilari tutmak icin dizi tanimlanir ve yer acilir
		*old_count = old_user_count;	//eski kullanici sayisi maindeki degiskenin icine aktarilir
		
		//yeni kullan�c� say�s� okunur
		token = strtok(NULL,AYRAC);
		strcpy(temp,token);
		new_user_count = atoi(temp);
		new_users = userAllocator(new_user_count);	//yeni kullanicilari tutmak icin dizi tanimlanir ve yer acilir
		*new_count = new_user_count;	//yeni kullanici sayisi maindeki degiskenin icine aktarilir
		
		//dosyadaki ikinci sat�r okunur
		fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile);
		//printf("BUFFER : %s\n",buffer);
		
		books = allocateBookNames(buffer, book_count);	//kitap isimleri al�n�r
		*book_names = books;	//maindeki matrise aktar�l�r
		
		//buffer temizlenir
		for(i = 0; i < BUFFER_SIZE; i++) {
			buffer[i] = '\0';
		}
		
		//dosyadaki satirlar eski kullanici sayisi kadar okunur ve eski kullanicilarin verileri struct dizisine kaydedilir
		for(i = 0; i < old_user_count; i++) {
			fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile);	//satiri dosyadan oku
			old_users[i] = fillStruct(book_count,old_user_count,buffer);	//fonksiyondan donen kullaniciyi diziye ata	
		}
		
		//dosyadaki satirlar yeni kullanici sayisi kadar okunur ve yeni kullanicilarin verileri struct dizisine kaydedilir
		for(i = 0; i < new_user_count; i++) {
			fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile);	//satiri dosyadan oku
			new_users[i] = fillStruct(book_count,old_user_count,buffer);	//fonksiyondan donen kullaniciyi diziye ata
		}
		
		//eski ve yeni kullan�c� bilgileri maindeki dizilere aktar�l�r
		*old_users_main = old_users;
		*new_users_main = new_users;
		
		fclose(inputFile);
	}
	
	//free islemleri
	free(buffer);
	free(temp);
	
	return 0;
	
}

//i�ine ald��� kullan�c�n�n okudu�u kitaplar�n puan ortalamas�n� d�nd�ren fonksiyon (estimationda kullan�lan mean)
double getAveragePoint(struct user_info user, int book_count) {
	int sum = 0;
	int i;
	
	for(i = 0; i < book_count; i++) {
		sum += user.points[i];
	}
	
	return (double) sum / (double) user.count;
}

//similarity hesab�nda kullan�lan mean / icine ald�g� iki kullan�c�n�n ortak okudugu kitaplar�n, 1. kullan�c� bak�m�ndan puan ortalamas�n� hesaplayan fonksiyon
double getAveragePointV2(struct user_info user_1, struct user_info user_2, int book_count) {
	int sum = 0;
	int i;
	int common_count = 0;	//ortak kitap say�s�
	for(i = 0; i < book_count; i++) {
		//kitap ortak ise
		if(user_1.points[i] != 0 && user_2.points[i] != 0) {
			common_count++;
			sum += user_1.points[i];
		}
	}
	return (double) sum / (double) common_count;	//return mean
}

//fonksiyon i�erisine ald��� userlar�n birbirlerine ne kadar benzedi�ini hesaplay�p d�nd�r�r
double getSimilarity(struct user_info user_1, struct user_info user_2, int book_count) {
	double similarity = 0.0;
	double parameter_1;
	double parameter_2;
	double avg_1;
	double avg_2;
	double sum_pay = 0.0;
	double sum_payda_1 = 0.0;
	double sum_payda_2 = 0.0;
	int i;
	
	//avg_1 = getAveragePoint(user_1, book_count);
	//avg_2 = getAveragePoint(user_2, book_count);
	
	//meanlar hesaplan�r
	avg_1 = getAveragePointV2(user_1, user_2, book_count);
	avg_2 = getAveragePointV2(user_2, user_1, book_count);

	//iki user'�n da okudugu kitaplar� gez
	for(i = 0; i < book_count; i++) {
		//ilgili kitab� ikisi de okuduysa
		//pay ve paydalar� hesaplayarak formulu uygula
		if(user_1.points[i] != 0 && user_2.points[i] != 0) {
			parameter_1 = (double) user_1.points[i] - avg_1;
			parameter_2 = (double) user_2.points[i] - avg_2;
			sum_pay += (parameter_1 * parameter_2);
			sum_payda_1 += pow(parameter_1, 2);
			sum_payda_2 += pow(parameter_2, 2);
		}
	}
	
	if(sum_payda_1 != 0 && sum_payda_2 != 0) {
		sum_payda_1 = sqrt(sum_payda_1);
		sum_payda_2 = sqrt(sum_payda_2);
		similarity = sum_pay / (sum_payda_1 * sum_payda_2);
	}
	
	return similarity;	//similartyi dondur
}

//sistemdeki yeni kullanicilarin, tum eski kullanicilara benzerligini olcup kaydeden fonksiyon
void totalSimilarities(struct user_info *old_users, struct user_info *new_users, int old_user_count, int new_user_count, int book_count) {
	int i, j; //dongu degiskenleri
	
	for(i = 0; i < new_user_count; i++) {
		for(j = 0; j < old_user_count; j++) {
			//yeni kullanicilarin eski kullanicilara olan benzerlikleri hesaplanir
			new_users[i].similarities[j] = getSimilarity(new_users[i], old_users[j], book_count);
		}
	}
}

//pred fonksiyonu ger�eklenir. new user'a en benzer k adet old user'a g�re, new user'�n okumad�g� kitap (book_id'si verilen kitap) icin tahmini puan hesaplan�r
double calculateEstimation(struct user_info new_user, struct user_info *old_users, int *mostSimilars, int book_count, int k, int book_id ) {
	int i;
	double sum_pay = 0.0;
	double sum_payda = 0.0;
	double parameter;
	int old_user_id;
	
	// k kadar don ve formulu uygula
	for(i = 0; i < k; i++) {
		old_user_id = mostSimilars[i];	//en benzer i. kisinin idsini al
		parameter = (double) old_users[old_user_id].points[book_id] - getAveragePoint(old_users[old_user_id], book_count);
		sum_pay += new_user.similarities[old_user_id] * parameter;
		sum_payda += new_user.similarities[old_user_id];
	}
	
	return getAveragePoint(new_user, book_count) + (sum_pay / sum_payda);
}

//i�ine ald�g� kullan�c�n�n okumad�g� tum kitaplara tahmin yapan fonksiyon
double **totalEstimations(struct user_info new_user, struct user_info *old_users, int *mostSimilars, int book_count, int k) {
	
	int i, j;
	int recommend_count = book_count - new_user.count;	//new user'�n okumad�g� kitap say�s�
	
	//ilk sat�r� tahmini puanlar�, ikinci sat�r� kitap indisini tutan double matris tan�mlan�r
	//sutun say�s� new user'�n okumad�g� kitap say�s� kadard�r
	double **resultMatrix = (double**) calloc(2, sizeof(double*)); 
	for(i = 0; i < 2; i++) {
		resultMatrix[i] = (double*) calloc(recommend_count, sizeof(double));
	}
	//matrisin ilk sat�r� 0'lan�r
	for(i = 0; i < recommend_count; i++) {
		resultMatrix[0][i] = 0.0;
	}
	j = 0;
	//ikinci sat�ra kitap kitap indisleri (id) leri at�l�r
	for(i = 0; i < book_count; i++) {
		if(new_user.points[i] == 0) {
			resultMatrix[1][j] = (double) i;
			j++;
		}
	}
	//okunmayan her bir kitap icin tahmini puan hesaplan�r
	for(i = 0; i < recommend_count; i++) {
		resultMatrix[0][i] = calculateEstimation(new_user, old_users, mostSimilars, book_count, k, (int) resultMatrix[1][i]);
	}
	
	return resultMatrix;	//hesaplanan matris dondurulur
}

//sistemdeki k degerini kullanicidan alip buna gore tahminleri tablo halinde her kullanici icin yazdiran fonksiyon
void printTable(int new_user_count, int old_user_count, struct user_info *new_users, struct user_info *old_users, int book_count, char **book_names) {
	
	int i, j; //dongu degiskenleri
	int k; //k degeri (en benzer kac adet eski kullanici sayisi)
	int *similars; //en benzer k eski kullanicinin indisleri(id)
	int recommend_count; //kullanicinin okumadigi ve kullaniciya onerilecek olan kitap sayisi
	int rec_id; //en fazla tahmini puana sahip okunmamis kitab�n id'sini tutan degisken
	double **estimate_results; //ilk sat�r� tahmini puanlar�, ikinci sat�r� okunmam�s kitaplar�n idlerini tutan matris
	
	
	//Verileri Tablo Seklinde yazd�ran blok
	printf("Benzer kullanici sayisini (K) giriniz : ");
	scanf("%d",&k);	//K degeri alinir
	
	printf("\nNew Users\t%d Most Similar Users\t\t\tRecommended Book\n\n",k);
	for(i = 0; i < new_user_count; i++) {
		similars = findSimilars(old_user_count,new_users[i].similarities,k);
		recommend_count = book_count - new_users[i].count;
		
		estimate_results = (double**) calloc(2, sizeof(double*));
		for(j = 0; j < 2; j++) {
			estimate_results[j] = (double*) calloc(recommend_count, sizeof(double));
		}
		
		estimate_results = totalEstimations(new_users[i],old_users, similars, book_count, k);	//tahmini puanlar hesaplan�r
		
		
		//en fazla puana sahip okunmam�s kitap bulunur ve onerilir
		//okunmam�s kitap say�s� 1'den fazla ise
		if(recommend_count > 1) {
			rec_id = findMaxV2(estimate_results[0], recommend_count);	//en fazla tahmini puana sahip okunmam�s kitab� bul
			//printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][rec_id]]);	//en fazla puana sahip okunmam�s kitap onerilir
		}
		else {
			rec_id = 0;
			//printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][0]]);	//en fazla puana sahip okunmam�s kitap onerilir
		}
		
		
		for(j = 0; j < k; j++) {
			if(j == k / 2) {
				printf("%s",new_users[i].name);
				printf("\t\t%d.) %s  similarity = %lf",(j+1),old_users[similars[j]].name, new_users[i].similarities[similars[j]]);
				printf("\t\t%s\n",book_names[(int) estimate_results[1][rec_id]]);
			}
			else {
				printf("\t\t%d.) %s  similarity = %lf\n",(j+1),old_users[similars[j]].name, new_users[i].similarities[similars[j]]);
			}
		}
		
		for(j = 0; j < recommend_count; j++) {
			free(estimate_results[j]);
		}
		free(estimate_results);
		free(similars);
		
		printf("\n---------------------------------------------------------------------------------------\n");
	}	
}

//sistemdeki k degerini ve kullanici isimini alip bu kullaniciya gore tahminleri yazdiran fonksiyon
void printUser(int new_user_count, int old_user_count, struct user_info *new_users, struct user_info *old_users, int book_count, char **book_names) {
	
	int control_1 = 1; //dogru input(gecerli kullanici ismi) alimini kontrol eden degisken
	int i, j; //dongu degiskenleri
	int id; //oneri yapilacak new user'�n idsi
	int k; //k degeri (en benzer kac adet eski kullanici sayisi)
	int *similars; //en benzer k eski kullanicinin indisleri(id)
	int recommend_count; //kullanicinin okumadigi ve kullaniciya onerilecek olan kitap sayisi
	int rec_id; //en fazla tahmini puana sahip okunmamis kitab�n id'sini tutan degisken
	char *user_name = (char*) calloc(NAME_SIZE, sizeof(char)); //oneri yap�lacak kullanici ismi
	double **estimate_results; //ilk sat�r� tahmini puanlar�, ikinci sat�r� okunmam�s kitaplar�n idlerini tutan matris
	
	//inputu kontrol et, dogrusunu alana kadar don
	while(control_1) {
		id = 0;
		printf("Arama yapilacak kullanici ismini giriniz : ");
		scanf("%s",user_name);
		printf("\n");
		while(id < new_user_count && control_1 == 1) {
			if(strcmp(new_users[id].name, user_name) == 0) {
				control_1 = 0;
			}
			id++;
		}
		if(control_1) {
			printf("Gecersiz kullanici ismi girdiniz! Tekrar giriniz.\n");
		}
	}
	id--;	//kullanici id'si belirlenir
	printf("Benzer kullanici sayisini (K) giriniz : ");
	scanf("%d",&k);	//K degeri alinir
	printf("\n");
	similars = findSimilars(old_user_count,new_users[id].similarities,k);	//id'si belli olan kisiye en benzer K kisinin id'si belirlenir
	for(i = 0; i < k; i++) {
		printf("%d. most similar person is %s to %s with similarities %lf\n",(i+1),old_users[similars[i]].name, new_users[id].name, new_users[id].similarities[similars[i]]);
	}
	
	recommend_count = book_count - new_users[id].count;	//kullan�c�n�n okumad�g� kitap say�s� belirlenir
	
	//kullan�c�n�n okumad�g� kitaplar icin tahmini puanlar� ve kitap id'lerini tutan 2 sat�rl� matris icin yer acilir
	estimate_results = (double**) calloc(2, sizeof(double*));
	for(i = 0; i < 2; i++) {
		estimate_results[i] = (double*) calloc(recommend_count, sizeof(double));
	}
	
	estimate_results = totalEstimations(new_users[id],old_users, similars, book_count, k);	//tahmini puanlar hesaplan�r
	
	//hesaplanan puanlara gore oneri kitaplar� ekrana yazdirilir
	printf("\nRecommendations for %s :\n",new_users[id].name);
	for(i = 0; i < recommend_count; i++) {
		printf("%d. recommend : %s with estimation % lf\n",(i+1), book_names[(int) estimate_results[1][i]], estimate_results[0][i]);
	}
	
	//en fazla puana sahip okunmam�s kitap bulunur ve onerilir
	//okunmam�s kitap say�s� 1'den fazla ise
	if(recommend_count > 1) {
		rec_id = findMaxV2(estimate_results[0], recommend_count);	//en fazla tahmini puana sahip okunmam�s kitab� bul
		printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][rec_id]]);	//en fazla puana sahip okunmam�s kitap onerilir
	}
	else {
		printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][0]]);	//en fazla puana sahip okunmam�s kitap onerilir
	}
	
	printf("---------------------------------------------------------\n");
	
	//free islemleri
	for(i = 0; i < recommend_count; i++) {
		free(estimate_results[i]);
	}
	free(estimate_results);
	free(similars);
	free(user_name);
	
}

int main() {
	
	int book_count, old_user_count, new_user_count;	//sistemdeki kitap ile birlikte eski ve yeni kullanici sayilarini tutan degiskenler
	int choice;	//kullanicinin veri goruntuleme secenegini tutan degisken
	int cont = 1; //kullan�c� istedigi surece arama yapmas�n� saglayan while dongusune verilecek degisken
	int file_control;	//dosyan�n acilip acilamadigini kontrol eden degisken
	char **book_names;	//kitap isimlerini tutuan matris
	struct user_info *old_users, *new_users;	//eski ve yeni kullanici bilgilerini tutuan diziler
	
	
	//veriler dosyadan okunur
	file_control = readFile(&book_count, &old_users, &new_users, &old_user_count, &new_user_count, &book_names);
	//dosya okunamad�ysa program sonland�r�l�r
	if(file_control == 1) {
		return 0;
	}
		
	//yeni kullanicilarin eski kullanicilara olan benzerlikleri hesaplanir
	totalSimilarities(old_users, new_users, old_user_count, new_user_count, book_count);
	
	//kullan�c� istedigi surece donen while dongusu
	while(cont) {
		
		printf("Programi kullanici ozelinde calistirmak icin 1'e, tablo halinde tum kullanicilari gozlemlemek icin 0'a basiniz...\n");
		scanf("%d",&choice);
		
		//kullan�c�n secimine gore output ekrana yazd�r�l�r
		if(choice) {
			printUser(new_user_count, old_user_count, new_users, old_users, book_count, book_names);
		}
		else {
			printTable(new_user_count, old_user_count, new_users, old_users, book_count, book_names);
		}
				
		printf("\nYeniden islem yapmak icin 1'e, cikmak icin 0'a basiniz.\n");
		scanf("%d",&cont);
		printf("\n");
	}
	
	return 0;
}
