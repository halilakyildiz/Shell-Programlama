# Shell-Programlama
Linux ortamında çalışan basit bir komut yorumlayıcısı.
-C diliyle yazılmış bazı komutları işleyebilen (Örn. cat, ls, sleep, echo, man vb.) program.

-Make derleyiciyi indirmeniz gerekir.

-Programı ubuntu ortamına atıp, terminalden kaynak kodların bulunduğu dizine gidip "make" yazmanız yeterli olacaktır.

-Dizindeki "makefile" dosyası içindeki derleme komutları otomatikmen çalışacak ve program derlenecektir.

-Derleme bittikten sonra programı iki şekilde çalıştırabilirsiniz. İnteraktif (etkileşimli) ve batch (toplu).

İnteraktif modda ekranda bir promtp görüntülenir ve kullanıcıdan bir komut bekler. Girilen komutu yürütür ve kullanıcıya geri döner.
Batch modda ise program çalıştırılırken programa ikinci bir argüman verilir. Porgram bu argümanın içindeki komutları okur ve yürütür.

  >Porgamın interaktif modda çalışma şekkli : ./shell
  
  >Programın batch  modda çalışma şekli : ./shell batch.txt
  
Batch.txt burada bir text dosyasıdır ve örnek verilmiltir. Başka bir text dosyası ya da herhangi bir okunabilir dosya olabilir.

İnteraktif modda kullancı çeşitli komutları yürütebilir. Girilen her bir komut için fork() ile alt proses oluşturulur ve komut yürütülür. Böylece eşzamnlılık sağlanmış olur.
Diğer önemli  bir nokta ise birden fazla komut icrası. Kullanıcı tek satırda birden fazla komut girebilir. Burada komutları ';' karakteri ile ayırmalıdır.
";" ile ayrılan komutların her biri eş zamanlı olarak yürütülür ve çıktıları farklı sıralarda olabilir. 

Örnek için aşağıdaki çıktıyı inceleyebilirsiniz;

 >Komut : echo test_yazisi;echo test_yazisi2;echo test_yazisi3
 
 >Çıktı :  
 >tesyazisi2  
 >test_yazisi3
 >test_yazisi  
 >yada bunun diğer kombinasyonları olabilir.


