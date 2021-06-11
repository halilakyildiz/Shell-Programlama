#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ayristirma.h"
#include <wait.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include <pwd.h> 
#include <grp.h> 
#include <time.h> 

int commandBoyut; // çalıştırılacak komutları tutan dizinin boyutu
 char *prompt; // Komut ptomptu

// kullanıcıdan girdi alır
char *lsh_read_line(void) 
{
  char *line=NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line,&bufsize,stdin);
  return line;
}
// parametrede aldığı dizinin boyutunu bulur. Burada boyutu global commanBoyut değişkene atıyor
void c_boyut(char **c){
	int i;
	for(i=0;c[i]!=NULL;i++);
	
	commandBoyut=i;		
}

// Aldığı satırın boyutunu döndürür.
int SatirBoyut(char * line){
	int i;
	for(i=0;line[i]!='\0';i++);
	return i;
}


//Komutların içinde arama yapar.
int Search(char ** komut,char *aranan){
	for(int i=0;komut[i]!=NULL;i++){
		if(komut[i]==aranan)
			return i;
	}
	return -1;
}
// Tek komutu işletir.
int ExecuteCommends(char **args,int index){
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[index], args) == -1) {
      perror("lsh:execvp");
    }

    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process

    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

//Bütün komutlardan istenen komutu döndürür.
char** KomutGetir(char** komutlar,int komut_sirasi){
	int sonuc = Search(komutlar,";");
	int sayac =0;
	int n_virgul=0;// ";" sayısını tutar
	int n_virgul_index=0;
	if(komut_sirasi!=1){// İlk komut değilde başka komut istenirse
		for(int i=0;komutlar[i]!=NULL;i++){
			if(komutlar[i]==";"){//";" görürse girer
				n_virgul++;//";"adedini artırır
				n_virgul_index=i;// ";"indeksini alır.
			}
			if(n_virgul==komut_sirasi-1)//İstenen komuttan önceki ";" e geldiyse çıkar.
				break;
		}
	}
	char **new_ =malloc(commandBoyut * sizeof(char *));
	if(komut_sirasi==1){//İlk komut istenirse
		for(int i=0;komutlar[i]!=NULL;i++){//komutları dolaşır
			if(komutlar[i]!=";")//";" değilse alır
				new_[sayac++]=komutlar[i];
			else{//";" bulursa komut sonuna gelinmiştir.
				new_[sayac]=NULL;
				break;
			}// eğer son komuta geldiyse dizinin sonunu NULL yap çünkü for bitiyor.
			if(komutlar[i+1]==NULL)	
				new_[sayac]=NULL;
		}
	}
	else{// Yukardaki if de bulunan ";" den başlayark komutu alır.
		for(int i=n_virgul_index+1;komutlar[i]!=NULL;i++){//";" den başlar.
			if(komutlar[i]!=";")//";" değilse alır
				new_[sayac++]=komutlar[i];
			else{//";" bulursa komut sonuna gelinmiştir.
				new_[sayac]=NULL;
				break;
			}// eğer son komuta geldiyse dizinin sonunu NULL yap çünkü for bitiyor.
			if(komutlar[i+1]==NULL)	
				new_[sayac]=NULL;
		}
	}
	return new_;//"İstenen komutu döndürür.
}

// birden fazla komutu parelel icra eder.
void CokluKOmutIcrasi(int komut_sayisi,char** komutlar){
	
	int numberOfChildren = komut_sayisi;// komut sayısı çocuk sayısı olucaktır
	pid_t *childPids = NULL;
	pid_t p;
	/* Allocate array of child PIDs: error handling omitted for brevity*/ 
	childPids = malloc(numberOfChildren * sizeof(pid_t));//komut sayısı kadar çocuk oluşturulur.

	/* Start up children *//*çocuklar başlatılır.*/
	for (int ii = 0; ii < numberOfChildren; ii++) {
	
	   if ((p = fork()) == 0) {
	/*Burada her çocuk kendi komutunu ister ve icra eder.KomutGetir metoduna bütün komutları
	ve kaçıncı komutu istediğini gönderir. İstediği komut gelince execvp ile icra eder.
	Her komut parelel çalışır ve program he kapatıp tekrar aynı komut girildiğine çıktı sıraları
	değişkenlik gösterecektir.Bu parelelliğin göstergisidir.*/
	      char **new_ = KomutGetir(komutlar,ii+1);
		 if (execvp(new_[0], new_) == -1) {
		      perror("lsh:execvp");//execvp hatası
		    }

    exit(EXIT_FAILURE);	
	   }
	   else {
	      childPids[ii] = p;// Parent child ı tutar.
	   }
	}

	/* Wait for children to exit */ /*Çocukların bitmesi beklenir.*/
	int stillWaiting;
	do {
	   stillWaiting = 0;
	    for (int ii = 0; ii < numberOfChildren; ++ii) {
	       if (childPids[ii] > 0) {
		  if (waitpid(childPids[ii], NULL, WNOHANG) != 0) {
		     /* Child is done */
		     childPids[ii] = 0;
		  }
		  else {
		     /* Still waiting on this child */
		     stillWaiting = 1;
		  }
	       }
	       /* Give up timeslice and prevent hard loop: this may not work on all flavors of Unix */
	       sleep(0);
	    }
	} while (stillWaiting);

	/* Cleanup */
	free(childPids);//Bütün çocuklar bitince çocuk dizisi silinir.
}

// Tekli komut icrasını yöndendirir.
// Burdaki amaç eğer komutta
void RunCommand(char ** c){	
	int x=0;
	if(c[0]==NULL || c[0] =="")
		return;		
	if(strcmp(c[x],"quit")==0) // eğer ilk yerleşik komuta,yani quit komutuna eşitse sonlandırır
			exit(1);
	else
		ExecuteCommends(c,0);
	

}

// parametrede aldığı komutları ; var ise bölerek işletmeye gönderir
void split_command(char **cm){
	int sayac_c=0;
	if(cm[0][0]=='\0') // Eğer dizide işletilecek komut yok ise geri döndürür
		return;
	int sonuc = Search(cm,";"); // Komutlarda ";" arar.
		/* Eğer ";" varsa birden fazla komut icrası vardır. Ve parelel işletmek gerekir.
		Bu yüzden ayrı metoda gönderiyoruz*/
		if(sonuc!=-1){
			char ** new_ = malloc(commandBoyut * sizeof(char *));
			int sayac=0;
			int kontrol=0;
			/* Eğer komut dizisinde "quit" komutu varsa bu komut çalışmadan önce
			diğer komutlar çalışmalı ve program ondan sonra kapanmalı.
			Bu yüzden "quit" komutunu diziden alıyoruz.*/
			for(int i=0;cm[i]!=NULL;i++){// Komutları dolaşır.
								
				if(!strcmp(cm[i],"quit") || !strcmp(cm[i],"QUIT")){//"quit" var ise
					kontrol=1;//printf("girdi");
					if(i==0){//"quit" başta olabilir.Örn(quit;ls;ls)
						for(int k=i+2;cm[k]!=NULL;k++)
							new_[sayac++]=cm[k];
						new_[sayac++]=NULL;
						break;
					}
					else if(cm[i+1]==NULL){//"quit" sonda olabilir.Örn(ls;ls;quit)
						for(int k=0;k<i-1;k++)
							new_[sayac++]=cm[k];
						new_[sayac++]=NULL;
						break;
					}
					else{//"quit" arada olabilir.Örn(ls;quit;ls)
						for(int k=0;k<i;k++){
							new_[sayac++]=cm[k];
							if(k+1==i){
							for(int j=k+3;cm[j]!=NULL;j++)
								new_[sayac++]=cm[j];
							}
						}
						new_[sayac++]=NULL;
						break;
					}
				}
					
			}
			if(kontrol){// "quit" komutu tespit edildiyse yeni diziyi çalıştırır.
				free(cm);//eski dizi silinir.				
				int adet=0;
				for(int i=0;new_[i]!=NULL;i++){// Komutları dolaşır.
					if(!strcmp(new_[i],";"))// ";" varsa arttırır.
						adet++;	
				}
				CokluKOmutIcrasi(adet+1,new_);// Parelel icraya gönderir.
				exit(1);// "quit" tespit edildiği için bütün komutlar çalıştıktan sonra kapanır.
			}
			
			//"quit" yok ise normal devam ederç
			int adet=0;
			for(int i=0;cm[i]!=NULL;i++){// Komutları dolaşır.
				if(cm[i]==";")// ";" varsa arttırır.
					adet++;
			}
			/* Parelel işletmek üzere gönderir. Komut adedi bize kaç adet child proses
			oluşturacağımızı verir.*/
			CokluKOmutIcrasi(adet+1,cm);
		}
		else
			RunCommand(cm);//tek komut varsa icra etmeye gönderir.
}

// Gelen komut dizisindeki boşlukları siler.
char ** Clear_Space(char ** c){
	char ** new_ = malloc(50 * sizeof(char *));
	int sayac=0;
	for(int i=0;c[i]!=NULL;i++){
		if(strcmp(c[i]," ")!=0)
			new_[sayac++]=c[i];
	}
	new_[sayac]=NULL;
	free(c);
	return new_;
}

// Ayn şekilde dosyayı okur ve satır sayısını döndürür.
int DosyaSatirSayisiBul(char *dosya_konum){
	int sayac=0;
  FILE *dosya = fopen(dosya_konum, "r");
	char *str;
	if(dosya==NULL)
	{                 
		 //printf("%s\n",dosya_konum);
		 printf("Batch dosyası mevcut değil veya açılamıyor.\n");
                 exit(1);
	}
	
  while(! feof(dosya)) {
	str = malloc(1000 * sizeof(char));
    if( fgets (str, 1000, dosya)!=NULL )
		sayac++;
	free(str);
  }
  fclose(dosya);
return sayac;
}

// Bacth modda girilen dosyayı okur.
void DosyaOku(char* dosya_konum){
	/* Önce dosyadaki satır sayısını bulur.
	 Önce satır sayısını bulmamın sebebi program garip bir şekilde sonsuz döngüye girerek
	 son komutu işliyor sürekli. Bunu önce dosyadaki satır sayısını bularak çözebildim.*/
	int satir_sayisi=DosyaSatirSayisiBul(dosya_konum);
	int sayac=0;
  	FILE *dosya = fopen(dosya_konum, "r");//! feof(dosya
	char *str;// satırı tutat
	char **args=NULL;// komut dizisi
  while(sayac<satir_sayisi) {// satır sayısı kadar döner.
	str = malloc(1000 * sizeof(char));
    if( fgets (str, 1000, dosya)!=NULL ){// satırı okur.
			int boyut = SatirBoyut(str);// boyutunu getirir.
			if(boyut<=512){
				printf("\n%s",str);// Önce satırı yazar.
				args = satir_kes(str); // Parsing yapar.
				args = Clear_Space(args);
				c_boyut(args);
				if(args[0]!=NULL)
	    			split_command(args);// işlemeye gönderir.
	    			free(args);
				args=NULL;
	
			}
			else
				printf("Çok uzun bir komut satırı (bu proje için '\n' dahil 512 karakterden fazla). !\n");
			free(str);
			
  	}
	sayac++;
}
  fclose(dosya);

}
// kullanıcıdan sürekli komut bekler ve işletmeye gönderir
void loop(void)
{
	char **args=NULL;// Komutları tutar
	char *line;// girilen satırı tutar
 
  do {
	printf("\033[1;36m");
    	printf("\n%s:~$ ",prompt); // Komut prompt rengi
	printf("\033[0m");
	fflush(stdout);

	line = lsh_read_line();//Kullanıcıdan girdi alır.
	if(line[0]=='\0'){ // Eğer Ctrl+D ise bu '\0' demektir ve programı kapatır.
		printf("\n");
		exit(0);
	}
	int boyut = SatirBoyut(line);// Satırın uzunluğunu bulut.
	// Eğer 512 den büyükse hata verir ve devam eder.
	if(boyut<=512){
	    	args = satir_kes(line); // Ayrıstıma dosyasındaki metotları çağırarak parsing işlemleri başlar.
		args = Clear_Space(args);// Komut dizisindeki boşlukları siler.
		c_boyut(args);// Komutların adedini bulur.
		if(args[0]!=NULL) // Eğer komut dizisi boş değilse işlemeye gönderir.
	    	split_command(args);
	    free(line); // satırı siler.
		line=NULL;
	    free(args);// Komutları siler.
		args=NULL;
	}
	else
		printf("Çok uzun bir komut satırı (bu proje için '\n' dahil 512 karakterden fazla).\n");
  } while (1);
}


int main(int argc, char *argv[], char *envp[])
{
	prompt = "MY-SHELL "; // komut promtu.
	// interaktif mi yoksa batch moddamı çalıştırıldı kontrolü.
	if(argc>1)
		DosyaOku(argv[1]);
	else
		loop();
	
	return 0;
}

