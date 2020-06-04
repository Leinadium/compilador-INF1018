/* DANIEL_SCHREIBER GUIMARAES 1910462*/

/*
    A funcao CompilaLinb eh divida em varias etapas
    
    Primeiro, examina o arquivo, e eh criada uma lista com varias estruturas Linha
    Essa estrutura possui ate 3 Var (variaveis) -> possui um t (tipo, p ou v ou $) e um n (1-4 ou 1-2 ou n qualquer)
                                o tipo da linha
                                a operacao
                                quantidade de comandos de assembly

    Depois, ele começa a montar o assembly.
    Para isso, criei outra estrutura, Assembly,
    que possui uma lista de inteiros, que na verdade sao os bytes
                e a quantidade de bytes

    No final, chamo uma funcao que junta todos os Assemblys em um so,
    criando uma lista enorme de bytes

    E ainda faco um dump, mostrando cada byte, para caso queira verificar

    Cada funcao possui a sua explicao, e um pouco de documentacao
    Para melhor entender o programa, eh recomendado vendo as ultimas funcoes e ir subindo

    OBS: "Linha" -> refere a linha em .linb
         "Assembly" -> se refere a uma instrucao em assembly, como movq v1 v2
         "Byte/Binario" -> Byte/Binario.
*/

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


void verificar_erro(void *lista)
{
    /* funcao para ver se a lista alocada dinamicamente ficou vazia */
    if (lista == NULL)
    {
        printf("\nFalta de memoria! Nao consegui alocar memoria para uma lista.\n");
        exit(1);
    }
    return;
}

void dump_lista(unsigned char* l, int tamanho)
{
    /* faz o dump de cada byte */
    int i;
    printf(" DUMP(%d): ", tamanho);
    for (i=0;i<tamanho;printf("|%x",l[i++]));
    printf("|\n");
    return;
}

void libera_assembly(Assembly *lista, int q)
{
    /* libera a lista de assembly, e cada lista dentro dela */
    int i;
    for (i=0;i<q;i++) free(lista[i].lista);
    free(lista);
    return;
}

unsigned char* junta_binario(Assembly *lista_binario, int q, int *tamanho_final)
{
    /*
        Essa funcao recebe uma lista de Assemblys, e vai juntar em uma unica lista
        Primeiro descobre a quantidade total, e depois vai dando "append"
    */
    unsigned char* lista_final;
    int tamanho_total = 0;
    int i, j, k = 0;
    for (i=0; i<q;i++) tamanho_total += lista_binario[i].q;

    lista_final = (unsigned char*)malloc(sizeof(unsigned char)*tamanho_total);
    verificar_erro(lista_final);

    for (i=0; i<q; i++)
    {
        for (j=0;j<lista_binario[i].q;j++)
        {
            lista_final[k++] = lista_binario[i].lista[j] & 0xff; 
            // o & 0xff garante que so um byte entrou
            // vai que...
        }
    }
    *tamanho_final = tamanho_total;
    return lista_final;
}

void monta_desvio(Linha *linhas, int indice_linha, Assembly *lista_assembly, int indice_assembly)
{
    /*
        Essa funcao monta os desvios (if var n).
        Linha *linhas -> Lista inteira de Linhas
        int indice_linha -> o indice do desvio nas Linhas
        Assembly *lista_assembly -> Lista inteira em Assembly
        int indice_assembly -> indice do desvio na lista de Assembly
    */

    // eu tenho a minha posicao na lista binario.
    // preciso calcular o offset ate o indice da linha q quero
    int destino_linha = (int) linhas[indice_linha].v2.n - 1; // meu array comeca em 0, mas a linha comeca em 1
    int offset_linha = destino_linha - indice_linha;
    int pulo_bytes = 0; // esse sera colocado no binario
    int copia_indice_assembly = indice_assembly; // copia pois sera modificado
    int tamanho_linha_assembly; // tamanho em bytes da instrucao de desvio
    int i;
    
    if (offset_linha == 0)
    {
        printf("LOOP INFINITO DETECTADO.\n");
        exit(1);
    }

    else if (offset_linha > 0) // vamos andar pra frente
    {
        /* lembrando que o rif ja esta apontando para a proxima instrucao*/
        copia_indice_assembly += 1; // comeca na proxima instrucao em assembly
        while (offset_linha > 1)
        {
            indice_linha++; // ando uma LINHA (tamanho em assembly / bytes eh variavel)
            tamanho_linha_assembly = linhas[indice_linha].q_assembly; // ve quantas instrucoes em assembly vou pular
            for (i=0;i<tamanho_linha_assembly;i++) // para cada instrucao, vamos somar os bytes dela
            {
                pulo_bytes += lista_assembly[copia_indice_assembly++].q;
            }
            offset_linha = destino_linha - indice_linha; //atualiza o offset
        }
    }
    else // offset negativo  
    {
        // sabemos que a linha que estamos contem 3 ou 4 bytes com certeza, + 2 ou 6 dependendo do tamanho do pulo
        pulo_bytes -= lista_assembly[copia_indice_assembly - 1].q; //ja conta os bytes da instrucao em assembly anterior (cmpl $0 v1)
        copia_indice_assembly-= 2; //ja comecando na linha debaixo
        // mas teoricamente, o pulo sera a quantidade de bytes que temos nas linha debaixo
        // pra isso, vamos contar a os bytes da nossa linha, e descer
        do
        {
            /* basicamente a mesma coisa que antes, porem descendo. E comecamos fazendo e depois comparando para ver se anda mais */
            indice_linha--;
            tamanho_linha_assembly = linhas[indice_linha].q_assembly;
            for (i=0;i<tamanho_linha_assembly;i++)
            {
                pulo_bytes -= lista_assembly[copia_indice_assembly--].q;
            }
        } while (indice_linha != destino_linha);
    }
    if (offset_linha <0) pulo_bytes -= 2; // contando o seu proprio tamanho da instrucao "b8 XX"
    if ((pulo_bytes < 127) && (pulo_bytes > -128)) // se o pulo for pequeno ^^
        {
            /* igual ao restante */
            char *b = (char *)malloc(sizeof(char)*2);
            lista_assembly[indice_assembly].q = 2;
            b[0] = 0x75;
            b[1] = (char) pulo_bytes;
            lista_assembly[indice_assembly].lista = b;
        }
        else
        {
            if (offset_linha >0) pulo_bytes -= 4; // atualiza o tamanho do pulo "b8 XX XX XX XX"
            char *b = (char *)malloc(sizeof(char)*6);
            lista_assembly[indice_assembly].q = 6;
            b[0] = 0x0f;
            b[1] = 0x85;
            /* int armazenado em little endian */
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
    /* todo inicio em assembly eh o mesmo, por isso considerei como uma unica linha em assembly bem grande */
    /* em vez de tres. Nao faz muita diferenca */
    /*
        pushq   %rbp
        movq    %rsp, %rbp
        subq    $16, %rsp
    */
    char *b = (char*)malloc(sizeof(char)*8);
    verificar_erro(b);

    // verificado com objdump 
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
    /* assim como o inicio, todo retorno eh igual */
    /*
        movl    -16(%rbp), %eax
        add     $16, %rsp
        leave
        ret
    */
   *q = 9;
    char *b = (char*)malloc(sizeof(char)*9);
    verificar_erro(b);

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
    /* essa eh a maior funcao. Ela vai receber basicamente toda uma instrucao em assembly, e vai retornar uma lista de bytes referente a ela */
    /* TODOS os bytes foram examinados a partir de objdump
       Por isso nao vou colocar comentarios em cada linha. Afinal foi a partir de observacoes.
       Houve mais ou menos 15 casos diferentes
    */
    /*
        REFERENCIA: 
        m = mov v eax
        n = mov eax v
        a = add v eax
        s = sub v eax
        i = imu v eax
        d = cmp $0, v

        p1,p2 = %edi, %esi
        v1..4 = -16(%rpb)..-4(%rbp)

        OBS: A explicacao do uso de %eax esta mais embaixo
    */

   /* preferi usar if/else em vez de switch/case, pra nao ter que usar break... */
    char *b;
    if (tipo == 'm')  //mov var %eax --> primeiro comando da atribuicao
    {
        if (var->t == '$')
        {
            *q = 5;
            b = (char*)malloc(sizeof(char)*5);
            verificar_erro(b);

            b[0] = 0xb8;            // salva o n em little endian
            b[1] = (char) var->n;
            b[2] = (char) var->n >> 8;
            b[3] = (char) var->n >> 16;
            b[4] = (char) var->n >> 24;
        }
        else if (var->t == 'p')
        {
            *q = 2;
            b = (char*)malloc(sizeof(char)*2);
            verificar_erro(b);

            b[0] = 0x89;
            b[1] = (var->n == 1) ? 0xf8 : 0xf0; // depende do parametro
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            verificar_erro(b);

            b[0] = 0x8b;
            b[1] = 0x45;
            b[2] = 0xf0 + 4 * ((char) var->n - 1); // depende do numero da variavel
        }
        
    }
    else if (tipo == 'n') // mov %eax var --> ultima intrucao da atribuicao
                          // bem parecido com o mov v1 %eax
    {
        if (var->t == 'p')
        {
            *q = 2;
            b = (char*)malloc(sizeof(char)*2);
            verificar_erro(b);

            b[0] = 0x89;
            b[1] = (var->n == 1) ? 0xc7 : 0xc6;
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            verificar_erro(b);

            b[0] = 0x89;
            b[1] = 0x45;
            b[2] = 0xf0 + 4*(((char)var->n)-1);
        }
        
    }
    else if ((tipo == 'a') || (tipo == 's')) // add var %eax OU sub var %eax
    {
        if (var->t == '$')
        {
            if ((var->n < 16)&&(var->n > -15))
            {
                *q = 3;
                b = (char*)malloc(sizeof(char)*3);
                verificar_erro(b);

                b[0] = 0x83;
                b[1] = (tipo == 'a') ? 0xc0 : 0xe8; // a pequena diferenca entre add e sub no binario
                b[2] = (char) var->n;
            }
            else
            {
                *q = 5;
                b = (char*)malloc(sizeof(char)*5);
                verificar_erro(b);

                b[0] = (tipo == 'a') ? 0x05 : 0x2d; // mesma pequena diferenca observada
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
            verificar_erro(b);

            b[0] = (tipo == 'a') ? 0x01: 0x29;
            b[1] = (var->n == 1) ? 0xf8 : 0xf0;
        }
        else
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            verificar_erro(b);

            b[0] = (tipo == 'a') ? 0x03 : 0x2b;
            b[1] = 0x45;
            b[2] = 0xf0 + 4 * ((char) var->n - 1);
        }
    }
    else if (tipo == 'i')   // imul var %eax
    {
        if (var->t == '$')
        {
            if ((var->n < 16)&&(var->n > -15))
            {
                *q = 3;
                b = (char*)malloc(sizeof(char)*3);
                verificar_erro(b);

                b[0] = 0x6b;
                b[1] = 0xc0;
                b[2] = (char) var->n;
            }
            else
            {
                *q = 6;
                b = (char*)malloc(sizeof(char)*6);
                verificar_erro(b);

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
            verificar_erro(b);

            b[0] = 0x0f;
            b[1] = 0xaf;
            b[2] = (var->n == 1) ? 0xc7 : 0xc6;
        }
        else
        {
            *q = 4;
            b = (char*)malloc(sizeof(char)*4);
            verificar_erro(b);

            b[0] = 0x0f;
            b[1] = 0xaf;
            b[2] = 0x45;
            b[3] = 0xf0 + 4 * ((char) var->n - 1);
        }        
    }
    else /* if (tipo == 'd') --> cmpl $0, var */
    { 
        if (var->t == 'p')
        {
            *q = 3;
            b = (char*)malloc(sizeof(char)*3);
            verificar_erro(b);

            b[0] = 0x83;
            b[1] = (var->n == 1) ? 0xff : 0xfe;
            b[2] = 0x00;
        }
        else
        {
            *q = 4;
            b = (char*)malloc(sizeof(char)*4);
            verificar_erro(b);

            b[0] = 0x83;
            b[1] = 0x7d;
            b[2] = 0xf0 + 4 * ((char) var->n - 1);
            b[3] = 0x00;
        }
        
    }
    return b;
}

unsigned char* monta_assembly(Linha *linhas, int q_linhas, int q_linhas_assembly, int *tamanho_final)
{
    /*
        Essa funcao recebe a lista de Linhas, vai criar uma lista de Assembly,
        e depois juntar numa so lista a partir de juntar_binario()
        Ela le as Linhas, e vai ver os comandos em assembly para cada uma
    */

    int i, j = 1, k = 0; // variaveis contadoras
     // i -> looping nas linhas do arquivo 
     // j -> indice das linhas_assembly (comecando em 1, pois 0 sera o inicio)
     // k -> indice de lista_desvios_linhas e tbm lista_desvios_assembly
    unsigned char* resposta; // resposta final
    int q_desvios; // conta quantos desvios tem para alocar as listas de desvios paralelas
    int *lista_desvios_linhas; // duas listas para guardar informacoes para depois criar os desvios separadamente
    int *lista_desvios_assembly; // sao paralelas.

    Assembly *linhas_assembly = (Assembly *)malloc(sizeof(Assembly)*q_linhas_assembly);
    if (linhas_assembly == NULL)
    {
        printf("Nao foi possivel criar uma lista para o codigo assembly.\n");
        exit(1);
    }
    // cada linhas possui alguns comandos
    // esse comando tem uma lista alocada dinamicamente, e a qtd de bytes nesse comando

    //inicio
    printf("\tAdicionando inicio.\n");
    linhas_assembly[0].lista = monta_inicio(&linhas_assembly[0].q);

    printf("\tContando desvios.\n");
    for (i=0; i<q_linhas;i++) {if (linhas[i].tipo == 'd') q_desvios++;}

    // criando as listas paralelas de desvio
    lista_desvios_linhas = (int *)malloc(sizeof(int)*q_desvios);
    verificar_erro(lista_desvios_linhas);

    lista_desvios_assembly = (int *)malloc(sizeof(int)*q_desvios);
    verificar_erro(lista_desvios_assembly);

    printf("\tExaminando linhas:\n");
    for (i=0; i<q_linhas; i++)
    {
        switch (linhas[i].tipo)
        {
            case 'a': // ATRIBUICAO
            {
                /* 3 INSTRUCOES */
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
    printf("\t\tExaminado todas as linhas\n");
    printf("\t\tCriando os desvios.\n");
    for (i=0;i<k;i++)
    {
        /* depois de finalizar os binarios, agora sim podemos preencher os desvios */
        monta_desvio(linhas, lista_desvios_linhas[i], linhas_assembly,
                    lista_desvios_assembly[i]);
    }

    printf("\t\tLiberando listas auxiliares.\n");
    free(lista_desvios_assembly);
    free(lista_desvios_linhas);

    printf("\t\tCriando binario e limpando lista auxiliar final.\n");
    resposta = junta_binario(linhas_assembly, j, tamanho_final);
    libera_assembly(linhas_assembly, q_linhas_assembly);

    return resposta;
}

void preenche_linhas(FILE *f, Linha *linhas, int *q_linhas, int *q_linhas_assembly, int *q_desvios)
{
    /* Le as linhas, cria uma estrutura Linha e salva numa lista */
    /* Bem semelhante ao exemplo dado no site */
    /*  note-se que cada linha vai ser atribuido uma quantidade de linhas em assembly diferente
        pois, uma atribuicao precisa de 3 instrucoes, um desvio de 2, e um retorno de 4
        as instrucoes em si sao detalhadas na funcao monta_assembly
    */
    // q_linhas_assembly e q_desvios serao usadas na funcao monta_assembly

    char c;
    int q = 0;

    printf("Lendo linhas:\n");
    while ((c = fgetc(f)) != EOF )          //le ate chegar ao fim do arquivo
    {
        /* nova linha! */
        if ((c == 'v') || (c == 'p'))       // ATRIBUICAO
        {
            printf("\tAchei uma atribuicao.\n");
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
            printf("\tAchei um if\n");
            linhas[q].tipo = 'd';
            fscanf(f, "f %c%d %d", &linhas[q].v1.t, &linhas[q].v1.n,
                                    &linhas[q].v2.n);

            *q_linhas_assembly += 2;
            linhas[q].q_assembly = 2;
            *q_desvios++;
            // printf("variavel %c%d, indo para %d\n",linhas[q].v1.t,linhas[q].v1.n,linhas[q].op);

        }
        else if (c == 'r')
        {
            printf("\tAchei um retorno.\n");
            linhas[q].tipo = 'r';
            fscanf(f, "et");
            *q_linhas_assembly += 1;    // considerando uma linha bem grande
            linhas[q].q_assembly = 1;
        }
        else
        {
            printf("\tachei alguma coisa estranha: %c", c);
        }
        while ((c != '\n') && (c != EOF)) c = fgetc(f);
        q++;
    }
    *q_linhas = q;
    return;
}

//funcp CompilaLinB (FILE *f)
funcp CompilaLinB (FILE *f)
{
    /* Funcao principal. Recebe arquivo, retorna o binario */
    Linha linhas[50]; // Vai receber as Linha lidas do arquivo
    int q_linhas = 0; // quantidade de linhas
    int q_linhas_assembly = 1;  // quantidades de instrucoes em assembly. Comeca em 1 pois [0] possuira as instrucoes iniciais todas juntas
    int q_desvios = 0; // quantidades de desvios (if var n). usada na monta_assembly
    int tamanho_final; // tamanho da lista de bytes
    
    unsigned char *lista_final; // lista de bytes

    printf("Inicio da compilacao\n");
    preenche_linhas(f,linhas, &q_linhas, &q_linhas_assembly, &q_desvios); // preenche linhas[50]
    printf("Montando assembly:\n");
    lista_final = monta_assembly(linhas, q_linhas, q_linhas_assembly, &tamanho_final); // preeche a lista final

    printf("\nFinalizado!");
    dump_lista(lista_final, tamanho_final); // mostra a lista final. Pode ser tirada sem alteracao no programa
    return (funcp) lista_final;
}

void LiberaFuncao(void *p)
{
    free(p); // como foi alocada dinamicamente, e todas as outras listas ja foram liberadas, so esta precisa ser liberada.
    return;
}

/* a funcao main abaixo era para rodar os testes atraves do visual studio code, so usando o dump */
/*
int main()
{
    char *nome_arquivo = "cod.linb";
    FILE *f = fopen(nome_arquivo, "r");
    if (f==NULL) {printf("Erro ao abrir o arquivo\n"); exit(1);}
    CompilaLinB(f);
    printf("\nFINALIZADO.\n\n");
    return 0;
}
*/
