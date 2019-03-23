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
	scanf("%s",filec_name); //Pseudo kod dosyas� ve C kodu yaz�lacak dosya ismi al�nd�.
	fp1=open_file(filep_name,"r"); //Pseudo kod dosyas� a��ld�.
	file_size=find_file_size(fp1);	// Dosya boyutu bulundu.
	rewind(fp1);	//Dosya ba��na d�n�ld�.
	printf("Pseudocode dosyasi okunuyor..\n");
	fgets(p_code,LINESIZE-1,fp1);	//Pseudo kod dosyas�n�n ilk sat�r� p_code stringine al�nd�.
	data_array=(char*)malloc(file_size);	//data_array stringine yer a��ld�
	strcpy(data_array,p_code);
	while(fgets(p_code,LINESIZE-1,fp1))
	{
		strcat(data_array,p_code);		//B�t�n pseudo kod dosyas� data_array stringine al�nd�.
	}
	fclose(fp1);
	printf("Pseudocode dosyasi basariyla okundu.\n");
	isDeclared(data_array);		//data_array haz�r, sembol tablosu olu�turmak i�in isDeclared �a�r�l�yor.
	printf("Degiskenlerle ilgili bilgileri tutan symboltable, linkli liste olarak olusturuldu.\n");
	arrange_tokens();	//Sembol tablosu olu�turuldu. 1.3.10. k�s�mdaki olaylar i�in �a�r�l�r.
	printf("\n\n");
	printf("\tPTR\t\tPROC\tTOKEN\tIDENT.\tDTYPE\tISARR?\tISPTR?\tNEXT\n");
	for(ptr=head;ptr!=NULL;ptr=ptr->db_next)
		printf("\t%d\t%s\t%s\t%s\t%s\t%d\t%d\t%d\n\n",ptr,ptr->proc,ptr->token,ptr->identifier,ptr->data_type,ptr->isarray,ptr->ispointer,ptr->db_next);
	system("PAUSE");
	print_file(filep_name,filec_name);	//Sembol tablosu haz�r, C kodu dosyaya yaz�lmaya ba�lan�r
	printf("C kodu dosyaya yazildi.\n");
	printf("Olusturulacak exe adini giriniz: ");
	scanf("%s",output_file);
	sysout=(char*)malloc(strlen(filec_name)+strlen(output_file)+10);
	strcpy(sysout,"gcc ");
	strcat(sysout,filec_name);
	strcat(sysout," -o ");
	strcat(sysout,output_file);
	system(sysout);	//Olu�turulan c kodu dosyas� gcc ile derlenir.
	printf("Executable dosya olusturuldu.");
	system("PAUSE");
	system(output_file);	//Derleme sonu olu�turulan dosya �al��t�r�l�r.
	return 1;
}
FILE *open_file(char *file_name,char* file_mode)
{
	//Dosya A�ma fonksiyonu
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
	if(!strcmp(token,"PROCEDURE")) //�lk sat�rda fonksiyon ba�lang�c� m� var?
	{
		new_Proc=TRUE;
	}
	if(strchr(token,'[')) //Yoksa bir dizi mi?
	{
		ifarray=TRUE;
	}
	tmptoken=(char*)malloc(strlen(token)+1); //token'� kaybetmemek i�in olu�turulan
	strcpy(tmptoken,token);			//tmptoken'a token'� al�yoruz
	if(!_stricmp(token,"DEFINE"))	//DEFINE geldiyse
	{								//tokenize et
		ident=strtok_2(token,datalimiter);	
		dtype=strtok_2(NULL,datalimiter);
		array_size=atoi(strtok_2(NULL,"]"));	//stringi int'e d�n��t�r.
	}
	else
	{
		ident=strtok_2(token,datalimiter);	//DEFINE gelmediyse sadece tokenize et
		dtype=strtok_2(NULL,datalimiter);
	}
	goto label2;		//ilk �a�r�mda label2 ye git.
label1:
	while(token=strtok(NULL,tokenlimiter)) //sat�r sonuna gelene kadar tokenize etmeye devam.
	{
		array_size=100;		//dizi boyutu default olarak 100
		if(new_Proc)		//Sat�rda fonksiyon ba�l�yorsa new_Proc set edilmi�ti.
		{
			strcpy(Proc,token);	//Proc stringine fonksiyon ad�n� al�yoruz
			new_Proc=FALSE;	//new_Proc default olarak FALSE
		}
		if(!strcmp(token,"PROCEDURE"))	//yeni fonksiyon geldiyse new_Proc set.
		{
			new_Proc=TRUE;
			k++;		//Fonksiyon s�ras�n� tutar.
		}
		if(strchr(token,'['))	//[ varsa dizidir.
		{
			ifarray=TRUE;
		}
		strcpy(tmptoken,token);	//token'� sonradan kullanaca��m, de�eri de�i�ece�i i�in tmptoken'a al�yorum.
		if(!_stricmp(token,"DEFINE"))//DEFINE m� geldi?
		{	
			token=strtok(NULL," \n");	
			ifarray=TRUE;	
			ident=strtok_2(token,datalimiter);
			dtype=strtok_2(NULL,datalimiter);
			array_size=atoi(strtok_2(NULL,"]"));	//dizi boyutunu ayarl�yoruz
		}
		else
		{
			ident=strtok_2(token,datalimiter);	//dizi de�ilse normal devam.
			dtype=strtok_2(NULL,datalimiter);
		}
		for(tmptr=head;tmptr!=NULL;tmptr=tmptr->db_next)//De�i�ken daha �nce tan�mland� m�?
		{
			if(!strcmp(ident,tmptr->identifier))
			{
				if(!strcmp(Proc,tmptr->proc))
				{
					ifarray=FALSE;
					goto label1;	//Tan�mland�ysa label1 e gidip yeni token'� al.
				}
			}
		}
label2:	if(!dtype)	//�lk �evrimde direkt buraya atl�yordu.
		{			//Token, de�i�ken de�ilse dtype null olacak.
			goto label1;	//label1e gidip yeni token al
		}
		switch((int)*dtype)	//switch'in i�indeki case, de�i�ken tiplerinin
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
				if(strlen(dtype)==4)		//FILE * m�?
				{
					ptr=arrange_bools(ident,"FILE",tmptoken,&ifarray);
					uses_vars[k][2]=TRUE;
				}
				else						//FLOAT m�?
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
				goto label1; //De�i�ken de�ilse buraya d��er. label1 e gidip yeni token al.
		}		
		if(strchr(dtype,'p'))//De�i�ken tipi, p i�eriyorsa pointerd�r.
		{
			ptr->ispointer=TRUE;
		}
		else
		{
			ptr->ispointer=FALSE;
		}
		ptr->identifier=ident;	//de�i�ken ad�n� ayarla.
		if(ifarray)		//diziyse, boyutunu ayarla.
		{
			ptr->isarray=array_size;
			ifarray=FALSE;
		}
		strcpy(ptr->proc,Proc);	//hangi fonksiyonda ge�ti�ini ayarla
		insert(ptr);
	}
	return;
}
DATABASE *create_list_element(unsigned int strsize, unsigned int dtypesize, unsigned int tokensize)
{
	DATABASE *ptr;
	ptr=(DATABASE*) malloc(sizeof(DATABASE));	//structa yer a�
	ptr->identifier=(char*)malloc(strsize+1);	//De�i�ken ad�na 
	ptr->data_type=(char*)malloc(dtypesize+1);	//De�i�ken tipine
	ptr->token=(char*)malloc(tokensize+1);	//De�i�ken token'�na yer a�.
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
	if(head==NULL)	//�lk elemansa
	{
		head=e;	//ba� eleman olarak ayarla.
		return;
	}
	for(i=0;i<USES_VARS_SIZE;i++)	//T�m de�i�ken tipleri i�in d�n.
	{
		if(_stricmp((e->data_type),vars[i])==0)	//Arad���m de�i�ken tiplerinin
		{							//tutuldu�u alana kadar buraya girmeyecek.
			if(i==4)		//int'se en ba�ta tutulacak.
			{
				e->db_next=head;
				head=e;
				return;
			}	//int de�ilse, ilgili de�i�ken tipinin en son eleman� olarak eklenir.
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
	for(ptr=head;ptr!=NULL;ptr=ptr->db_next)	//En ba�tan ba�la, sona kadar d�n
	{
		strcpy(ptr->token,ptr->identifier);	//token'a de�i�ken ad�n� al
		switch((int)*ptr->data_type) //de�i�ken tipine g�re ilgili
		{						//uzant�y� ekle.
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
		if(ptr->ispointer)	//pointersa p uzant�s�n� da ekle
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
	fp1=open_file(filep_name,"r");	//pseudo kod dosyas� a�.
	fp2=open_file(filec_name,"w+");	//c kodu yaz�lacak dosyay� a�.
	fputs("#include <stdio.h>\n",fp2);
	fgets(p_code,LINESIZE-1,fp1);	//ilk sat�r� al.
	if(_stricmp("PROCEDURE MAIN()\n",p_code))	//main var m�?
	{		//Yoksa label4 e git.
		goto label4;
	}
	else
	{
		poss[t+1].pos=ftell(fp2);	//fonksiyon ba�lang�c�n�n pozisyonu
		strcpy(poss[t+1].funcname,"main()");//fonksiyon ad�
		fputs("void main()       \n",fp2);//main ba�lang�c�n� yaz.
		strcpy(Proc,"MAIN");//�uanki fonksiyon main.
	}
	
label3:	//yeni fonksiyon ba�lang�c� gelirse buraya atlar.
	t++;	//fonksiyon s�ras�n� tutan de�i�ken artar.
	fgets(p_code,LINESIZE-1,fp1);	//yeni sat�r� al
	if(!_stricmp("BEGIN\n",p_code))	//BEGINse { koy
	{
		fputs("{\n\t",fp2);
		tab_counter++;	//indentasyon ayarlamalar�.
		open_bracelet++;
	}
	else
	{
		printf("Mainden sonra suslu parantez eksik!!");
		system("PAUSE");
		exit(1);
	}
	print_Vars(fp2,Proc,t);	//t. s�radaki fonksiyona ait de�i�kenleri yaz.
	new_Proc=FALSE;

	while((!new_Proc)&&(fgets(p_code,LINESIZE-1,fp1)))//yeni fonksiyon gelene ya da
	{						//dosya sonuna gelene kadar yaz.
		iffunc=FALSE;	//default olarak bunlar false olmal�.
		iffunccall=FALSE;
label4:	//ilk sat�rda mainden farkl� fonksiyon varsa buraya atlar.
		token=strtok(p_code," \t\n");
		if(!strcmp(token,"PROCEDURE"))	//yeni fonksiyon ba�lang�c� m� geldi?
		{
			new_Proc=TRUE;	
			if(!strcmp(Proc,"MAIN"))	//�nceki fonksiyon mainse, system pause yaz.
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
		i=0;	//while i�in haz�rl�k.
		while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
		{	//reserved word struct�ndaki eleman say�s�na gelene kadar
			i++;//veya tokenlar e�le�ene kadar d�necek ve i++ olacak.
		}
		switch(i)	//en son i ne kalm��sa ona g�re ilgili
		{			//case'e girecek.
			case KEYWORDS_SIZE:	//e�er i, boyuta e�itse, reserved word de�ildir.
				ptr=head;	//de�i�ken midir diye bak�l�r.
				while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
				{	//struct sonuna gelene ya da tokenlar e�le�ene kadar ptr'� next yap.
					ptr=ptr->db_next;
				}
				switch((int)ptr)	//pointer de�erine g�re;
				{
					case 0:	//e�er NULL'sa dizi olabilir.
						if(strchr(token,'['))
						{
							if(!strchr(token,']'))
							{
								printf("Brackets does not match!!\n");
								system("PAUSE");
							}
							isArray(fp2,token);	//diziyse isArray fonksiyonu �a�r�l�r.
						}
						else if(strchr(token,']'))
						{
							printf("Brackets does not match!!\n");
							system("PAUSE");	
						}
						else	//dizi de�ilse, reserved word de�ilse, de�i�ken de�ilse
						{	//function call olur ��nk� sat�r ba��ndaki ilk token, say� olamaz.
							fputs(token,fp2);	//buraya girer ve token neyse o yaz�l�r dosyaya.
							if(*token=='\"')
							{
								putc(' ',fp2); //Dizi,keyword,datatype olmayan tokenlardan sonra bosluk olacak mi?
							}
							iffunccall=TRUE;	//iffunccall set,
							iffunc=FALSE;		//iffunc false edilir ve,
							isFunc(fp2,token,Proc);	//isFunc �a�r�l�r.
						}
						break;
					default:
						if(ptr->ispointer)	//ptr, pointersa
						{			//ve char de�ilse
							if(ptr->data_type!="char")
							{		//* yaz�l�r.
								putc('*',fp2);
							}
						}
						fputs(ptr->identifier,fp2);	//de�i�ken ad� yaz�l�r.
						fputs(" ",fp2);			//okunabilirlik i�in bo�luk koyulur.
						break;
				}
				break;
			case 0:	//FONKSIYON MU ?
				fputs(keywords[i].c_code,fp2);
				iffunc=TRUE;
				iffunccall=FALSE;
				token=strtok(NULL,"("); //fonksiyon ad� al�n�r
				strcpy(Proc,token);	//Proc'a atan�r
				strcpy(poss[t+1].funcname,Proc);//pozisyonu al�n�r
				if(!_stricmp(token,"MAIN"))	//main'se dosyaya yaz�l�r.
				{
					fputs("main",fp2);
				}
				else
					fputs(token,fp2);	//de�ilse direkt bas�l�r.
				putc('(',fp2);
				putc(' ',fp2);	//isFunc yap�s� gere�i ('dan sonra bir bo�luk olmal�.
				isFunc(fp2,token,Proc);	//isFunc �a�r�l�r
				break;
			case 1:
				fputs(keywords[i].c_code,fp2);	//1=BEGIN
				tab_counter++;		//tab say�s� artmal�
				open_bracelet++;//s�sl� parantez a�ma say�s� artar
				for(k=0;k<tab_counter;k++)
				{
					fputs("\t",fp2);	//gerekli tablar koyulur
				}
				break;
			case 2:	//2=END, tablar koyulmu�tu. 1 tab geriye yaz�lmal�.
				fseek(fp2,-1,SEEK_CUR);	//1 karakter geri git.
				fputs(keywords[i].c_code,fp2);//END yaz.
				tab_counter--;	//tablar�n say�s� azal�r.
				close_bracelet++;//s�sl� parantez kapatma say�s� artar.
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
				//PRINT_LINE i�in �zel durum olu�tur.
				GetVal_PrintLine(fp2, token, i);
				break;
			case 21:  //21=GET_VAL
				//GET_VAL i�in �zel durum olu�tur.
				GetVal_PrintLine(fp2, token, i);
				break;
			case 3://3=RETURN
				fputs(keywords[i].c_code,fp2); //return yaz ve,
				return_type(fp2,token,poss[t].pos);//ilgili fonksiyonu �a��r.
				break;
			case 22:  //22=DEFINE geldiyse, ekrana yaz�lmayacak,
				while(token=strtok(NULL," \n"));//sadece deklarasyon k�sm�nda,
				def=TRUE;	//bir farkl�l�k olacak ve bu da def de�i�keni,
				break;		//arac�l���yla olacak.
			default:	//di�er reserved wordler geldiyse de,
				fputs(keywords[i].c_code,fp2);//direkt kar��l���n� yaz.
				//putc(' ',fp2);//Satir basinda ilk alinan reserved wordden sonra bosluk olacak mi?
				break;
		}
		while(token=strtok(NULL," \t\n"))//Al�nan sat�r�n�n sonuna gelene kadar d�ner.
		{				
			i=0;//while'a haz�rl�k
			while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
			{//reserved word struct�ndaki eleman say�s�na gelene kadar
				i++;//veya tokenlar e�le�ene kadar d�necek ve i++ olacak.
			}
			switch(i)	//en son i ne kalm��sa ona g�re ilgili
			{			//case'e girecek.
				case KEYWORDS_SIZE:	//e�er i, boyuta e�itse, reserved word de�ildir.
					ptr=head;	//de�i�ken midir diye bak�l�r.
					while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
					{	//struct sonuna gelene ya da tokenlar e�le�ene kadar ptr'� next yap.
						ptr=ptr->db_next;
					}
					switch((int)ptr)
					{		//pointer de�erine g�re;
						case 0:	//E�er null'sa
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
							{		//dizi de�ilse, reserved word de�ilse, de�i�ken de�ilse
								fputs(token,fp2);//buraya girer ve neyse o yaz�l�r.
								if(*token=='\"')
								{
									putc(' ',fp2);  //Dizi olmayan keyword,datatype olmayan (mesela sayilar) tokenlardan sonra bosluk olacak mi?
								}
								if(strchr(token,'('))
								{// ( varsa function calldur,PROCEDURE olmad��� i�in
									iffunccall=TRUE;//yeni fonksiyon ba�lang�c� olamaz.
								}
								if(iffunc||iffunccall)
								{	//iki de�i�kenden biri set ise isFunc �a�r�l�r.
									isFunc(fp2,token,Proc);
								}
							}
							break;
						default:
							if(ptr->ispointer) //ptr, pointersa,
							{		//buraya girer ve tipi char de�ilse
								if(ptr->data_type!="char")
								{		//* yaz�l�r.
									putc('*',fp2);
								}
							}
							fputs(ptr->identifier,fp2); //de�i�ken tipi yaz�l�r.
							//putc(' ',fp2);//Datatypelardan sonra(int,char vs.) bosluk olacak mi?
							break;
					}
					break;
				case 0:	//FONKSIYON MU ? 0=PROCEDURE
					fputs(keywords[i].c_code,fp2); //fonksiyonsa, buraya girer,
					iffunc=TRUE;	//void yazar.
					iffunccall=FALSE; //iffunc set, iffunccall FALSE olur.
					isFunc(fp2,token,Proc);	//ilgili fonksiyon �a�r�l�r.
					break;
				case 1:	// 1= BEGIN.
					EOL=0;	//put_tabs_new_lines yap�s� gere�i 0 olmal�d�r. ; konmamas� i�in
					put_tabs_new_lines(fp2,tab_counter,i,open_bracelet,EOL);
					break;
				case 2:// 2= END.
					EOL=0;//put_tabs_new_lines yap�s� gere�i 0 olmal�d�r. ; konmamas� i�in
					put_tabs_new_lines(fp2,tab_counter,i,close_bracelet,EOL);
					break;
				default://di�er reserved wordler geldiyse de,
					fputs(keywords[i].c_code,fp2);//direkt kar��l�klar�n� yaz
					//putc(' ',fp2);//RESERVED wordlerden sonra bosluk olacak mi?
					break;
			}
		}
		//Satir sonuna gelindiginde yeni satira gecme ve indentetion ayarlamalari.
		EOL=1;//End of line de�i�keni set edilir.
		if(iffunc)
		{	//fonksiyon ba�lang�c�ysa,
			i=15; //i 15 yap�l�r. sonuna ; eklenmesin diye.
			iffunc=FALSE;
			iffunccall=FALSE;
		}
		if(!def)//DEFINE gelmediyse buraya gelecek. DEFINE geldiyse bir�ey yazma.
			put_tabs_new_lines(fp2,tab_counter,i,close_bracelet,EOL);//Ad� kendini anlat�yor.
		def=FALSE;//default olarak false olmal�.
	}
	if(new_Proc)		//Yeni fonksiyon geldiyse
	{		//label3'e atlar.
		goto label3;
	}
	//Buraya geldiyse, dosya bitmi�tir,
	if(!_stricmp(Proc,"MAIN"))//son yaz�lan fonksiyon mainse
	{ //system pause yaz�l�r.
		fseek(fp2,-3,SEEK_CUR);
		putc('\t',fp2);
		fputs("system(\"PAUSE\");",fp2);
		putc('\n',fp2);
		putc('}',fp2);
		putc('\n',fp2);
	}
	if(open_bracelet!=close_bracelet)
	{ //ikisi e�it de�ilse syntax hatas� vard�r.
		printf("\nSuslu parantezler eslesmiyor!\n");
		system("PAUSE");
	}
	fclose(fp1); //a��lan dosyalar kapat�l�r.
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
		{ //ptr null olana yada tokenlar e�le�ene kadar ptr'� next yap.
			ptr=ptr->db_next;
		} 
		if(ptr!=NULL)
		{ //ptr null de�ilse de�i�kendir.
			if(!iffunccall) //function call de�ilse,
			{		//yeni fonksiyon ba�lang�c�d�r.Bu y�zden,
				fputs(ptr->data_type,fp2);//de�i�ken tipini yaz,
				putc(' ',fp2); //ve bo�luk b�rak.
				if(!_stricmp(Proc,ptr->proc))
					ptr->isPara=TRUE;//Parametreyse, fonksiyon i�inde
			}						//tekrar tan�mlanmas�n kontrol�.
			if(ptr->ispointer)//pointersa
			{ //* koy.
				if(ptr->data_type!="char")
					putc('*',fp2);
				else if(!iffunccall)
					putc('*',fp2);
			}
			fputs(ptr->identifier,fp2);//ve de�i�ken ad�n� yaz.
			if(isARR)
			{
				putc('[',fp2);
				putc(']',fp2);
			}
			putc(',',fp2);  //sonraki elemanla aras�na virg�l koy
			control=TRUE;//buraya girdiyse control set edilir.
		}
		else if(!control) //control false ise
		{	//dizi,de�i�ken de�ildir.(metin olabilir.)
			fputs(token,fp2);//direkt token� yaz.
			putc(',',fp2); //sonraki elemanla aras�na virg�l koy.
		}
		else if(ptr==NULL)//ptr, nullsa de�i�ken de�ildir.
		{ //1 karakter geri gel.
			fseek(fp2,-1,SEEK_CUR);
		}
		control=FALSE;//control default olarak false yap�lmal�d�r.
		if(isARR)
		strcpy(token,token2);
	}
	if(!control)//en son token, de�i�ken de�ilse buraya girer.
		fseek(fp2,-1,SEEK_CUR);//1 geri gelerek fazla karakteri siler.
	putc(')',fp2);//fonksiyon kapat�l�r.
}
void put_tabs_new_lines(FILE *fp2, int tab_counter, int i, int close_bracelet, int EOL)
{
	switch(i)//parametre olarak gelen i'ye g�re,
	{		//ilgili case'e girer.
		case 1:// 1=BEGIN
			if(!EOL)
			{	//EOL 0 gelir e�er i=1 ise.
				isOpen_Bracelet(fp2,tab_counter,i,close_bracelet);
				break;//sat�r sonu ; eklemez.
			}
		case 2: //2=END
			if(!EOL)
			{	//EOL 0 gelir e�er i=2 ise.
				isClose_Bracelet(fp2,tab_counter,i,close_bracelet);
				break;//sat�r sonu ; eklemez.
			}
		case 15:// 15=DO   (WHILE oldu�unu bildirir.
			isLoop(fp2,tab_counter);//sat�r sonu ; eklemez.
			break;
		case 16:// 16=THEN (IF oldu�unu belirtir.)
			isLoop(fp2,tab_counter);//sat�r sonu ; eklemez.
			break;
		case 17:// 17=LOOP (FOR oldu�unu bildirir.)
			isLoop(fp2,tab_counter);//sat�r sonu ; eklemez.
			break;
		default:// D�ng� de�ilse BEGIN,END de�ilse buraya girer.
			isNot_Loop(fp2,tab_counter);//sat�r sonu ; ekler.
			break;
	}
}
void isLoop(FILE *fp2, int tab_counter)
{		//D�ng�yse put_tabs_new_lines'dan buraya dallan�r,
	int k;//sat�r sonuna ; eklemeden gerekli tableri ve,
	fputs("\n",fp2);//yeni sat�r karakterini yazar.
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isNot_Loop(FILE *fp2, int tab_counter)
{		//D�ng� de�ilse put_tabs_new_lines'dan buraya dallan�r,
	int k;//sat�r sonuna ; ekler, gerekli tableri ve,
	fputs(";\n",fp2);//yeni sat�r karakterini yazar.
	for(k=0;k<tab_counter;k++)
	{
		fputs("\t",fp2);
	}
}
void isOpen_Bracelet(FILE *fp2, int tab_counter, int i, int open_bracelet)
{			//BEGIN'se put_tabs_new_lines'tan buraya dallan�r,
	int k;//sat�r sonuna ; eklemeden gerekli,
	fputs("\n",fp2);//indentasyonlar� yapar.
	for(k=0;k<tab_counter;k++)//tableri ekler ve
	{					//yeni sat�r karakterini yazar.
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
{			//END'se put_tabs_new_lines'tan buraya dallan�r,
	int k;//sat�r sonuna ; eklemeden gerekli,
	tab_counter--;//indentasyonlar� yapar.
	close_bracelet++;
	fputs("\n",fp2);//tableri ekler ve
	for(k=0;k<tab_counter;k++)
	{					//yeni sat�r karakterini yazar.
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
	token=strtok_2(token,"_");//dizi ad�n� al�r
	fputs(token,fp2);//dosyaya yazar
	putc('[',fp2);//k��eli parantezi yazar
	token=strtok_2(NULL,"[");//de�i�ken tipini al�r ama yazmaz.
	token=strtok_2(NULL," ]_");//indis ad�n� al�r(varsa)
	fputs(token,fp2);//ve yazar.
	token=strtok_2(NULL,",");//indis tipini al�r ama yazmaz.
	while(token=strtok_2(NULL," \n,]"))
	{ //yukar�daki i�lemleri, indis bitene kadar yapar,
		fputs(token,fp2);//��nk� dizi_c[i_i+j_i] durumu olabilir.
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
	char *hold;//kaybedilmemesi gereken bir token bunda tutulacakt�r.
	int i=12;//reserved words[12]=FOR
	fputs(keywords[i].c_code,fp2);//for ( yaz�l�r.
	while(token=strtok(NULL," .\t\n"))
	{  //sat�r sonuna kadar tokenize edilir.
		i=0;
		while((i<KEYWORDS_SIZE)&&(_stricmp(keywords[i].token,token)))
		{ //FOR'un bitti�ini g�steren LOOP gelirse i=17 oldu�unda buradan ��kar.
			i++;
		}
		switch(i)
		{
			case KEYWORDS_SIZE:
				ptr=head;//FOR bitmemi�se buraya girer.
				while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
				{
					ptr=ptr->db_next;
				}
				switch((int)ptr)
				{
					case 0://de�i�ken de�ilse say� olabilir.buraya girer.
						fputs(token,fp2);//token neyse o yaz�l�r.
						fputs("; ",fp2);//for syntax� gere�i ; araya at�l�r
						fputs(hold,fp2);//holdda tutulan de�i�ken ad� yaz�l�r.
						putc('<',fp2);//k���k oldu�u s�re boyunca d�necek.
						token=strtok(NULL," .\t\n");//maksimum terim al�n�r.
						ptr=head;
						while((ptr!=NULL)&&(_stricmp(ptr->token,token)))
						{
							ptr=ptr->db_next;
						}
						fputs(ptr->identifier,fp2);//maks.terim hangi de�i�kense,
						fputs("; ",fp2);//yaz�l�r ve ; koyulur.
						fputs(hold,fp2);//holddaki indis yaz�l�r.
						fputs("++",fp2);//for syntax'� gere�i bu konulur.
						break;
					default:
						fputs(ptr->identifier,fp2);//ilk ba�ta buraya girecek ve
						hold=ptr->identifier;//indisi ekrana bas�p, hold'a koyacak.
						break;
				}
				break;
			case 5://Bosu bosuna ekstradan bir bosluk karakteri,
				putc('=',fp2);//yazilmasin diye direkt olarak = basiyorum. 
				break; //(database.h'da keywords[5].c_code="= ") 
			default://LOOP gelirse ) bas�l�r.
				fputs(keywords[i].c_code,fp2);
				break;
		}
	}
	return i;//i de�eri kullan�laca�� i�in geri d�nd�r�r.
}
void GetVal_PrintLine(FILE *fp2, char *token, int i)
{
	DATABASE *ptr;
	char *tmpcode;//token'� kaybetmemem laz�m. tmpcode'a kopyalanacak.
	fputs(keywords[i].c_code,fp2);//printf(" veya scanf(" yaz�l�r.
	tmpcode=(char*)malloc(LINESIZE);
	strcpy(tmpcode,token);//tmpcode'a token'� kopyala.
	while(token=strtok(NULL,"�� \"]\n\t"))//sat�r sonuna kadar,
	{				//ayarlanm�� olan delimiterlerle tokenize et.
		strcat(tmpcode,token);//tmpcode'a token'� ekle
		if(strchr(token,'$'))//$ karakteri geldiyse,
		{		//ilgili uzant�larla uyu�ma durumuna g�re,
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
				if(strchr(token,','))//virg�l varsa
				{ //virg�l yaz.
					putc(',',fp2);
				}
				if(strchr(token,'.'))//nokta varsa,
				{ //nokta yaz.
					putc('.',fp2);
				}
				putc(' ',fp2);//okunabilirlik i�in bo�luk b�rak
			}
		}
		else//$ karakteri yoksa d�z kelimedir.
		{ //direkt yaz
			fputs(token,fp2);
			putc(' ',fp2);//bo�luk b�rak.
		}
	}
	fseek(fp2,-1,SEEK_CUR);//sona gelince ekstra bo�lu�u sil
	if(i==20)
	{ //printf ise \n  " ve ,  yaz�lmal�.
		fputs("\\n\", ",fp2);
	}
	else
	{	//de�ilse sadece " ve , yaz�lmal�.	
		fputs("\", ",fp2);
	}
	if(strchr(tmpcode,'$'))//$ karakteri varsa
	{ //de�i�ken kullan�lm��t�r.yoksa d�z metin yaz�lm��t�r.
		token=strtok(tmpcode," $\n\t");//$ dan �ncesini al ama yazma, d�z metin.
		while(token=strtok(NULL," $\n\t"))
		{ //sat�r sonuna kadar tokenize et.
			if(strchr(token,'['))
			{ //[ varsa dizidir.
				if(i==21)
				{ //scanf ise & eklenmeli
					putc('&',fp2);
				}
				isArray(fp2,token);//isArray'i �a��r.
				fputs(", ",fp2);//sonraki elemanla ay�r.
			}
			for(ptr=head;ptr!=NULL;ptr=ptr->db_next)
			{
				if(strstr(token,ptr->token))
				{
					if(i==21)
					{ //scanf ise 
						if(!ptr->ispointer)//pointer de�ilse
							putc('&',fp2);//& eklenmeli.
					}
					fputs(ptr->identifier,fp2);//de�i�ken ad� yaz�l�r.
					fputs(", ",fp2);//sonraki elemanla ayr�l�r.
					break;
				}
			}
		}
	}
	fseek(fp2,-2,SEEK_CUR);//fazladan yaz�lan , ve bo�lu�u siler.
	putc(')',fp2);
	return;
}
void print_Vars(FILE *fp2, char *Proc,int k)
{
	DATABASE *ptr;
	int i;
	char text[4];//dizi boyutu i�in.(maksimum 999 olabilir)
	for(i=0;i<USES_VARS_SIZE;i++)
	{			//t�m de�i�ken tipleri i�in d�necek.
		if(uses_vars[k][i]==TRUE)
		{		//ilgili de�i�ken tipi varsa
			fputs(vars[i],fp2); //de�i�ken tipini basar,
			putc(' ',fp2);		//bo�luk b�rak�r.
			for(ptr=head;(ptr!=NULL)&&(ptr->data_type!=vars[i]);ptr=ptr->db_next);
			for(;(ptr!=NULL)&&(ptr->data_type==vars[i]);ptr=ptr->db_next)
			{	//ilgili de�i�ken tipinin oldu�u k�sma gelene kadar null statement d�ner.
				//de�i�ken tipi de�i�medi�i s�rece d�ner.
				if((!_stricmp(Proc,ptr->proc))&&(!ptr->isPara))//Parametreyse tan�mlama.
				{  //�uanki fonksiyon, ptr'�n bulundu�u fonksiyonsa,
					if(ptr->ispointer)//buraya girer
					{		//ptr pointersa * basar
						putc('*',fp2);
					}//de�i�ken ad�n� yazar.
					else if(i==2)
						putc('*',fp2);
					fputs(ptr->identifier,fp2);
					if(ptr->isarray)
					{ //de�i�ken bir diziyse
						putc('[',fp2);//[ basar.
						fputs(itoa(ptr->isarray,text,10),fp2);//int to string yap�l�p
						putc(']',fp2);//dizi boyutu bas�l�r ve ] bas�l�r.
					}
					putc(',',fp2);//sonraki elemandan ayr�l�r.
					putc(' ',fp2);//okunabilirlik i�in bo�luk koyulur.
				}
			}		//fazladan yaz�lan , ve bo�luk silinir.
			fseek(fp2,-2,SEEK_CUR);
			fputs(";\n\t",fp2);//sat�r sonuna gelindi, alta ge�ilir.
		}
	}
}
DATABASE * arrange_bools(char *ident,char *dtype, char *tmptoken,char *ifarray)
{
	DATABASE *ptr;//ptr'a parametrelerle yer a��l�r.
	ptr=create_list_element(strlen(ident),strlen(dtype)+1,strlen(tmptoken));
	if(*ifarray)
	{ //diziyse ilgili k�s�m set,
		ptr->isarray=TRUE;
	}
	else
	{ //dizi de�ilse ilgili k�s�m false yap�l�r.
		ptr->isarray=FALSE;
	}
	ptr->data_type=dtype;//de�i�ken tipi ilgili k�sma yaz�l�r.
	return ptr;//ptr de�eri d�ner
}
void return_type(FILE *fp2,char *token,long int pos)
{
	DATABASE *ptr;//RETURN komutu varsa bu fonksiyon �a�r�l�r.
	int space_len,i;//space_len, d�n�� tipine g�re ayarlanacak.
	token=strtok(NULL," _\n\t");//de�i�ken ad� al�n�r.
	if(!strcmp(token,"VOID"))//VOID'se
	{					//void yaz�l�r.
		fputs("void",fp2);
		return;//ve ��k�l�r.
	} //void de�ilse alt sat�ra ge�er.
	fputs(token,fp2);//de�i�ken ad� yaz�l�r.
	fseek(fp2,pos,SEEK_SET);//ilgili fonksiyonun ba�lang��� pos'ta var.oraya gidilir.
	for(ptr=head;(ptr->db_next!=NULL)&&(strcmp(token,ptr->identifier));ptr=ptr->db_next);
	fputs(ptr->data_type,fp2);//�lgili de�i�keni bulana kadar null for d�ner.
	space_len=11-strlen(ptr->data_type);//space len de�i�ken tipine g�re ayarlan�r.
	for(i=0;i<space_len;i++)//space_len kadar bo�luk koy.
		putc(' ',fp2);
	token=strtok(NULL," \t\n");//sat�r sonuna gelmek i�in yaz�l�r.
	if(ptr->ispointer) 
	{			//de�i�ken pointersa
		putc('*',fp2);//* koyulur.
	}
	else if(!strcmp("int",ptr->data_type))
	{//pointer de�ilse, int ise ekstra bir bo�luk koyulur,
		putc(' ',fp2);//��nk� int void'den 1 karakter k�sa.
	}
	putc(' ',fp2);
	if(!strcmp(ptr->proc,"MAIN"))//main i�in �zel durum var.
		fputs("main()",fp2);//
	else//main de�ilse, de�i�kenin ait oldu�u,
		fputs(ptr->proc,fp2);//fonksiyon yaz�l�r.
	fseek(fp2,0,SEEK_END);//dosya sonuna geri d�n.
}