#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 


void read_ver(FILE *fptr, char first[][10],char second[][10], char third[][10]);
int inOpcode(char ch[]);
int inSymtab(char ch[]);
int hexToDec(char ch[10]);
int opNum(char ch[]);
void decToBin(int n,int bin[30], int first, int last);
void binToHex(int bin[24],char obj[6]);
int inSymTab(char ch[]);

char opcode[59][2][10];
char sym[100][10];
int symadd[100]; //以十進位保存symbol的位址 
int sym_index=0;
int objcode[100][24];//存放二進位 
int addr[100];
char obj_16[100][6];

int main()
{
	FILE *fptr1,*fptr2;
	char label[100][10],instr[100][10],opper[100][10];

	int error=0;
	int i,j,endLine=0,locctr=0,start;
	int loc[100];//存取以十進位表示之loc
	char temp[10]="";
 
	
	//初始化各個參數
	for(i=0;i<100;i++)
	{
		strcpy(label[i],"");
		strcpy(sym[i],"");
		strcpy(instr[i],"");
		strcpy(opper[i],"");
		symadd[i]=0;
		addr[i]=0;
		loc[i]=0;
		for(j=0;j<24;j++)
			objcode[i][j]=0;
	} 
	
	fptr1 = fopen("opcode.txt", "r");
	if(fptr1 == NULL)
	{
		printf("Error!");
	}

	for(i=0;i<59;i++)
	{
		for(j=0;j<2;j++)
		{
			fscanf(fptr1,"%s ",opcode[i][j]);
		}
	}
	
	fptr2 = fopen("source.txt", "r");
	if(fptr2 == NULL)
	{
		printf("Error!");
	}
	read_ver(fptr2,label,instr,opper);
	
	i=0;
	while(strcmp(instr[i],"END")!=0)
	{
		i++;
	}
	endLine=i+1;
//	printf("Endline:%d",endLine);
	
	for(i=0;i<endLine;i++)
	{
		if(strcmp(instr[i],"START")==0)
		{
			strcpy(sym[sym_index],instr[i]);
			symadd[sym_index]=hexToDec(opper[i]);
			locctr = symadd[sym_index];
			loc[i]= symadd[sym_index];
			start=i+1; //從下一行開始讀 
			sym_index++;
			break;
		}
	}
	
	i=start;//代表第幾行 
	while(strcmp(instr[i],"END")!=0)
	{
		if(strcmp(label[i],"")!=0)
		{
			if(inSymtab(label[i]))
			{
				error=1;
				printf("[ERROR]:Duplicate symbols!!!");
			}
				
			else
			{
				strcpy(sym[sym_index],label[i]);
				symadd[sym_index]=locctr;
				sym_index++;
			}			
		}
		if(inOpcode(instr[i])>=0)
		{
			loc[i]=locctr;
			locctr+=3;	
		}
		else if(strcmp(instr[i],"WORD")==0)
		{
			loc[i]=locctr;
			locctr+=3; 
		} 
		else if(strcmp(instr[i],"RESW")==0)
		{
			loc[i]=locctr;
			int op = atoi(opper[i]);
			locctr = locctr + 3*op; 
		}
		else if(strcmp(instr[i],"RESB")==0)
		{
			loc[i]=locctr;
			int op = atoi(opper[i]);
			locctr += op;
		}
		else if(strcmp(instr[i],"BYTE")==0)
		{
			loc[i]=locctr;
			int len=0;
			
			if(opper[i][0]==67) //若第一個字是C 
			{
				int bound[2]={0,0},k=0;
				for(j=0;j<strlen(opper[i]);j++)
				{
					if(opper[i][j]==39)
					{
						bound[k++]=j;
					} 
				}
				len=bound[1]-bound[0]-1;
			} 
			else if(opper[i][0]==88) //若第一個字是X 
			{
				int bound[2]={0,0},k=0;
				for(j=0;j<strlen(opper[i]);j++)
				{
					if(opper[i][j]==39)
					{
						bound[k++]=j;
					} 
				}
				len=(bound[1]-bound[0]-1)/2;
			}
			locctr+=len;
		}
		
		else if((strcmp(instr[i++],"END"))==0)
		{
			break;
		}
		else
		{
			error=1;
		}
		i++;
	}
	//輸出結果 
	
	for(i=0;i<endLine-1;i++)
	{
		printf("%X\t%s\t%s\t%s\n",loc[i], label[i],instr[i],opper[i]);
	}
		printf("\t%s\t%s\t%s\n",label[endLine-1],instr[endLine-1],opper[endLine-1]);
	 
	
	printf("---Symbol Table---\n");
	for(i=0;i<sym_index;i++)
	{
		printf("%s\t%X\n",sym[i],symadd[i]);
	}
	

	for(i=0;i<sym_index;i++) //去掉檔案中不必要的空白 
	{
		for(j=0;j<10;j++)
		{
			if(sym[i][j]==32)
			{
				sym[i][j]='\0';
			}
		}
	}
	
	printf("\n");
	
	//Pass2
	for(i=start;i<endLine;i++) //從start的下一行開始 
	{
		int n=-1;
		n=opNum(instr[i]);
		if(n>=0) //尋找op number 
		{
			decToBin(n,objcode[i],0,7);	//把op code 填入 
			
			if(strcmp(opper[i],"")==0)
			{
				for(j=8;j<24;j++)
					objcode[i][j]=0;
				binToHex(objcode[i],obj_16[i]);
				
			}
			else
			{
				char* check=strstr(opper[i],",");//檢查是否有"," 
				if(check)
				{
					//分割字串 
					char temp_opp[2][10];
					char *ptr = strtok(opper[i],",");
					i=0;
					while(ptr!=NULL)
					{
						strcpy(temp_opp[i],ptr);
						ptr = strtok(NULL,",");
						i++;
					}
					if(i>2)	//大於兩項，輸入格式錯誤 
					{
						error=1;
						printf("ERROR!!\n");
						break;
					}
					if(strcmp(temp_opp[1],"X")==0)
					{
	
						int tempdisp=inSymTab(temp_opp[0]);
						if(tempdisp>=0)	//Found
						{
							decToBin(tempdisp,objcode[i],8,24);
							objcode[i][8]=1;	//set x=1
							binToHex(objcode[i],obj_16[i]);
						} 
						else if(tempdisp==-1)
						{
							for(j=8;j<24;j++)
								objcode[i][j]=0;
							binToHex(objcode[i],obj_16[i]);
							printf("teamp_opp[i]:%s\n",temp_opp[i]); 
							printf("[ERROR]：Undefined symbol!!!\n");
						}
					} 
					else
					{
						error=1;
						printf("ERROR!!\n");
						break;
					}
					
					
				}
				else
				{
					
					int tempdisp=inSymTab(opper[i]);
					if(tempdisp>=0)//Found
					{
						decToBin(tempdisp,objcode[i],8,23);
						binToHex(objcode[i],obj_16[i]);
					} 
					else if(tempdisp==-1)
					{
						for(j=8;j<24;j++)
							objcode[i][j]=0;
						binToHex(objcode[i],obj_16[i]);
						printf("opper:%s\n",opper[i]); 
						printf("[ERROR]：Undefined symbol!!!\n");
					}
				}
				
				
			}
			
			
		}
		else if (strcmp(instr[i],"BYTE")==0 )
				
		{
			if(opper[i][0]==67)
			{
				j=1;
				while(opper[i][j] !=39)
					j++;
				j++;
				int iForObj=0;
				
				while(opper[i][j] !=39)
				{
					int ascii=opper[i][j];
					decToBin(ascii,objcode[i],iForObj,iForObj+7);
					j++;
					iForObj+=8;
				}
				binToHex(objcode[i],obj_16[i]);
			}
			else if(opper[i][0]==88)
			{
				int iForObj16=0;
				j=1;
				while(opper[i][j] !=39)
					j++;
				j++;
				while(opper[i][j] !=39)
				{
					obj_16[i][iForObj16]=opper[i][j];
					iForObj16++;
					j++;
				}
					
				
			}
			
		
		}
		else if(strcmp(instr[i],"WORD")==0)
		{
			j=0;
			int sum=0,len=strlen(opper[i]),n;
			while(len>0)
			{
//				printf("sum:%d\n",sum);
				sum+=(opper[i][j]-48)*pow(10,len-1);
				
				j++;
				len--;
			}
//			printf("sum:%d\n",sum);
			n=sum;
			j= 5;
			while(n>0)
			{
				if((n%16)>=0&&(n%16)<16)
					obj_16[i][j]= n%16+48;
				else
				{
					switch (n%16)
					{
						case 10:
							obj_16[i][j]= 65;
							break;
						case 11:
							obj_16[i][j]= 66;
							break;
						case 12:
							obj_16[i][j]= 67;
							break;
						case 13:
							obj_16[i][j]= 68;
							break;
						case 14:
							obj_16[i][j]= 69;
							break;
						case 15:
							obj_16[i][j]= 70;
							break;
					
					}
				}
				n/=16;
				j--;
			}
			while(j>=0)
			{
				obj_16[i][j]=48;
				j--;
			}
		}
	}
	
	printf("-----Pass2-----\n");
	for(i=0;i<endLine-1;i++)
	{
		printf("%X\t%s\t%s\t%s\t",loc[i],label[i],instr[i],opper[i]);
		for(j=0;j<6;j++)
		{
			
			printf("%c",obj_16[i][j]);
		}
			
		printf("\n");
	}
	printf("\t%s\t%s\t%s\t",label[endLine-1],instr[endLine-1],opper[endLine-1]);
		for(j=0;j<6;j++)
			printf("%c",obj_16[i][j]);
	printf("\n");
	
	
	
	
	return 0;
} 



void read_ver(FILE *fptr, char first[][10],char second[][10], char third[][10]) //將檔案以直行的方式讀取後，儲存於陣列中 
{
	char nextline[100]="";
	int line=0;
	
	while(!feof(fptr))
	{
		
		int count=0;
		fscanf(fptr,"%[^\n]",nextline); //讀取新的一行 
		char ch= getc(fptr); //吸收換行符號 
		char *lineptr = strtok(nextline,"\t"); //以"\t"作為分隔符號 
		
		
		while (lineptr != NULL) 
		{
			if(count==0)
			{
				strcpy(first[line],lineptr);
			}
			else if(count ==1)
			{
				strcpy(second[line],lineptr);
			}
			else if(count==2)
			{
				strcpy(third[line],lineptr);
			}
			count++; 
        	lineptr = strtok(NULL, "\t");
    	}
    	
		if(count!=3)
		{
			if(count ==1)
			{
				strcpy(second[line],first[line]);
				strcpy(first[line],"");
			}
			else if(count==2)
			{
				strcpy(third[line],second[line]);
				strcpy(second[line],first[line]);
				strcpy(first[line],"");
			}
			
		}
		line++;	
	}
}

int inOpcode(char ch[])
{
	int i;
	for(i=0;i<59;i++)
	{
		if(strcmp(ch,opcode[i][0])==0)
		{
			return 1;
		}
		
	}
	return -1;
}

int inSymtab(char ch[])
{
	int i;
	for(i=0;i<sym_index;i++)
	{
		if(strcmp(ch,sym[i])==0)
		{
			return 1;
		}
	}
	return 0;
}

int inSymTab(char ch[])
{
	int i;
	for(i=0;i<sym_index;i++)
	{
		if(strcmp(ch,sym[i])==0)
		{
			return symadd[i];
			
		}
	}
	return -1;
} 

int hexToDec(char ch[10])
{
	int i,sum=0;
	for(i=strlen(ch)-1;i>=0;i--)
	{
		if((ch[i]>'9'|| ch[i]<'0'))
		{
			switch (ch[i])
			{
				case 'A':
					sum += pow(16,strlen(ch)-i-1)*10;
					break;
				case 'B':	
					sum += pow(16,strlen(ch)-i-1)*11;
					break;
				case 'C':	
					sum += pow(16,strlen(ch)-i-1)*12;
					break;
				case 'D':	
					sum += pow(16,strlen(ch)-i-1)*13;
					break;
				case 'E':	
					sum += pow(16,strlen(ch)-i-1)*14;
					break;
				case 'F':	
					sum += pow(16,strlen(ch)-i-1)*15;
					break;	
			}
			
		}
		else
		{
			int temp=ch[i]-48;
			sum += pow(16,strlen(ch)-i-1)*temp;
		}
	}
	return sum;
	
}


int opNum(char ch[])
{
	int i;
	for(i=0;i<59;i++)
	{
		if(strcmp(ch,opcode[i][0])==0)
		{
//			printf("opNum:%s\n",opcode[i][1]);
			return hexToDec(opcode[i][1]);
		}
		
	}
	return -1;
}

void binToHex(int bin[24],char obj[6])
{
	int i,sum=0,j=0;
	for(i=0;i<24;i+=4)
	{
		sum+=bin[i]*8+bin[i+1]*4+bin[i+2]*2+bin[i+3]*1;
		if(sum<10 && sum>=0)
		{
			obj[j]= 48 + sum;
		}
		else
		{
			switch (sum)
			{
				case 10:
					obj[j]= 65;
					break;
				case 11:
					obj[j]= 66;
					break;
				case 12:
					obj[j]= 67;
					break;
				case 13:
					obj[j]= 68;
					break;
				case 14:
					obj[j]= 69;
					break;
				case 15:
					obj[j]= 70;
					break;
					
			}
		}
		j++;
		sum=0;
	}
}

void decToBin(int n,int bin[24], int first , int last) //for Object code
{
	int size=last-first+1;
	int temp[size];
	int num=n,i=0,len=0,j=0;
	
	for(i=0;i<size;i++)
		temp[i]=0;
//	printf("num:%d\n",num);
	i=0;
	while(num>0)
	{
		temp[i]=num%2;
//		printf("%d",temp[i]);
		num /=2;
		i++;
	}
//	printf("i:%d\n",i);
	
	j=size-1;

	for(i=first;i<=last;i++)
	{
		bin[i]=temp[j];
//		printf("%d",bin[i]);
		j--;
	}

}


