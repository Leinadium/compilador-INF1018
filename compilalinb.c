#include <stdio.h>
#include <stdlib.h>

typedef int (*funcp) ();

/* temos 4 tipos de comandos */
/* atribuicao, que possui uma operacao */
/* desvio */
/* retorno */

typedef struct var {
    int n;          // valor da constante, ou numero do parametro/variavel
    char t;         // 'v' se variavel, 'p' se parametro, '$' se constante
} Var;


typedef struct linha {
    Var v1;
    Var v2;
    Var v3;
    char tipo;      // 'a' = atribuicao / 'd' = desvio / 'r' = retorno
    char op;        // contem operacao se 'a', destino do pulo se 'd'

} Linha;

void preenche_linhas(FILE *f, Linha *linhas, int *q_linhas)
{
    char c;
    int q = 0;

    while ((c = fgetc(f)) != EOF )          //le ate chegar ao fim do arquivo
    {
        /* nova linha! */
        if ((c == 'v') || (c == 'p'))       // ATRIBUICAO
        {
            printf("Achei uma atribuicao.\n");
            linhas[q].tipo = 'a';  
            linhas[q].v1.t = c;
            fscanf(f, "%d = %c%d %c %c%d", &linhas[q].v1.n, 
                                        &linhas[q].v2.t, &linhas[q].v2.n, 
                                        &linhas[q].op, 
                                        &linhas[q].v3.t, &linhas[q].v3.n);
        }
        else if (c == 'i')
        {
            printf("Achei um if.\n");
            linhas[q].tipo = 'd';
            fscanf(f, "f %c%d %c", &linhas[q].v1.t, &linhas[q].v1.n,
                                    &linhas[q].op);

        }
        else if (c == 'r')
        {
            printf("Achei um retorno.\n");
            linhas[q].tipo = 'r';
            fscanf(f, "et");
        }
        else
        {
            printf("achei alguma coisa estranha: %c", c);
        }

        while ((c = fgetc(f)) != '\n') {if (c==EOF) break;} //ve se tem algum espaco depois
        q++;
    }

    printf("\n\n O total de linhas eh de %d.\n", q);
    *q_linhas = q;
    return;
}

//funcp CompilaLinB (FILE *f)
void CompilaLinB (FILE *f)
{
    Linha linhas[50];
    int q_linhas = 0;

    preenche_linhas(f,linhas, &q_linhas);

    return;
}



int main(int argc, char *argv[]) {
    
    FILE *f = fopen(argv[1], "r");
    if (f==NULL) {printf("Erro ao abrir arquivo. "); exit(1);}

    CompilaLinB(f);

    return 0;
}