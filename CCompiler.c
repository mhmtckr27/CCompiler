//    _    _                  _    _____      _    _      
//   | |  | |                (_)  / ____|    | |  (_)     
//   | |__| | __ _ _   _ _ __ _  | |     __ _| | ___ _ __ 
//   |  __  |/ _` | | | | '__| | | |    / _` | |/ / | '__|
//   | |  | | (_| | |_| | |  | | | |___| (_| |   <| | |   
//   |_|  |_|\__,_|\__, |_|  |_|  \_____\__,_|_|\_\_|_|   
//                  __/ |                                 
//                 |___/                                  
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include <ctype.h>
int main()
{
	FILE *fp1;
    char p_code[LINESIZE];
	char *data_array,filep_name[100],filec_name[100],output_file[100],*sysout;
	int file_size;
	DATABASE *ptr;

	system("COLOR F5");
	printf(">PseudocodeConverter\n");
	printf("Pseudo kodun bulundugu dosya ismini giriniz: ");
	scanf("%s",filep_name);
	printf("C kodunun yazilacagi dosya ismini giriniz: ");
	scanf("%s",filec_name); //Pseudo kod dosyasý ve C kodu yazýlacak dosya ismi alýndý.
	fp1=open_file(filep_name,"r"); //Pseudo kod dosyasý açýldý.
	file_size=find_file_size(fp1);	// Dosya boyutu bulundu.
	rewind(fp1);	//Dosya baþýna dönüldü.
	printf("Pseudocode dosyasi okunuyor..\n");
	fgets(p_code,LINESIZE-1,fp1);	//Pseudo kod dosyasýnýn ilk satýrý p_code stringine alýndý.
	data_array=(char*)malloc(file_size);	//data_array stringine yer açýldý
	strcpy(data_array,p_code);
	while(fgets(p_code,LINESIZE-1,fp1))
	{
		strcat(data_array,p_code);		//Bütün pseudo kod dosyasý data_array stringine alýndý.
	}
	fclose(fp1);
	printf("Pseudocode dosyasi basariyla okundu.\n");
	isDeclared(data_array);		//data_array hazýr, sembol tablosu oluþturmak için isDeclared çaðrýlýyor.
	printf("Degiskenlerle ilgili bilgileri tutan symboltable, linkli liste olarak olusturuldu.\n");
	arrange_tokens();	//Sembol tablosu oluþturuldu. 1.3.10. kýsýmdaki olaylar için çaðrýlýr.
	printf("\n\n");
	printf("\tPTR\t\tPROC\tTOKEN\tIDENT.\tDTYPE\tISARR?\tISPTR?\tNEXT\n");
	for(ptr=head;ptr!=NULL;ptr=ptr->db_next)
		printf("\t%d\t%s\t%s\t%s\t%s\t%d\t%d\t%d\n\n",ptr,ptr->proc,ptr->token,ptr->identifier,ptr->data_type,ptr->isarray,ptr->ispointer,ptr->db_next);
	system("PAUSE");
	print_file(filep_name,filec_name);	//Sembol tablosu hazýr, C kodu dosyaya yazýlmaya baþlanýr
	printf("C kodu dosyaya yazildi.\n");
	printf("Olusturulacak exe adini giriniz: ");
	scanf("%s",output_file);
	sysout=(char*)malloc(strlen(filec_name)+strlen(output_file)+10);
	strcpy(sysout,"gcc ");
	strcat(sysout,filec_name);
	strcat(sysout," -o ");
	strcat(sysout,output_file);
	system(sysout);	//Oluþturulan c kodu dosyasý gcc ile derlenir.
	printf("Executable dosya olusturuldu.");
	system("PAUSE");
	system(output_file);	//Derleme sonu oluþturulan dosya çalýþtýrýlýr.
	return 1;
}
FILE *open_file(char *file_name,char* file_mode)
{
	//Dosya Açma fonksiyonu
	FILE *fp1;
	if((fp1=fopen(file_name,file_mode))==NULL)
	{ 
		printf("'%s' dosyasi '%s' modunda acilamadi.",file_name,file_mode);
		system("PAUSE");
		exit(1);
	}	
	printf("'%s' dosyasi, '%s' modunda acildi.\n",file_name,file_mode);
	return fp1;
}
int find_file_size(FILE *fp1)
{
	//Dosya Boyutu bulan fonksiyon
	fseek(fp1,0,SEEK_END);
	return ftell(fp1);
}
void isDeclared(char* data_array)
{
	DATABASE *ptr,*tmptr;
	char *tokenlimiter=" $.\"()<>\n\t";
	char *datalimiter="_[]";
	char *token,*ident,*dtype,*tmptoken,new_Proc=FALSE,Proc[12];
	static char ifarray=FALSE;
	int k=0,array_size=100;
	token=strtok(data_array,tokenlimiter);
	if(!strcmp(token,"PROCEDURE")) //Ýlk satýrda fonksiyon baþlangýcý mý var?
	{
		new_Proc=TRUE;
	}
	if(strchr(token,'[')) //Yoksa bir dizi mi?
	{
		ifarray=TRUE;
	}
	tmptoken=(char*)malloc(strlen(token)+1); //token'ý kaybetmemek için oluþturulan
	strcpy(tmptoken,token);			//tmptoken'a token'ý alýyoruz
	if(!_stricmp(token,"DEFINE"))	//DEFINE geldiyse
	{								//tokenize et
		ident=strtok_2(token,datalimiter);	
		dtype=strtok_2(NULL,datalimiter);
		array_size=atoi(strtok_2(NULL,"]"));	//stringi int'e dönüþtür.
	}
	else
	{
		ident=strtok_2(token,datalimiter);	//DEFINE gelmediyse sadece tokenize et
		dtype=strtok_2(NULL,datalimiter);
	}
	goto label2;		//ilk çaðrýmda label2 ye git.
label1:
	while(token=strtok(NULL,tokenlimiter)) //satýr sonuna gelene kadar tokenize etmeye devam.
	{
		array_size=100;		//dizi boyutu default olarak 100
		if(new_Proc)		//Satýrda fonksiyon baþlýyorsa new_Proc set edilmiþti.
		{
			strcpy(Proc,token);	//Proc stringine fonksiyon adýný alýyoruz
			new_Proc=FALSE;	//new_Proc default olarak FALSE
		}
		if(!strcmp(token,"PROCEDURE"))	//yeni fonksiyon geldiyse new_Proc set.
		{
			new_Proc=TRUE;
			k++;		//Fonksiyon sýrasýný tutar.
		}
		if(strchr(token,'['))	//[ varsa dizidir.
		{
			ifarray=TRUE;
		}
		strcpy(tmptoken,token);	//token'ý sonradan kullanacaðým, deðeri deðiþeceði için tmptoken'a alýyorum.
		if(!_stricmp(token,"DEFINE"))//DEFINE mý geldi?
		{	
			token=strtok(NULL," \n");	
			ifarray=TRUE;	
			ident=strtok_2(token,datalimiter);
			dtype=strtok_2(NULL,datalimiter);
			array_size=atoi(strtok_2(NULL,"]"));	//dizi boyutunu ayarlýyoruz
		}
		else
		{
			ident=strtok_2(token,datalimiter);	//dizi deðilse normal devam.
			dtype=strtok_2(NULL,datalimiter);
		}
		for(tmptr=head;tmptr!=NULL;tmptr=tmptr->db_next)//Deðiþken daha önce tanýmlandý mý?
		{
			if(!strcmp(ident,tmptr->identifier))
			{
				if(!strcmp(Proc,tmptr->proc))
				{
					ifarray=FALSE;
					goto label1;	//Tanýmlandýysa label1 e gidip yeni token'ý al.
				}
			}
		}
label2:	if(!dtype)	//Ýlk çevrimde direkt buraya atlýyordu.
		{			//Token, deðiþken deðilse dtype null olacak.
			goto label1;	//label1e gidip yeni token al
		}
		switch((int)*dtype)	//switch'in içindeki case, deðiþken tiplerinin
		{					//ilk harfinin ASCII'si
			case 99 :   //CHAR
				ptr=arrange_bools(ident,"char",tmptoken,&ifarray);
				uses_vars[k][0]=TRUE;
				break;
			case 100 :  //DOUBLE
				ptr=arrange_bools(ident,"double",tmptoken,&ifarray);
				uses_vars[k][1]=TRUE;
				break;
			case 102 :  //FLOAT ve FILE *
				if(strlen(dtype)==4)		//FILE * mý?
				{
					ptr=arrange_bools(ident,"FILE",tmptoken,&ifarray);
					uses_vars[k][2]=TRUE;
				}
				else						//FLOAT mý?
				{
					ptr=arrange_bools(ident,"float",tmptoken,&ifarray);
					uses_vars[k][3]=TRUE;
				}
				break;
			case 105 :   //INT
				ptr=arrange_bools(ident,"int",tmptoken,&ifarray);
				uses_vars[k][4]=TRUE;
				break;
			case 108 :  //LONG
				if(strchr(dtype,'d'))	//LONG DOUBLE MI?
				{
					ptr=arrange_bools(ident,"long double",tmptoken,&ifarray);
					uses_vars[k][6]=TRUE;
				}
				else					//LONG MU?
				{
					ptr=arrange_bools(ident,"long",tmptoken,&ifarray);
					uses_vars[k][5]=TRUE;
				}
				break;
			default :
				goto label1; //Deðiþken deðilse buraya düþer. label1 e gidip yeni token al.
		}		
		if(strchr(dtype,'p'))//Deðiþken tipi, p içeriyorsa pointerdýr.
		{
			ptr->ispointer=TRUE;
		}
		else
		{
			ptr->ispointer=FALSE;
		}
		ptr->identifier=ident;	//deðiþken adýný ayarla.
		if(ifarray)		//diziyse, boyutunu ayarla.
		{
			ptr->isarray=array_size;
			ifarray=FALSE;
		}
		strcpy(ptr->proc,Proc);	//hangi fonksiyonda geçtiðini ayarla
		insert(ptr);
	}
	return;
}
DATABASE *create_list_element(unsigned int strsize, unsigned int dtypesize, unsigned int tokensize)
{
	DATABASE *ptr;
	ptr=(DATABASE*) malloc(sizeof(DATABASE));	//structa yer aç
	ptr->identifier=(char*)malloc(strsize+1);	//Deðiþken adýna 
	ptr->data_type=(char*)malloc(dtypesize+1);	//Deðiþken tipine
	ptr->token=(char*)malloc(tokensize+1);	//Deðiþken token'ýna yer aç.
	if(ptr==NULL)
	{
		printf("Creating list element with malloc failed.\n");
		exit(1);
	}
	ptr->db_next=NULL;
	ptr->isPara=FALSE;
	return ptr;
}
void insert(DATABASE *e)
{
	DATABASE *ptr;
	int i;
	if(head==NULL)	//Ýlk elemansa
	{
		head=e;	//baþ eleman olarak ayarla.
		return;
	}
	for(i=0;i<USES_VARS_SIZE;i++)	//Tüm deðiþken tipleri için dön.
	{
		if(_stricmp((e->data_type),vars[i])==0)	//Aradýðým deðiþken tiplerinin
		{							//tutulduðu alana kadar buraya girmeyecek.
			if(i==4)		//int'se en baþta tutulacak.
			{
				e->db_next=head;
				head=e;
				return;
			}	//int deðilse, ilgili deðiþken tipinin en son elemaný olarak eklenir.
			for(ptr=head;((ptr->db_next!=NULL)&&(ptr->db_next->data_type!=vars[i]));ptr=ptr->db_next);
			e->db_next=ptr->db_next;
			ptr->db_next=e;
			return;
		}
	}
}
void arrange_tokens()
{
	DATABASE *ptr;
	for(ptr=head;ptr!=NULL;ptr=ptr->db_next)	//En baþtan baþla, sona kadar dön
	{
		strcpy(ptr->token,ptr->identifier);	//token'a deðiþken adýný al
		switch((int)*ptr->data_type) //deðiþken tipine göre ilgili
		{						//uzantýyý ekle.
			case 70:	//FILE *
				strcat(ptr->token,"_file");
				break;
			case 99:	//CHAR
				strcat(ptr->token,"_c");
				break;
			case 100:	//DOUBLE
				strcat(ptr->token,"_d");
				break;
			case 102:	//FLOAT
				strcat(ptr->token,"_f");
				break;
			case 105:	//INT
				strcat(ptr->token,"_i");
				break;
			case 108:	//LONG
				if(strlen(ptr->data_type)==11)
				{		//long double olma durumunu kontrol et.
					strcat(ptr->token,"_ld");
				}
				else
				{
					strcat(ptr->token,"_l");
				}
				break;
		}
		if(ptr->ispointer)	//pointersa p uzantýsýný da ekle
		{
			strcat(ptr->token,"p");
		}
	}
}
char *strtok_2(char *s1, const char *delimit)
{
    static char *lastToken_2 = NULL; /* UNSAFE SHARED STATE! */
    char *tmp;
    /* Skip leading delimiters if new string. */
    if ( s1 == NULL ) {
        s1 = lastToken_2;
        if (s1 == NULL)         /* End of story? */
            return NULL;
    } else {
        s1 += strspn(s1, delimit);
    }
    /* Find end of segment */
    tmp = strpbrk(s1, delimit);
    if (tmp) {
        /* Found another delimiter, split string and save state. */
        *tmp = '\0';
        lastToken_2 = tmp + 1;
    } else {
        /* Last segment, remember that. */
        lastToken_2 = NULL;
    }
    return s1;
}
void print_file(char *filep_name, char *filec_name)
{
	FILE *fp1,*fp2;	
	DATABASE *ptr;
	char *token,def=FALSE;
	char p_code[LINESIZE], Proc[12],new_Proc=FALSE;
	static char  iffunc=FALSE;
	int i,k,t=-1,tab_counter=0,open_bracelet=0,close_bracelet=0,EOL=0;
	res_keywords();
	fp1=open_file(filep_name,"r");	//pseudo kod dosyasý aç.
	fp2=open_file(filec_name,"w+");	//c kodu yazýlacak dosyayý aç.
	fputs("#include <stdio.h>\n",fp2);
	fgets(p_code,LINESIZE-1,fp1);	//ilk satýrý al.
	if(_stricmp("PROCEDURE MAIN()\n",p_code))	//main var mý?
	{		//Yoksa label4 e git.
		goto label4;
	}
	else
	{
		poss[t+1].pos=ftell(fp2);	//fonksiyon baþlangýcýnýn pozisyonu
		strcpy(poss[t+1].funcname,"main()");//fonksiyon adý
		fputs("void main()       \n",fp2);//main baþlangýcýný yaz.
		strcpy(Proc,"MAIN");//þuanki fonksiyon main.
	}
	
label3:	//yeni fonksiyon baþlangýcý gelirse buraya atlar.
	t++;	//fonksiyon sýrasýný tutan deðiþken artar.
	fgets(p_code,LINESIZE-1,fp1);	//yeni satýrý al
	if(!_stricmp("BEGIN\n",p_code))	//BEGINse { koy
	{
		fputs("{\n\t",fp2);
		tab_counter++;	//indentasyon ayarlamalarý.
		open_bracelet++;
	}
	else
	{
		printf("Mainden sonra suslu parantez eksik!!");
		system("PAUSE");
		exit(1);
	}
	print_Vars(fp2,Proc,t);	//t. sýradaki fonksiyona ait deðiþkenleri yaz.
	new_Proc=FALSE;

	while((!new_Proc)&&(fgets(p_code,LINESIZE-1,fp1)))//yeni fonksiyon gelene ya da
	{						//dosya sonuna gelene kadar yaz.
		iffunc=FALSE;	//default olarak bunlar false olmalý.
		iffunccall=FALSE;
label4:	//ilk satýrda mainden farklý fonksiyon varsa buraya atlar.
		token=strtok(p_code," \t\n");
		if(!strcmp(token,"PROCEDURE"))	//yeni fonksiyon baþlangýcý mý geldi?
		{
			new_Proc=TRUE;	
			if(!strcmp(Proc,"MAIN"))	//önceki fonksiyon mainse, system pause yaz.
			{							//
				fseek(fp2,-3,SEEK_CUR);	//
				putc('\t',fp2);			//
				fputs("system(\"PAUSE\");",fp2);
				putc('\n',fp2);			//
				putc('}',fp2);			//
				putc('\n',fp2);			//------------------------------------------
			}
			poss[t+1].pos=ftell(fp2);	//pozisyonu structa al.
		}
		i=0;	//while için hazýrlýk.
		while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
		{	//reserved word structýndaki eleman sayýsýna gelene kadar
			i++;//veya tokenlar eþleþene kadar dönecek ve i++ olacak.
		}
		switch(i)	//en son i ne kalmýþsa ona göre ilgili
		{			//case'e girecek.
			case KEYWORDS_SIZE:	//eðer i, boyuta eþitse, reserved word deðildir.
				ptr=head;	//deðiþken midir diye bakýlýr.
				while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
				{	//struct sonuna gelene ya da tokenlar eþleþene kadar ptr'ý next yap.
					ptr=ptr->db_next;
				}
				switch((int)ptr)	//pointer deðerine göre;
				{
					case 0:	//eðer NULL'sa dizi olabilir.
						if(strchr(token,'['))
						{
							if(!strchr(token,']'))
							{
								printf("Brackets does not match!!\n");
								system("PAUSE");
							}
							isArray(fp2,token);	//diziyse isArray fonksiyonu çaðrýlýr.
						}
						else if(strchr(token,']'))
						{
							printf("Brackets does not match!!\n");
							system("PAUSE");	
						}
						else	//dizi deðilse, reserved word deðilse, deðiþken deðilse
						{	//function call olur çünkü satýr baþýndaki ilk token, sayý olamaz.
							fputs(token,fp2);	//buraya girer ve token neyse o yazýlýr dosyaya.
							if(*token=='\"')
							{
								putc(' ',fp2); //Dizi,keyword,datatype olmayan tokenlardan sonra bosluk olacak mi?
							}
							iffunccall=TRUE;	//iffunccall set,
							iffunc=FALSE;		//iffunc false edilir ve,
							isFunc(fp2,token,Proc);	//isFunc çaðrýlýr.
						}
						break;
					default:
						if(ptr->ispointer)	//ptr, pointersa
						{			//ve char deðilse
							if(ptr->data_type!="char")
							{		//* yazýlýr.
								putc('*',fp2);
							}
						}
						fputs(ptr->identifier,fp2);	//deðiþken adý yazýlýr.
						fputs(" ",fp2);			//okunabilirlik için boþluk koyulur.
						break;
				}
				break;
			case 0:	//FONKSIYON MU ?
				fputs(keywords[i].c_code,fp2);
				iffunc=TRUE;
				iffunccall=FALSE;
				token=strtok(NULL,"("); //fonksiyon adý alýnýr
				strcpy(Proc,token);	//Proc'a atanýr
				strcpy(poss[t+1].funcname,Proc);//pozisyonu alýnýr
				if(!_stricmp(token,"MAIN"))	//main'se dosyaya yazýlýr.
				{
					fputs("main",fp2);
				}
				else
					fputs(token,fp2);	//deðilse direkt basýlýr.
				putc('(',fp2);
				putc(' ',fp2);	//isFunc yapýsý gereði ('dan sonra bir boþluk olmalý.
				isFunc(fp2,token,Proc);	//isFunc çaðrýlýr
				break;
			case 1:
				fputs(keywords[i].c_code,fp2);	//1=BEGIN
				tab_counter++;		//tab sayýsý artmalý
				open_bracelet++;//süslü parantez açma sayýsý artar
				for(k=0;k<tab_counter;k++)
				{
					fputs("\t",fp2);	//gerekli tablar koyulur
				}
				break;
			case 2:	//2=END, tablar koyulmuþtu. 1 tab geriye yazýlmalý.
				fseek(fp2,-1,SEEK_CUR);	//1 karakter geri git.
				fputs(keywords[i].c_code,fp2);//END yaz.
				tab_counter--;	//tablarýn sayýsý azalýr.
				close_bracelet++;//süslü parantez kapatma sayýsý artar.
				for(k=0;k<tab_counter;k++)
				{
					fputs("\t",fp2);	//gerekli tablar koyulur.
				}
				break;
			case 12:	//12=FOR
				////FOR icin ozel durum olustur.  for ( i = 0; i < n; i++ ) mesela.
				i=for_loop_exception(fp2, token);
				break;
			case 20: //20=PRINT_LINE 
				//PRINT_LINE için özel durum oluþtur.
				GetVal_PrintLine(fp2, token, i);
				break;
			case 21:  //21=GET_VAL
				//GET_VAL için özel durum oluþtur.
				GetVal_PrintLine(fp2, token, i);
				break;
			case 3://3=RETURN
				fputs(keywords[i].c_code,fp2); //return yaz ve,
				return_type(fp2,token,poss[t].pos);//ilgili fonksiyonu çaðýr.
				break;
			case 22:  //22=DEFINE geldiyse, ekrana yazýlmayacak,
				while(token=strtok(NULL," \n"));//sadece deklarasyon kýsmýnda,
				def=TRUE;	//bir farklýlýk olacak ve bu da def deðiþkeni,
				break;		//aracýlýðýyla olacak.
			default:	//diðer reserved wordler geldiyse de,
				fputs(keywords[i].c_code,fp2);//direkt karþýlýðýný yaz.
				//putc(' ',fp2);//Satir basinda ilk alinan reserved wordden sonra bosluk olacak mi?
				break;
		}
		while(token=strtok(NULL," \t\n"))//Alýnan satýrýnýn sonuna gelene kadar döner.
		{				
			i=0;//while'a hazýrlýk
			while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
			{//reserved word structýndaki eleman sayýsýna gelene kadar
				i++;//veya tokenlar eþleþene kadar dönecek ve i++ olacak.
			}
			switch(i)	//en son i ne kalmýþsa ona göre ilgili
			{			//case'e girecek.
				case KEYWORDS_SIZE:	//eðer i, boyuta eþitse, reserved word deðildir.
					ptr=head;	//deðiþken midir diye bakýlýr.
					while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
					{	//struct sonuna gelene ya da tokenlar eþleþene kadar ptr'ý next yap.
						ptr=ptr->db_next;
					}
					switch((int)ptr)
					{		//pointer deðerine göre;
						case 0:	//Eðer null'sa
							if(strchr(token,'['))
							{		//Dizi olabilir.
								if(!strchr(token,']'))
								{
									printf("Brackets does not match!!\n");
									system("PAUSE");
								}
								isArray(fp2,token);
							}
							else if(strchr(token,']'))
							{
								printf("Brackets does not match!!\n");
								system("PAUSE");			
							}
							else
							{		//dizi deðilse, reserved word deðilse, deðiþken deðilse
								fputs(token,fp2);//buraya girer ve neyse o yazýlýr.
								if(*token=='\"')
								{
									putc(' ',fp2);  //Dizi olmayan keyword,datatype olmayan (mesela sayilar) tokenlardan sonra bosluk olacak mi?
								}
								if(strchr(token,'('))
								{// ( varsa function calldur,PROCEDURE olmadýðý için
									iffunccall=TRUE;//yeni fonksiyon baþlangýcý olamaz.
								}
								if(iffunc||iffunccall)
								{	//iki deðiþkenden biri set ise isFunc çaðrýlýr.
									isFunc(fp2,token,Proc);
								}
							}
							break;
						default:
							if(ptr->ispointer) //ptr, pointersa,
							{		//buraya girer ve tipi char deðilse
								if(ptr->data_type!="char")
								{		//* yazýlýr.
									putc('*',fp2);
								}
							}
							fputs(ptr->identifier,fp2); //deðiþken tipi yazýlýr.
							//putc(' ',fp2);//Datatypelardan sonra(int,char vs.) bosluk olacak mi?
							break;
					}
					break;
				case 0:	//FONKSIYON MU ? 0=PROCEDURE
					fputs(keywords[i].c_code,fp2); //fonksiyonsa, buraya girer,
					iffunc=TRUE;	//void yazar.
					iffunccall=FALSE; //iffunc set, iffunccall FALSE olur.
					isFunc(fp2,token,Proc);	//ilgili fonksiyon çaðrýlýr.
					break;
				case 1:	// 1= BEGIN.
					EOL=0;	//put_tabs_new_lines yapýsý gereði 0 olmalýdýr. ; konmamasý için
					put_tabs_new_lines(fp2,tab_counter,i,open_bracelet,EOL);
					break;
				case 2:// 2= END.
					EOL=0;//put_tabs_new_lines yapýsý gereði 0 olmalýdýr. ; konmamasý için
					put_tabs_new_lines(fp2,tab_counter,i,close_bracelet,EOL);
					break;
				default://diðer reserved wordler geldiyse de,
					fputs(keywords[i].c_code,fp2);//direkt karþýlýklarýný yaz
					//putc(' ',fp2);//RESERVED wordlerden sonra bosluk olacak mi?
					break;
			}
		}
		//Satir sonuna gelindiginde yeni satira gecme ve indentetion ayarlamalari.
		EOL=1;//End of line deðiþkeni set edilir.
		if(iffunc)
		{	//fonksiyon baþlangýcýysa,
			i=15; //i 15 yapýlýr. sonuna ; eklenmesin diye.
			iffunc=FALSE;
			iffunccall=FALSE;
		}
		if(!def)//DEFINE gelmediyse buraya gelecek. DEFINE geldiyse birþey yazma.
			put_tabs_new_lines(fp2,tab_counter,i,close_bracelet,EOL);//Adý kendini anlatýyor.
		def=FALSE;//default olarak false olmalý.
	}
	if(new_Proc)		//Yeni fonksiyon geldiyse
	{		//label3'e atlar.
		goto label3;
	}
	//Buraya geldiyse, dosya bitmiþtir,
	if(!_stricmp(Proc,"MAIN"))//son yazýlan fonksiyon mainse
	{ //system pause yazýlýr.
		fseek(fp2,-3,SEEK_CUR);
		putc('\t',fp2);
		fputs("system(\"PAUSE\");",fp2);
		putc('\n',fp2);
		putc('}',fp2);
		putc('\n',fp2);
	}
	if(open_bracelet!=close_bracelet)
	{ //ikisi eþit deðilse syntax hatasý vardýr.
		printf("\nSuslu parantezler eslesmiyor!\n");
		system("PAUSE");
	}
	fclose(fp1); //açýlan dosyalar kapatýlýr.
	fclose(fp2);
	return;
}
void isFunc(FILE *fp2,char *token,char *Proc)
{
	DATABASE *ptr;
	int control=FALSE;
	char isARR=FALSE, *token2;
	while(token=strtok(NULL," \n,]()"))//tokenize et.
	{
		isARR=FALSE;
		if(strstr(token,"["))	//dizi mi?
		{
			isARR=TRUE;
			token2=(char*)malloc(strlen(token)+1);
			strcpy(token2,token);
			token=strtok_2(token,"[");
		}
		ptr=head;
		while((ptr!=NULL)&&((_stricmp(Proc,ptr->proc))||(_stricmp(ptr->token,token))))
		{ //ptr null olana yada tokenlar eþleþene kadar ptr'ý next yap.
			ptr=ptr->db_next;
		} 
		if(ptr!=NULL)
		{ //ptr null deðilse deðiþkendir.
			if(!iffunccall) //function call deðilse,
			{		//yeni fonksiyon baþlangýcýdýr.Bu yüzden,
				fputs(ptr->data_type,fp2);//deðiþken tipini yaz,
				putc(' ',fp2); //ve boþluk býrak.
				if(!_stricmp(Proc,ptr->proc))
					ptr->isPara=TRUE;//Parametreyse, fonksiyon içinde
			}						//tekrar tanýmlanmasýn kontrolü.
			if(ptr->ispointer)//pointersa
			{ //* koy.
				if(ptr->data_type!="char")
					putc('*',fp2);
				else if(!iffunccall)
					putc('*',fp2);
			}
			fputs(ptr->identifier,fp2);//ve deðiþken adýný yaz.
			if(isARR)
			{
				putc('[',fp2);
				putc(']',fp2);
			}
			putc(',',fp2);  //sonraki elemanla arasýna virgül koy
			control=TRUE;//buraya girdiyse control set edilir.
		}
		else if(!control) //control false ise
		{	//dizi,deðiþken deðildir.(metin olabilir.)
			fputs(token,fp2);//direkt tokený yaz.
			putc(',',fp2); //sonraki elemanla arasýna virgül koy.
		}
		else if(ptr==NULL)//ptr, nullsa deðiþken deðildir.
		{ //1 karakter geri gel.
			fseek(fp2,-1,SEEK_CUR);
		}
		control=FALSE;//control default olarak false yapýlmalýdýr.
		if(isARR)
		strcpy(token,token2);
	}
	if(!control)//en son token, deðiþken deðilse buraya girer.
		fseek(fp2,-1,SEEK_CUR);//1 geri gelerek fazla karakteri siler.
	putc(')',fp2);//fonksiyon kapatýlýr.
}
void put_tabs_new_lines(FILE *fp2, int tab_counter, int i, int close_bracelet, int EOL)
{
	switch(i)//parametre olarak gelen i'ye göre,
	{		//ilgili case'e girer.
		case 1:// 1=BEGIN
			if(!EOL)
			{	//EOL 0 gelir eðer i=1 ise.
				isOpen_Bracelet(fp2,tab_counter,i,close_bracelet);
				break;//satýr sonu ; eklemez.
			}
		case 2: //2=END
			if(!EOL)
			{	//EOL 0 gelir eðer i=2 ise.
				isClose_Bracelet(fp2,tab_counter,i,close_bracelet);
				break;//satýr sonu ; eklemez.
			}
		case 15:// 15=DO   (WHILE olduðunu bildirir.
			isLoop(fp2,tab_counter);//satýr sonu ; eklemez.
			break;
		case 16:// 16=THEN (IF olduðunu belirtir.)
			isLoop(fp2,tab_counter);//satýr sonu ; eklemez.
			break;
		case 17:// 17=LOOP (FOR olduðunu bildirir.)
			isLoop(fp2,tab_counter);//satýr sonu ; eklemez.
			break;
		default:// Döngü deðilse BEGIN,END deðilse buraya girer.
			isNot_Loop(fp2,tab_counter);//satýr sonu ; ekler.
			break;
	}
}
void isLoop(FILE *fp2, int tab_counter)
{		//Döngüyse put_tabs_new_lines'dan buraya dallanýr,
	int k;//satýr sonuna ; eklemeden gerekli tableri ve,
	fputs("\n",fp2);//yeni satýr karakterini yazar.
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isNot_Loop(FILE *fp2, int tab_counter)
{		//Döngü deðilse put_tabs_new_lines'dan buraya dallanýr,
	int k;//satýr sonuna ; ekler, gerekli tableri ve,
	fputs(";\n",fp2);//yeni satýr karakterini yazar.
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isOpen_Bracelet(FILE *fp2, int tab_counter, int i, int open_bracelet)
{			//BEGIN'se put_tabs_new_lines'tan buraya dallanýr,
	int k;//satýr sonuna ; eklemeden gerekli,
	fputs("\n",fp2);//indentasyonlarý yapar.
	for(k=0;k<tab_counter;k++)//tableri ekler ve
	{					//yeni satýr karakterini yazar.
		fputs("\t",fp2);
	}
	fputs(keywords[i].c_code,fp2);
	fputs("\n",fp2);
	tab_counter++;
	open_bracelet++;
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isClose_Bracelet(FILE *fp2, int tab_counter, int i, int close_bracelet)
{			//END'se put_tabs_new_lines'tan buraya dallanýr,
	int k;//satýr sonuna ; eklemeden gerekli,
	tab_counter--;//indentasyonlarý yapar.
	close_bracelet++;
	fputs("\n",fp2);//tableri ekler ve
	for(k=0;k<tab_counter;k++)
	{					//yeni satýr karakterini yazar.
		fputs("\t",fp2);
	}
	fputs(keywords[i].c_code,fp2);
	fputs("\n",fp2);
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isArray(FILE *fp2,char *token)
{			//Dizi var ise
	token=strtok_2(token,"_");//dizi adýný alýr
	fputs(token,fp2);//dosyaya yazar
	putc('[',fp2);//köþeli parantezi yazar
	token=strtok_2(NULL,"[");//deðiþken tipini alýr ama yazmaz.
	token=strtok_2(NULL," ]_");//indis adýný alýr(varsa)
	fputs(token,fp2);//ve yazar.
	token=strtok_2(NULL,",");//indis tipini alýr ama yazmaz.
	while(token=strtok_2(NULL," \n,]"))
	{ //yukarýdaki iþlemleri, indis bitene kadar yapar,
		fputs(token,fp2);//çünkü dizi_c[i_i+j_i] durumu olabilir.
		if(token=strtok_2(NULL,"_]"))
		{
			fputs(token,fp2);
			token=strtok_2(NULL," \n,");
		}
		else
		{
		}
	}
	putc(']',fp2);
}
int for_loop_exception(FILE *fp2, char *token)
{
	//FOR icin ozel durum olustur.  for (i=0; i < n; i++) seklinde yazdirabilmek icin.
	DATABASE *ptr;
	char *hold;//kaybedilmemesi gereken bir token bunda tutulacaktýr.
	int i=12;//reserved words[12]=FOR
	fputs(keywords[i].c_code,fp2);//for ( yazýlýr.
	while(token=strtok(NULL," .\t\n"))
	{  //satýr sonuna kadar tokenize edilir.
		i=0;
		while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
		{ //FOR'un bittiðini gösteren LOOP gelirse i=17 olduðunda buradan çýkar.
			i++;
		}
		switch(i)
		{
			case KEYWORDS_SIZE:
				ptr=head;//FOR bitmemiþse buraya girer.
				while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
				{
					ptr=ptr->db_next;
				}
				switch((int)ptr)
				{
					case 0://deðiþken deðilse sayý olabilir.buraya girer.
						fputs(token,fp2);//token neyse o yazýlýr.
						fputs("; ",fp2);//for syntaxý gereði ; araya atýlýr
						fputs(hold,fp2);//holdda tutulan deðiþken adý yazýlýr.
						putc('<',fp2);//küçük olduðu süre boyunca dönecek.
						token=strtok(NULL," .\t\n");//maksimum terim alýnýr.
						ptr=head;
						while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
						{
							ptr=ptr->db_next;
						}
						fputs(ptr->identifier,fp2);//maks.terim hangi deðiþkense,
						fputs("; ",fp2);//yazýlýr ve ; koyulur.
						fputs(hold,fp2);//holddaki indis yazýlýr.
						fputs("++",fp2);//for syntax'ý gereði bu konulur.
						break;
					default:
						fputs(ptr->identifier,fp2);//ilk baþta buraya girecek ve
						hold=ptr->identifier;//indisi ekrana basýp, hold'a koyacak.
						break;
				}
				break;
			case 5://Bosu bosuna ekstradan bir bosluk karakteri,
				putc('=',fp2);//yazilmasin diye direkt olarak = basiyorum. 
				break; //(database.h'da keywords[5].c_code="= ") 
			default://LOOP gelirse ) basýlýr.
				fputs(keywords[i].c_code,fp2);
				break;
		}
	}
	return i;//i deðeri kullanýlacaðý için geri döndürür.
}
void GetVal_PrintLine(FILE *fp2, char *token, int i)
{
	DATABASE *ptr;
	char *tmpcode;//token'ý kaybetmemem lazým. tmpcode'a kopyalanacak.
	fputs(keywords[i].c_code,fp2);//printf(" veya scanf(" yazýlýr.
	tmpcode=(char*)malloc(LINESIZE);
	strcpy(tmpcode,token);//tmpcode'a token'ý kopyala.
	while(token=strtok(NULL,"“” \"]\n\t"))//satýr sonuna kadar,
	{				//ayarlanmýþ olan delimiterlerle tokenize et.
		strcat(tmpcode,token);//tmpcode'a token'ý ekle
		if(strchr(token,'$'))//$ karakteri geldiyse,
		{		//ilgili uzantýlarla uyuþma durumuna göre,
			if(strstr(token,"_c"))//ilgili format kodunu yaz.
			{	
				if(strstr(token,"_cp"))
					fputs("%s",fp2);
				else
					fputs("%s",fp2);
			}
			else if(strstr(token,"_d"))
			{	fputs("%f",fp2);	}
			else if(strstr(token,"_f"))
			{	fputs("%f",fp2);	}
			else if(strstr(token,"_ld"))
			{	fputs("%lf",fp2);	}
			else if(strstr(token,"_l"))
			{	fputs("%li",fp2);	}
			else if(strstr(token,"_i"))
			{	fputs("%d",fp2);	}
			if(1)	//her durumda girmesi gerekiyor.
			{
				if(strchr(token,','))//virgül varsa
				{ //virgül yaz.
					putc(',',fp2);
				}
				if(strchr(token,'.'))//nokta varsa,
				{ //nokta yaz.
					putc('.',fp2);
				}
				putc(' ',fp2);//okunabilirlik için boþluk býrak
			}
		}
		else//$ karakteri yoksa düz kelimedir.
		{ //direkt yaz
			fputs(token,fp2);
			putc(' ',fp2);//boþluk býrak.
		}
	}
	fseek(fp2,-1,SEEK_CUR);//sona gelince ekstra boþluðu sil
	if(i==20)
	{ //printf ise \n  " ve ,  yazýlmalý.
		fputs("\\n\", ",fp2);
	}
	else
	{	//deðilse sadece " ve , yazýlmalý.	
		fputs("\", ",fp2);
	}
	if(strchr(tmpcode,'$'))//$ karakteri varsa
	{ //deðiþken kullanýlmýþtýr.yoksa düz metin yazýlmýþtýr.
		token=strtok(tmpcode," $\n\t");//$ dan öncesini al ama yazma, düz metin.
		while(token=strtok(NULL," $\n\t"))
		{ //satýr sonuna kadar tokenize et.
			if(strchr(token,'['))
			{ //[ varsa dizidir.
				if(i==21)
				{ //scanf ise & eklenmeli
					putc('&',fp2);
				}
				isArray(fp2,token);//isArray'i çaðýr.
				fputs(", ",fp2);//sonraki elemanla ayýr.
			}
			for(ptr=head;ptr!=NULL;ptr=ptr->db_next)
			{
				if(strstr(token,ptr->token))
				{
					if(i==21)
					{ //scanf ise 
						if(!ptr->ispointer)//pointer deðilse
							putc('&',fp2);//& eklenmeli.
					}
					fputs(ptr->identifier,fp2);//deðiþken adý yazýlýr.
					fputs(", ",fp2);//sonraki elemanla ayrýlýr.
					break;
				}
			}
		}
	}
	fseek(fp2,-2,SEEK_CUR);//fazladan yazýlan , ve boþluðu siler.
	putc(')',fp2);
	return;
}
void print_Vars(FILE *fp2, char *Proc,int k)
{
	DATABASE *ptr;
	int i;
	char text[4];//dizi boyutu için.(maksimum 999 olabilir)
	for(i=0;i<USES_VARS_SIZE;i++)
	{			//tüm deðiþken tipleri için dönecek.
		if(uses_vars[k][i]==TRUE)
		{		//ilgili deðiþken tipi varsa
			fputs(vars[i],fp2); //deðiþken tipini basar,
			putc(' ',fp2);		//boþluk býrakýr.
			for(ptr=head;(ptr!=NULL)&&(ptr->data_type!=vars[i]);ptr=ptr->db_next);
			for(;(ptr!=NULL)&&(ptr->data_type==vars[i]);ptr=ptr->db_next)
			{	//ilgili deðiþken tipinin olduðu kýsma gelene kadar null statement döner.
				//deðiþken tipi deðiþmediði sürece döner.
				if((!_stricmp(Proc,ptr->proc))&&(!ptr->isPara))//Parametreyse tanýmlama.
				{  //þuanki fonksiyon, ptr'ýn bulunduðu fonksiyonsa,
					if(ptr->ispointer)//buraya girer
					{		//ptr pointersa * basar
						putc('*',fp2);
					}//deðiþken adýný yazar.
					else if(i==2)
						putc('*',fp2);
					fputs(ptr->identifier,fp2);
					if(ptr->isarray)
					{ //deðiþken bir diziyse
						putc('[',fp2);//[ basar.
						fputs(itoa(ptr->isarray,text,10),fp2);//int to string yapýlýp
						putc(']',fp2);//dizi boyutu basýlýr ve ] basýlýr.
					}
					putc(',',fp2);//sonraki elemandan ayrýlýr.
					putc(' ',fp2);//okunabilirlik için boþluk koyulur.
				}
			}		//fazladan yazýlan , ve boþluk silinir.
			fseek(fp2,-2,SEEK_CUR);
			fputs(";\n\t",fp2);//satýr sonuna gelindi, alta geçilir.
		}
	}
}
DATABASE * arrange_bools(char *ident,char *dtype, char *tmptoken,char *ifarray)
{
	DATABASE *ptr;//ptr'a parametrelerle yer açýlýr.
	ptr=create_list_element(strlen(ident),strlen(dtype)+1,strlen(tmptoken));
	if(*ifarray)
	{ //diziyse ilgili kýsým set,
		ptr->isarray=TRUE;
	}
	else
	{ //dizi deðilse ilgili kýsým false yapýlýr.
		ptr->isarray=FALSE;
	}
	ptr->data_type=dtype;//deðiþken tipi ilgili kýsma yazýlýr.
	return ptr;//ptr deðeri döner
}
void return_type(FILE *fp2,char *token,long int pos)
{
	DATABASE *ptr;//RETURN komutu varsa bu fonksiyon çaðrýlýr.
	int space_len,i;//space_len, dönüþ tipine göre ayarlanacak.
	token=strtok(NULL," _\n\t");//deðiþken adý alýnýr.
	if(!strcmp(token,"VOID"))//VOID'se
	{					//void yazýlýr.
		fputs("void",fp2);
		return;//ve çýkýlýr.
	} //void deðilse alt satýra geçer.
	fputs(token,fp2);//deðiþken adý yazýlýr.
	fseek(fp2,pos,SEEK_SET);//ilgili fonksiyonun baþlangýçý pos'ta var.oraya gidilir.
	for(ptr=head;(ptr->db_next!=NULL)&&(strcmp(token,ptr->identifier));ptr=ptr->db_next);
	fputs(ptr->data_type,fp2);//Ýlgili deðiþkeni bulana kadar null for döner.
	space_len=11-strlen(ptr->data_type);//space len deðiþken tipine göre ayarlanýr.
	for(i=0;i<space_len;i++)//space_len kadar boþluk koy.
		putc(' ',fp2);
	token=strtok(NULL," \t\n");//satýr sonuna gelmek için yazýlýr.
	if(ptr->ispointer) 
	{			//deðiþken pointersa
		putc('*',fp2);//* koyulur.
	}
	else if(!strcmp("int",ptr->data_type))
	{//pointer deðilse, int ise ekstra bir boþluk koyulur,
		putc(' ',fp2);//çünkü int void'den 1 karakter kýsa.
	}
	putc(' ',fp2);
	if(!strcmp(ptr->proc,"MAIN"))//main için özel durum var.
		fputs("main()",fp2);//
	else//main deðilse, deðiþkenin ait olduðu,
		fputs(ptr->proc,fp2);//fonksiyon yazýlýr.
	fseek(fp2,0,SEEK_END);//dosya sonuna geri dön.
}