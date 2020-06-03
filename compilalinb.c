#include <stdio.h>
#include <stdlib.h>

typedef int (*funcp) ();

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
    int q_assembly;

} Linha;

typedef struct assembly {
    char *lista;
    int q;
} Assembly;

void dump(int *l, int tamanho)
{
    int i;
    printf("\nDUMP FINAL: ");
    for (i=0;i<tamanho;printf("%x ",l[i++]));
    return;
}


int *junta_binario(Assembly *lista_binario, int q, int *tamanho_final)
{
    int* lista_final;
    int tamanho_total = 0;
    int i, j, k = 0;
    printf("Juntando binario final.\n");
    for (i=0; i<q;i++)
    {
        tamanho_total += lista_binario[i].q;
    }

    lista_final = (int*)malloc(sizeof(int)*tamanho_total);
    for (i=0; i<q; i++)
    {
        for (j=0;j<lista_binario[i].q;j++)
        {
            lista_final[k++] = lista_binario[i].lista[j] & 0xff;
        }
    }
    printf("o tamanho da lista final final final eh de %d", k);
    *tamanho_final = tamanho_total;
    return lista_final;

}


void monta_desvio(Linha *linhas, int indice_linha, Assembly *lista_assembly, int indice_assembly)
{

    // eu tenho a minha posicao na lista binario.
    // preciso calcular o offset ate o indice da linha q quero
    int destino_linha = linhas[indice_linha].v1.n - 1;
    int offset_linha = destino_linha - indice_linha;
    int pulo_bytes = 0;
    int copia_indice_assembly = indice_assembly;
    int tamanho_linha_assembly;
    int i;
    
    if (offset_linha == 0)
    {
        printf("LOOP INFINITO DETECTADO.\n");
        exit(1);
    }

    else if (offset_linha > 0)
    {
        /* pulo_bytes = 1; */
        copia_indice_assembly += 1;
        while (offset_linha > 1)
        {
            indice_linha++;
            tamanho_linha_assembly = linhas[indice_linha].q_assembly;
            for (i=0;i<tamanho_linha_assembly;i++)
            {
                pulo_bytes += lista_assembly[copia_indice_assembly++].q;
            }
            offset_linha = destino_linha - indice_linha;
        }
    }
    else // offset negativo :0 
    {
        // sabemos que a linha que estamos contem 3 ou 4 bytes com certeza, + 2 ou 6 dependendo do tamanho do pulo
        pulo_bytes -= lista_assembly[copia_indice_assembly - 1].q;
        copia_indice_assembly-= 2; //ja comecando na linha debaixo
        // mas teoricamente, o pulo sera a quantidade de bytes que temos nas linha debaixo
        // pra isso, vamos contar a os bytes da nossa linha, e descer
        do
        {
            indice_linha--;
            tamanho_linha_assembly = linhas[indice_linha].q_assembly;
            for (i=0;i<tamanho_linha_assembly;i++)
            {
                pulo_bytes -= lista_assembly[copia_indice_assembly--].q;
            }
        } while (indice_linha != destino_linha);
    }
    if (offset_linha <0) pulo_bytes -= 2; // contando o seu proprio tamanho.
    if ((pulo_bytes < 127) && (pulo_bytes > -128))
        {
            char *b = (char *)malloc(sizeof(char)*2);
            lista_assembly[indice_assembly].q = 2;
            b[0] = 0x75;
            b[1] = (char) pulo_bytes;
            lista_assembly[indice_assembly].lista = b;
        }
        else
        {
            if (offset_linha >0) pulo_bytes -= 4; // o seu proprio tamanho foi de 2 a 6
            char *b = (char *)malloc(sizeof(char)*6);
            lista_assembly[indice_assembly].q = 6;
            b[0] = 0x0f;
            b[1] = 0x85;
            b[2] = (char) pulo_bytes;
            b[3] = (char) pulo_bytes >> 8;
            b[4] = (char) pulo_bytes >> 16; 
            b[5] = (char) pulo_bytes >> 24;
            lista_assembly[indice_assembly].lista = b;
        }
    
    return;
}

char *monta_inicio(int *q)
{
    /*
        pushq   %rbp
        movq    %rsp, %rbp
        subq    $16, %rsp
    */
    char *b = (char*)malloc(sizeof(char)*8);
    *q = 8;
    b[0] = 0x55;
    b[1] = 0x48;
    b[2] = 0x89;
    b[3] = 0xe5;
    b[4] = 0x48;
    b[5] = 0x83;
    b[6] = 0xec;
    b[7] = 0x10;
    return b;
}

char *monta_retorno(int *q)
{
    /*
        movl    -16(%rbp), %eax
        add     $16, %rsp
        leave
        ret
    */
   *q = 9;
    char *b = (char*)malloc(sizeof(char)*9);
    b[0] = 0x8b;
    b[1] = 0x45;
    b[2] = 0xf0;
    b[3] = 0x48;
    b[4] = 0x83;
    b[5] = 0xc4;
    b[6] = 0x10;
    b[7] = 0xc9;
    b[8] = 0xc3;
    return b;
}

char *monta_binario(char tipo, Var *var, int *q)
{
    /*
        m = mov v eax
        n = mov eax v
        a = add v eax
        s = sub v eax
        i = imu v eax
        d = cmp $0, v
    */
    char *b;
    // recebe comando e variavel.
    // retorna lista de binario, com a quantidade
    if (tipo == 'm')  //mov v1 %eax
    {
        if (var->t == '$')
        {
            *q = 5;
            b = (char*)malloc(sizeof(char)*5);
            b[0] = 0xb8;
            b[1] = (char) var->n;
            b[2] = (char) var->n >> 8;
            b[3] = (char) var->n >> 16;
            b[4] = (char) var->n >> 24;
        }
        else if (var->t == 'p')
        {
            *q = 2;
            b = (char*)malloc(sizeof(char)*2);
            b[0] = 0x89;
            b[1] = (var->n == 1) ? 0xf8 : 0xf0; 
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            b[0] = 0x8b;
            b[1] = 0x45;
            b[2] = 0xf0 + 4 * ((char) var->n - 1);
        }
        
    }
    else if (tipo == 'n')
    {
        if (var->t == 'p')
        {
            *q = 2;
            b = (char*)malloc(sizeof(char)*2);
            b[0] = 0x89;
            b[1] = (var->n == 1) ? 0xc7 : 0xc6;
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            b[0] = 0x89;
            b[1] = 0x45;
            b[2] = 0xf0 + 4*(((char)var->n)-1);
        }
        
    }
    else if ((tipo == 'a') || (tipo == 's'))
    {
        if (var->t == '$')
        {
            if ((var->n < 16)&&(var->n > -15))
            {
                *q = 3;
                b = (char*)malloc(sizeof(char)*3);
                b[0] = 0x83;
                b[1] = (tipo == 'a') ? 0xc0 : 0xe8;
                b[2] = (char) var->n;
            }
            else
            {
                *q = 5;
                b = (char*)malloc(sizeof(char)*5);
                b[0] = (tipo == 'a') ? 0x05 : 0x2d;
                b[1] = (char) var->n;
                b[2] = (char) *(&(var->n) + 1);
                b[3] = (char) *(&(var->n) + 2);
                b[4] = (char) *(&(var->n) + 3);
            }
            
        }
        else if (var ->t == 'p')
        {
            *q = 2;
            b = (char*)malloc(sizeof(char)*2);
            b[0] = (tipo == 'a') ? 0x01: 0x29;
            b[1] = (var->n == 1) ? 0xf8 : 0xf0;
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            b[0] = (tipo == 'a') ? 0x03 : 0x2b;
            b[1] = 0x45;
            b[2] = 0xf0 + 4 * ((char) var->n - 1);
        }
    }
    else if (tipo == 'i')
    {
        if (var->t == '$')
        {
            if ((var->n < 16)&&(var->n > -15))
            {
                *q = 3;
                b = (char*)malloc(sizeof(char)*3);
                b[0] = 0x6b;
                b[1] = 0xc0;
                b[2] = (char) var->n;
            }
            else
            {
                *q = 6;
                b = (char*)malloc(sizeof(char)*6);
                b[0] = 0x69;
                b[1] = 0xc0;
                b[2] = (char) var->n;
                b[3] = (char) *(&(var->n) + 1);
                b[4] = (char) *(&(var->n) + 2);
                b[5] = (char) *(&(var->n) + 3);
            }
         
        }
        else if (var->t == 'p')
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            b[0] = 0x0f;
            b[1] = 0xaf;
            b[2] = (var->n == 1) ? 0xc7 : 0xc6;
        }
        else
        {
            *q = 4;
            b = (char*)malloc(sizeof(char)*4);
            b[0] = 0x0f;
            b[1] = 0xaf;
            b[2] = 0x45;
            b[3] = 0xf0 + 4 * ((char) var->n - 1);
        }        
    }
    else /* if (tipo == 'd') */
    { 
        if (var->t == 'p')
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            b[0] = 0x83;
            b[1] = (var->n == 1) ? 0xff : 0xfe;
            b[2] = 0x00;
        }
        else
        {
            *q = 4;
            b = (char*)malloc(sizeof(char)*4);
            b[0] = 0x83;
            b[1] = 0x7d;
            b[2] = 0xf0 + 4 * ((char) var->n - 1);
            b[3] = 0x00;
        }
        
    }
    // MONTA SAIDA AA
    return b;
}

int* monta_assembly(Linha *linhas, int q_linhas, int q_linhas_assembly, int *tamanho_final)
{
    int i, j = 1, k = 0; // variaveis contadoras
     // i -> looping nas linhas do arquivo 
     // j -> indice das linhas_assembly (comecando em 1, pois 0 eh o inicio)
     // k -> indice de lista_desvios_linhas e tbm lista_desvios_assembly
    int temp;
    int * resposta;
    int q_desvios;
    int *lista_desvios_linhas;
    int *lista_desvios_assembly;

    Assembly *linhas_assembly = (Assembly *)malloc(sizeof(Assembly)*q_linhas_assembly);
    if (linhas_assembly == NULL)
    {
        printf("ERRO CRIANDO LINHAS ASSEMBLY.\n");
        exit(1);
    }
    // cada linhas possui um comando
    // esse comando tem uma lista alocada dinamicamente, e a qtd de bytes nesse comando
    printf("montando assembly: ");

    //inicio
    linhas_assembly[0].lista = monta_inicio(&linhas_assembly[0].q);

    for (i=0; i<q_linhas;i++) {if (linhas[i].tipo == 'd') q_desvios++;}
    lista_desvios_linhas = (int *)malloc(sizeof(int)*q_desvios);
    lista_desvios_assembly = (int *)malloc(sizeof(int)*q_desvios);

    for (i=0; i<q_linhas; i++)
    {
        switch (linhas[i].tipo)
        {
            case 'a': 
            {
                // movl v2, %eax
                linhas_assembly[j].lista = monta_binario('m',&linhas[i].v2,&linhas_assembly[j].q);
                j++;
                // opr v3 %eax
                switch (linhas[i].op)
                {
                    case '+':
                    {linhas_assembly[j].lista = monta_binario('a',&linhas[i].v3,&linhas_assembly[j].q);break;}
                    case '-':
                    {linhas_assembly[j].lista = monta_binario('s',&linhas[i].v3,&linhas_assembly[j].q);break;}
                    case '*':
                    {linhas_assembly[j].lista = monta_binario('i',&linhas[i].v3,&linhas_assembly[j].q);break;}
                }
                j++;
                // movl %eax, v1
                linhas_assembly[j].lista = monta_binario('n',&linhas[i].v1,&linhas_assembly[j].q);
                j++;
                break;
            }
            case 'd': 
            {
                //cmpl $0, v1
                linhas_assembly[j].lista = monta_binario('d',&linhas[i].v1,&linhas_assembly[j].q);
                j++;
                //jne _ _ _ _ (nao podemos preencher agora, so no final)
                lista_desvios_assembly[k] = j; // valor de onde deveria estar o jne
                lista_desvios_linhas[k] = i; //valor de onde esta o desvio da linha
                k++;j++;
                break;
            }
            case 'r': 
            {
                linhas_assembly[j].lista = monta_retorno(&linhas_assembly[j].q);
                j++;
                break;
            }
        }
    }
    for (i=0;i<k;i++)
    {
        monta_desvio(linhas, lista_desvios_linhas[i], linhas_assembly,
                    lista_desvios_assembly[i]);
    }

    resposta = junta_binario(linhas_assembly, j, tamanho_final);
    return resposta;
}

void preenche_linhas(FILE *f, Linha *linhas, int *q_linhas, int *q_linhas_assembly, int *q_desvios)
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
            *q_linhas_assembly += 3;
            linhas[q].q_assembly = 3;

        }
        else if (c == 'i')
        {
            printf("Achei um if.\n");
            linhas[q].tipo = 'd';
            fscanf(f, "f %c%d %c", &linhas[q].v1.t, &linhas[q].v1.n,
                                    &linhas[q].op);
            *q_linhas_assembly += 2;
            linhas[q].q_assembly = 2;
            *q_desvios++;

        }
        else if (c == 'r')
        {
            printf("Achei um retorno.\n");
            linhas[q].tipo = 'r';
            fscanf(f, "et");
            *q_linhas_assembly += 1;    // considerando uma linha bem grande
            linhas[q].q_assembly = 1;

        }
        else
        {
            printf("achei alguma coisa estranha: %c", c);
        }
        while ((c != '\n') && (c != EOF)) c = fgetc(f);
        q++;
    }
    *q_linhas = q;
    return;
}

//funcp CompilaLinB (FILE *f)
int* CompilaLinB (FILE *f)
{
    Linha linhas[50];
    int q_linhas = 0;
    int q_linhas_assembly = 1;  //considerando o inicio e o fim uma linha bem grande :)
    int q_desvios = 0;
    int tamanho_final;
    int *lista_final;
    preenche_linhas(f,linhas, &q_linhas, &q_linhas_assembly, &q_desvios);
    lista_final = monta_assembly(linhas, q_linhas, q_linhas_assembly, &tamanho_final);
    dump(lista_final, tamanho_final);
    return lista_final;
}

/*
int main(int argc, char *argv[]) {
    
    FILE *f = fopen(argv[1], "r");
    if (f==NULL) {printf("Erro ao abrir arquivo. "); exit(1);}
    CompilaLinB(f);
    return 0;
}
*/

int main()
{
    char *nome_arquivo = "cod.linb";
    FILE *f = fopen(nome_arquivo, "r");
    if (f==NULL) {printf("Erro ao abrir o arquivo\n"); exit(1);}
    CompilaLinB(f);
    printf("\nFINALIZADO.\n\n");
    return 0;
}