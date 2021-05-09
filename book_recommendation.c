/* 
@file
BLM3021 2020-2021 GUZ Proje

Ýþbirlikçi filtre (collaborative filtering) yöntemi ile bir kiþinin önceki seçimlerine bakarak
yeni kitap öneren bir sistem tasarýmý.

@author
Ýsim: Ahmet Said SAÐLAM
Öðrenci No: 17011501
Tarih: 20.01.2021
E-Mail: l1117501@std.yildiz.edu.tr
Compiler: TDM-GCC 4.9.2 64 bit-Release
IDE: DEV-C++ (version 5.11)
Ýþletim Sistemi: Windows 10 Pro 64 bit
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include <ctype.h>
#include<math.h>
#define INPUT "RecomendationDataSet.csv"
#define BUFFER_SIZE 1000 //txt dosyadan alýnan satýrýn saklanacaðý bufferin boyutu
#define WORD_SIZE 50
#define NAME_SIZE 20
#define AYRAC ";"	//kelimelerin ayrilacagi delim ifadesi

//her bir kullanýcý için kullanýcý bilgilerini tutan structure
typedef struct user_info {
	int *points;	//kitaplara verdigi puanlarý tutan dizi icin pointer
	char name[NAME_SIZE];	//kullanýcý ismi
	double *similarities;	//diger kullanýcýlara benzerligini tutan dizi icin pointer
	int count;	//kullanýcýnýn okudugu kitap sayýsý
}user_info;

//double arraydeki daha onceden maks olarak donmemis elemanlar arasýndaki maks elemanýn indisini donduren fonksiyon
int findMax(double *array, int *flags, int size) {
	int i;
	double temp;
	int x = 0;
	
	
	//maks hesabýna dahil olmamýs ilk degerin indisini bul
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

//double dizideki en buyuk elemanýn indisini donduren fonksiyon
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

//icerisine aldýgý similarities dizininin maks k elamnýnýn indislerini donduren fonksiyon
int *findSimilars(int size, double *similarities, int k) {
	int i;
	int *array = (int*) calloc(k, sizeof(int));
	int *flags = (int*) calloc(size, sizeof(int));	//flags dizisine tum elemanlarý 0 olacak sekilde yer acýlýr
	
	//en benzer k kisi bulunur
	for(i = 0; i < k; i++) {
		array[i] = findMax(similarities, flags, size);	//en benzer i. kisi diziye aktarýlýr
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
	
	//bufferin yedegi alýnýr
	strcpy(temp,buffer);
	
	token = strtok(buffer, AYRAC);
	while(token != NULL) {
		count++;	//kac adet kitap var sayilir
		token = strtok(NULL, AYRAC);
	}
	
	count--; //ilk gozdeki gereksiz kelime counta sayýlmaz
	
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
	
	//alýnan son kitabýn son harfi new line karakter ise temizlenir
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
	
	token = strtok(buffer,AYRAC);	//kullanýcý ismini al
	//printf("token isim : %s\n",token);
	strcpy(temp_user.name, token);	//structa ata
	
	//kullanicin kitaplar icin verdigi puanlar sirasiyla alinir ve points dizisine yerlestirilir
	token = strtok(NULL,AYRAC);	
	for(i = 0; i < b_count; i++) {
		temp_user.points[i] = atoi(token);
		//printf("FILL KONTROL :string : %s, int : %d\n",token, atoi(token));
		//kullanici kitabý okuduysa count arttirilir
		if(temp_user.points[i] != 0) {
			temp_user.count++;
		}
		token = strtok(NULL,AYRAC);
	}
	return temp_user;	//gecici kullanici disari dondurulur
}

//csv dosyasýnýn ilk satýrýndan eski ve yeni kullancý sayýlarýný, ikinci satýrýndan kitap isimlerini ve diðer satýrlarýndan kullanýcý verilerini okuyan fonksiyon
int readFile(int *book_count, struct user_info **old_users_main, struct user_info **new_users_main, int *old_count, int *new_count, char ***book_names) {
	int old_user_count;	//eski kullanýcý sayýsýný tutan degisken
	int new_user_count;	//yeni kullanýcý sayýsýný tutan degisken
	FILE *inputFile; //file pointer
	char *buffer = (char*) calloc(BUFFER_SIZE,sizeof(char)); //dosyadan alinan satirin tutuldugu buffer
	char *temp = (char*) calloc(WORD_SIZE,sizeof(char));	//temporary dizi
	char *token;
	char **books;	//kitap isimlerini tutan matris
	int i, j;	//dongu degiskenleri
	struct user_info *old_users, *new_users;	//eski ve yeni kullanýcý bilgilerini tutan diziler
	
	//dosya acýlamazsa hata verilir
	if((inputFile = fopen(INPUT,"r")) == NULL) {
		printf("Dosya okunmak icin acilamadi!\n");
		return 1;
	}
	//dosya acilirsa
	else {
		//dosyadaki ilk satýr okunur
		fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile); //ilk satýrý dosyadan buffer'a al
		//printf("BUFFER : %s\n",buffer);
		
		//eski kullanýcý sayýsý okunur
		token = strtok(buffer,AYRAC);
		strcpy(temp,token);
		old_user_count = atoi(temp);
		old_users = userAllocator(old_user_count);	//eski kullanicilari tutmak icin dizi tanimlanir ve yer acilir
		*old_count = old_user_count;	//eski kullanici sayisi maindeki degiskenin icine aktarilir
		
		//yeni kullanýcý sayýsý okunur
		token = strtok(NULL,AYRAC);
		strcpy(temp,token);
		new_user_count = atoi(temp);
		new_users = userAllocator(new_user_count);	//yeni kullanicilari tutmak icin dizi tanimlanir ve yer acilir
		*new_count = new_user_count;	//yeni kullanici sayisi maindeki degiskenin icine aktarilir
		
		//dosyadaki ikinci satýr okunur
		fgets(buffer,BUFFER_SIZE * sizeof(char),inputFile);
		//printf("BUFFER : %s\n",buffer);
		
		books = allocateBookNames(buffer, book_count);	//kitap isimleri alýnýr
		*book_names = books;	//maindeki matrise aktarýlýr
		
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
		
		//eski ve yeni kullanýcý bilgileri maindeki dizilere aktarýlýr
		*old_users_main = old_users;
		*new_users_main = new_users;
		
		fclose(inputFile);
	}
	
	//free islemleri
	free(buffer);
	free(temp);
	
	return 0;
	
}

//içine aldýðý kullanýcýnýn okuduðu kitaplarýn puan ortalamasýný döndüren fonksiyon (estimationda kullanýlan mean)
double getAveragePoint(struct user_info user, int book_count) {
	int sum = 0;
	int i;
	
	for(i = 0; i < book_count; i++) {
		sum += user.points[i];
	}
	
	return (double) sum / (double) user.count;
}

//similarity hesabýnda kullanýlan mean / icine aldýgý iki kullanýcýnýn ortak okudugu kitaplarýn, 1. kullanýcý bakýmýndan puan ortalamasýný hesaplayan fonksiyon
double getAveragePointV2(struct user_info user_1, struct user_info user_2, int book_count) {
	int sum = 0;
	int i;
	int common_count = 0;	//ortak kitap sayýsý
	for(i = 0; i < book_count; i++) {
		//kitap ortak ise
		if(user_1.points[i] != 0 && user_2.points[i] != 0) {
			common_count++;
			sum += user_1.points[i];
		}
	}
	return (double) sum / (double) common_count;	//return mean
}

//fonksiyon içerisine aldýðý userlarýn birbirlerine ne kadar benzediðini hesaplayýp döndürür
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
	
	//meanlar hesaplanýr
	avg_1 = getAveragePointV2(user_1, user_2, book_count);
	avg_2 = getAveragePointV2(user_2, user_1, book_count);

	//iki user'ýn da okudugu kitaplarý gez
	for(i = 0; i < book_count; i++) {
		//ilgili kitabý ikisi de okuduysa
		//pay ve paydalarý hesaplayarak formulu uygula
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

//pred fonksiyonu gerçeklenir. new user'a en benzer k adet old user'a göre, new user'ýn okumadýgý kitap (book_id'si verilen kitap) icin tahmini puan hesaplanýr
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

//içine aldýgý kullanýcýnýn okumadýgý tum kitaplara tahmin yapan fonksiyon
double **totalEstimations(struct user_info new_user, struct user_info *old_users, int *mostSimilars, int book_count, int k) {
	
	int i, j;
	int recommend_count = book_count - new_user.count;	//new user'ýn okumadýgý kitap sayýsý
	
	//ilk satýrý tahmini puanlarý, ikinci satýrý kitap indisini tutan double matris tanýmlanýr
	//sutun sayýsý new user'ýn okumadýgý kitap sayýsý kadardýr
	double **resultMatrix = (double**) calloc(2, sizeof(double*)); 
	for(i = 0; i < 2; i++) {
		resultMatrix[i] = (double*) calloc(recommend_count, sizeof(double));
	}
	//matrisin ilk satýrý 0'lanýr
	for(i = 0; i < recommend_count; i++) {
		resultMatrix[0][i] = 0.0;
	}
	j = 0;
	//ikinci satýra kitap kitap indisleri (id) leri atýlýr
	for(i = 0; i < book_count; i++) {
		if(new_user.points[i] == 0) {
			resultMatrix[1][j] = (double) i;
			j++;
		}
	}
	//okunmayan her bir kitap icin tahmini puan hesaplanýr
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
	int rec_id; //en fazla tahmini puana sahip okunmamis kitabýn id'sini tutan degisken
	double **estimate_results; //ilk satýrý tahmini puanlarý, ikinci satýrý okunmamýs kitaplarýn idlerini tutan matris
	
	
	//Verileri Tablo Seklinde yazdýran blok
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
		
		estimate_results = totalEstimations(new_users[i],old_users, similars, book_count, k);	//tahmini puanlar hesaplanýr
		
		
		//en fazla puana sahip okunmamýs kitap bulunur ve onerilir
		//okunmamýs kitap sayýsý 1'den fazla ise
		if(recommend_count > 1) {
			rec_id = findMaxV2(estimate_results[0], recommend_count);	//en fazla tahmini puana sahip okunmamýs kitabý bul
			//printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][rec_id]]);	//en fazla puana sahip okunmamýs kitap onerilir
		}
		else {
			rec_id = 0;
			//printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][0]]);	//en fazla puana sahip okunmamýs kitap onerilir
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
	int id; //oneri yapilacak new user'ýn idsi
	int k; //k degeri (en benzer kac adet eski kullanici sayisi)
	int *similars; //en benzer k eski kullanicinin indisleri(id)
	int recommend_count; //kullanicinin okumadigi ve kullaniciya onerilecek olan kitap sayisi
	int rec_id; //en fazla tahmini puana sahip okunmamis kitabýn id'sini tutan degisken
	char *user_name = (char*) calloc(NAME_SIZE, sizeof(char)); //oneri yapýlacak kullanici ismi
	double **estimate_results; //ilk satýrý tahmini puanlarý, ikinci satýrý okunmamýs kitaplarýn idlerini tutan matris
	
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
	
	recommend_count = book_count - new_users[id].count;	//kullanýcýnýn okumadýgý kitap sayýsý belirlenir
	
	//kullanýcýnýn okumadýgý kitaplar icin tahmini puanlarý ve kitap id'lerini tutan 2 satýrlý matris icin yer acilir
	estimate_results = (double**) calloc(2, sizeof(double*));
	for(i = 0; i < 2; i++) {
		estimate_results[i] = (double*) calloc(recommend_count, sizeof(double));
	}
	
	estimate_results = totalEstimations(new_users[id],old_users, similars, book_count, k);	//tahmini puanlar hesaplanýr
	
	//hesaplanan puanlara gore oneri kitaplarý ekrana yazdirilir
	printf("\nRecommendations for %s :\n",new_users[id].name);
	for(i = 0; i < recommend_count; i++) {
		printf("%d. recommend : %s with estimation % lf\n",(i+1), book_names[(int) estimate_results[1][i]], estimate_results[0][i]);
	}
	
	//en fazla puana sahip okunmamýs kitap bulunur ve onerilir
	//okunmamýs kitap sayýsý 1'den fazla ise
	if(recommend_count > 1) {
		rec_id = findMaxV2(estimate_results[0], recommend_count);	//en fazla tahmini puana sahip okunmamýs kitabý bul
		printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][rec_id]]);	//en fazla puana sahip okunmamýs kitap onerilir
	}
	else {
		printf("\nRecommended book for %s : %s\n",new_users[id].name, book_names[(int) estimate_results[1][0]]);	//en fazla puana sahip okunmamýs kitap onerilir
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
	int cont = 1; //kullanýcý istedigi surece arama yapmasýný saglayan while dongusune verilecek degisken
	int file_control;	//dosyanýn acilip acilamadigini kontrol eden degisken
	char **book_names;	//kitap isimlerini tutuan matris
	struct user_info *old_users, *new_users;	//eski ve yeni kullanici bilgilerini tutuan diziler
	
	
	//veriler dosyadan okunur
	file_control = readFile(&book_count, &old_users, &new_users, &old_user_count, &new_user_count, &book_names);
	//dosya okunamadýysa program sonlandýrýlýr
	if(file_control == 1) {
		return 0;
	}
		
	//yeni kullanicilarin eski kullanicilara olan benzerlikleri hesaplanir
	totalSimilarities(old_users, new_users, old_user_count, new_user_count, book_count);
	
	//kullanýcý istedigi surece donen while dongusu
	while(cont) {
		
		printf("Programi kullanici ozelinde calistirmak icin 1'e, tablo halinde tum kullanicilari gozlemlemek icin 0'a basiniz...\n");
		scanf("%d",&choice);
		
		//kullanýcýn secimine gore output ekrana yazdýrýlýr
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
