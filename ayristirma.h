#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ctype.h"

// parametrede aldığı satırda verilen k karakterini arar bulursa konumunu döndürür,bulamazsa -1 döner
int find_char(char *l,char k){
	int sonuc =-1;
	for(int i=0;l[i]!='\0';i++){
		if(l[i]==k){
			sonuc=i;	
			break;
		}
	}
	return sonuc;
}

// satırın sonundaki enter(\n) ı siler
char *deleteEnter(char *line){
	int i;
	for(i=0;line[i]!='\0';i++);				
	if(line[i-1]=='\n')
		line[i-1]='\0';
return line;
}


// Char** dizisinin son elemanı verilen karakter ise sile.
// aldığı dizinin son elemanı verilem karakterle uyuşuyorsa siler.
char **son_eleman_sil(char **c,char *k){
	int i;
	for(i=0;c[i]!=NULL;i++);// dizinin sonuna gelir
	if(c[i-1]==k)
		c[i-1]=NULL;
		
	return c;
}

// aldığı satırı verilen karaktere göre bölüp döndürür
// Ama aradaki karakteri silmez. Onuda tutarak döndürür.
// Örn(line: ls;ls;echo test -> ls ; ls ; echo test ;)şeklinde.
char **get_input(char *input,char *sg) {
	int size =1000;
    char **command = malloc(size * sizeof(char *));
    char *separator = sg;
    char *parsed;
    int index = 0;
    int sira=1;

    parsed = strtok(input, separator);
	//printf("%s\n",parsed);
    while (parsed != NULL) {
        command[index] = parsed;
        index++;
	command[index]=separator;
	index++;
        parsed = strtok(NULL, separator);
    }
    command[index] = NULL;
    return command;
}


//verilen diziyi yine diğer verilen diziye yazar
int sira=0;
char **diziye_yaz(char **run,char ** c){
	for(int i=0;c[i]!=NULL;i++){
		run[sira]=c[i];
		sira++;
	}
	return run;
}

// satir_kes metodu satırı ; e kadar parçalar ve komut_ata ya gönderşr
char **komut_ata(char** cm,char *c){
	char **temp;
	temp = get_input(c," ");// Gelen her bir komutu boşluğa göre parçalar tekrar.
	temp = son_eleman_sil(temp," ");// Yine sondaki bir elemanı siler.
	cm = diziye_yaz(cm,temp);// Son halini genel komut dekişkenine yazar.
	free(temp);
return cm;// son halini döndür
}

// satiri ; e göre parçalar.Satırdaki bütün komutları char** dizisi olarak döndürür.
char **satir_kes(char *ln){
	char **args=malloc(1000 * sizeof(char *));// bütün komutları tutar.("ls ; echo test ; cat bash.txt")
	ln = deleteEnter(ln); // Satırın sonundaki "\n" karakterini siler.
	char **cmmnd=NULL;
	int sonuc = find_char(ln,';');// satırda ";" arar bulursa indeksini bulamazsa -1 döndürür.
	if(sonuc!=-1){
		cmmnd = get_input(ln,";");// ";" göre parçalar
		cmmnd=son_eleman_sil(cmmnd,";");//parçalamadan sonra ";" silmez ve sonuna bir adet daha ";" bulunur. Bunu sileriz.
		for(int i=0;cmmnd[i]!=NULL;i++){ // ";" e kadar olan komutları args değişkenine yazmaya gönderir.
			args = komut_ata(args,cmmnd[i]);
		}
	}
	else{
		args = komut_ata(args,ln);//Eğer ";" yoksa parçalamadan gönderir
	}
	
	free(cmmnd);
	cmmnd=NULL;// hafızadan kaldırırız
	args[sira]=NULL;// Komut dizisinin on elemanı NULL olmalı.
	sira=0;
	return args;
}
