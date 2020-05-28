#include <stdio.h>
#include <stdlib.h>

typedef int (*funcp) ();

/* temos 4 tipos de comandos */
/* atribuicao, que possui uma operacao */
/* desvio */
/* retorno */

typedef struct linha {
    int id; /* numero da linha da instrucao */
    char *s_inteira;
    int len_s_inteira;
    
    char tipo; /* 'a', 'i' ou 'r' */


} Linha;

/*
funcp CompilaLinB (FILE *f)
{
    int c = 0;
    return c;
}
*/


int main(int argc, char *argv[]) {
    
    char c;
    char *linha_inteira;
    Linha programa[50];

    FILE *f = fopen(argv[1], "r");
    if (f==NULL) {printf("Erro ao abrir arquivo. "); exit(1);}

    while ((c = fgetc(f)) != EOF )
    {
        /* nova linha! */
        if ((c == 'v') || (c == 'p'))
        {
            printf("Achei uma atribuicao.\n");
        }
        else if (c == 'i')
        {
            printf("Achei um if.\n");
        }
        else if (c == 'r')
        {
            printf("Achei um retorno.\n");
        }
        else
        {
            printf("achei alguma coisa estranha: %c", c);
        }

        while ((c = fgetc(f)) != '\n') {if (c==EOF) break;}
        
    }
    return 0;
}