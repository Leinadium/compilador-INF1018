# Compilador de "Linguagem Basica"

## Implementacao em Assembly

Foi atribuida cada linha em linb certas instrucoes em assembly.

p1 = %edi / p2 = %esi / v1..v4 = -16(%rbp) .. -4(%rbp)

Atribuicao: var1 = var2 opr var 3
	mov  var2, %eax
	opr  var3, %eax
	mov  %eax, var1

Desvio: if var1 n
	cmp  $0, var1
	je  _ _ _ _

Retorno:
	mov  -16(%rbp) %eax
	addq $16, %rsp
	leave
	ret

## Procedimentos

Foi utilizado o comando objdump para se entender o binario de cada instrucao em assembly.

Alem disso, parte do codigo fornecido no site foi utilizado para a leitura do arquivo.

Porem, todo o resto do programa foi produzido por autoria propria

## Detalhes

Foram utilizados 3 tipos diferentes de estruturas, 10 funcoes auxiliares, totalizando 668 linhas de codigo.

Parte do historico do progresso foi perdido pela troca de sistema operacional, pois estava utilizando o windows,
mas depois decidi trocar para utilizar somente o ubuntu a partir de um dual boot. Anteriormente estava conectado
em um laptop antigo rodando linux a partir de uma conexao ssh, editando um programa em um disco rigido externo.
