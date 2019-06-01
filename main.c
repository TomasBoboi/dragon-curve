#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_L 2850

char c_e,c_f,s[262144],A[MAX_L][MAX_L];
unsigned int i,j,NR_IT,L_MIN,L_MAX,C_MIN,C_MAX;


void NEXT(char s[],unsigned int S)
{
    int i;
    i=S-1;
    while(i>=0)
    {
        if(s[i]=='U')
            strcat(s,"L");
        else if(s[i]=='R')
            strcat(s,"U");
        else if(s[i]=='D')
            strcat(s,"R");
        else strcat(s,"D");
        i--;
    }
}
void dragon(char A[MAX_L][MAX_L],char s[],unsigned int x0,unsigned int y0,char c_e,char c_f)
{
    unsigned int i,j,k;
    for(i=0;i<MAX_L;i++)
        for(j=0;j<MAX_L;j++)
            A[i][j]=c_e;
    i=x0;j=y0;
    for(k=0;k<strlen(s);k++)
    {
        A[i][j]=c_f;
        //printf("Generating matrix: %f % \n",((float)(k)/strlen(s))*100);
        if(s[k]=='U')
        {
            i--;A[i][j]=c_f;i--;
        }
        else if(s[k]=='R')
        {
            j++;A[i][j]=c_f;j++;
        }
        else if(s[k]=='D')
        {
            i++;A[i][j]=c_f;i++;
        }
        else
        {
            j--;A[i][j]=c_f;j--;
        }
    }
}
void min_dragon(char A[MAX_L][MAX_L])
{
    unsigned int L_MIN=0,L_MAX=0,C_MIN=MAX_L,C_MAX=0;
    char file_name[10]="";
    for(i=0;i<MAX_L;i++)
        for(j=0;j<MAX_L;j++)
            if(A[i][j]==c_f)
            {
                if(L_MIN==0)
                    L_MIN=i;
                L_MAX=i;
                if(j>C_MAX)
                    C_MAX=j;
                if(j<C_MIN)
                    C_MIN=j;
            }

    unsigned int MAXF_L=L_MAX-L_MIN+1,MAXF_C=C_MAX-C_MIN+1;
    char A_FIN[MAXF_L][MAXF_C];
    for(i=0;i<MAXF_L;i++)
        for(j=0;j<MAXF_C;j++)
            A_FIN[i][j]=A[i+L_MIN][j+C_MIN];

    FILE *f;
    snprintf(file_name,25,"dragon_%u.txt",NR_IT);
    f=fopen(file_name,"wt");
    for(i=0;i<MAXF_L;i++)
    {
        for(j=0;j<MAXF_C;j++)
            fprintf(f,"%c",A_FIN[i][j]);
        fprintf(f,"\n");
    }
    fclose(f);
}
int main()
{
    printf("No. of iterations (up to 18): ");
    scanf("%u",&NR_IT);

    c_e=' ';
    c_f='@';

    s[0]='L';
    printf("Generating steps...\n");
    for(i=1;i<=NR_IT;i++)
    {
        //printf("Generating steps: %2.3f % \n",((float)(i)/NR_IT)*100);
        NEXT(s,strlen(s));
    }

    printf("Generating matrix...\n");
    dragon(A,s,MAX_L/2+12*NR_IT,MAX_L/4+24*NR_IT,c_e,c_f);

    printf("Writing to file...\n");
    min_dragon(A);
    return 0;
}
