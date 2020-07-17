#include <stdio.h>		
#include <stdlib.h>	//EXIT_SUCCESS e EXIT_FAILURE
#include <unistd.h>	//pipe(), fork(), getpid() e getppid()
#include <sys/types.h>	//pid_t
#include <sys/wait.h>	//wait()

#define TAM 10000
#define NPROCS 4

int main(int argc, char *argv[]){
	int cont, cont2;
	int primos[TAM];

	pid_t processo_pai=1;
	int posPai = 0;
	int status;
	int p[NPROCS-1][2];
	int pidPai=getpid(); 
	int zero = 0;
	printf("\nProcesso pai: %d | posição: 0\n", getpid());

	//calculando intervalo
	int inicioIntervalo, finalIntervalo, intervalo;
	intervalo = TAM/NPROCS;
	
	int possiveisPrimos[intervalo];

	// processo pai criando pipes
	for(cont=0;cont<NPROCS-1;cont++){
		
		if(processo_pai>0){
			pipe(p[cont]);
		}
	}

	for(cont=0;cont<NPROCS-1;cont++){
		// processo pai criando filhos
		if(processo_pai>0){
			processo_pai=fork();
		}
		if(processo_pai==0){
	 		int pidFilho=getpid();
			int pos=pidFilho-pidPai-1;	// pos indica qual o pipe este processo interagirá
			int valor = 1;	//valor da flag: enquanto diferente de 0, indica que o processo nao recebeu 0
			int k = 0;	//contador de possiveis primo

			// posicao processo filho
			printf("\nProcesso filho: %d | posição: %d\n", getpid(), pos);
			//inicio e fim do intervalo
			inicioIntervalo = (pos+1) * intervalo + 1;
			if(pos+1==NPROCS-1)
				finalIntervalo = TAM;
			else	
				finalIntervalo = inicioIntervalo + intervalo -1;

			//armezenar em um vetor auxiliar os valores do intervalo
			int possiveisPrimos[intervalo];
			int iterator=0;
			int iterator2=0;
			int iterator3=0;
			int primoTemporario[intervalo];

			// preenchendo vetor de possiveis primos
			for(k = inicioIntervalo; k <= finalIntervalo; k++){
				possiveisPrimos[iterator] = k;
				iterator++;
			}
			iterator=0;
			printf("\n\n");
			// preenchendo vetor de primos temporarios
			for(k = inicioIntervalo; k <= finalIntervalo; k++){
				primoTemporario[iterator] = k;
				iterator++;
			}
			iterator=0;
			close(p[pos][1]); //Fechando a posição de escrita, pois nesse momento o processo filho irá apenas ler os resultados
			
			// enquanto o valor recebido não for 0, ele pegará todos os seus valores do intervalo e dividirá por todos os valores recebidos
			while(valor!=0){
			   	read(p[pos][0], &valor, sizeof(int));
				primos[iterator] = valor;
				for(k = inicioIntervalo; k <= finalIntervalo; k++) {
					if(primos[iterator] == 0){
						break;
					}
					else if((possiveisPrimos[iterator2]%primos[iterator])==0){ //não é primo
						possiveisPrimos[iterator2] = 0;
					}else{
						possiveisPrimos[iterator2] = k;
					}
					iterator2++;
				}
				iterator++;
				iterator2 = 0;
			}	
			// limpando os '0s' do vetor possíveis primos e add em um vetor temporário
			// alem disso, enviar o seu valor para os outros processos
			int it = 0, primosEnviar[intervalo];	
			close(p[pos][0]);
			close(p[pos+1][0]);

			//limpando os 0 dos filhos que não é o último e enviando para o próximo
			if(pos+1<NPROCS-1){
				for(it = 0; it <intervalo; it++){
					if(possiveisPrimos[it] > 0){
						primosEnviar[iterator2] = possiveisPrimos[it];
						primos[iterator-1] = possiveisPrimos[it];	//iterator-1 porque vai vai colocar no lugar do 0 que ele recebeu
						// implementar envio dos numeros primos para os processos a seguir
						for(iterator3=pos; iterator3<NPROCS-1; iterator3++)
							write(p[iterator3][1], &primosEnviar[iterator2], sizeof(int));
						iterator++;
						iterator2++;
					}
				}
			}
			//limpando os 0 apenas do último filho
			if(pos+1==NPROCS-1){
				intervalo = finalIntervalo-inicioIntervalo+1;
				for(it = 0; it <intervalo; it++){
					if(possiveisPrimos[it] > 0){
						primos[iterator-1] = possiveisPrimos[it];	//iterator-1 porque vai vai colocar no lugar do 0 que ele recebeu
						iterator++;
						iterator2++;
					}
				}
			}			
			//se o processo atual não for o último processo, enviar 0 pro próximo processo.
			if(pos+1<NPROCS-1){
				//printf("Sou o processo %d e estou ENVIANDO O 0\n", pos);
				write(p[pos+1][1], &zero, sizeof(int));
				close(p[pos+1][1]);
			}

			if(pos+1==NPROCS-1){
				for(it = 0; it <iterator-1; it++){
					printf("Sou o processo %d e meu %dº numero primo é o %d\n", pos, it+1, primos[it]);
				}
			}
			_exit(EXIT_SUCCESS);
		}
	}
	
	//pai
	if(processo_pai>0){
		primos[0] = 2;
		int prox;	// indica qual a posicao do numero primo (no vetor) que sera dividido pelo valor testado
		int pos = 1; // posicao do vetor em que o próximo primo sera armazenado
		//inicio e fim
		inicioIntervalo = posPai * intervalo + 1;
		finalIntervalo = inicioIntervalo + intervalo-1;

		//Enviando o primeiro primo para os demais processos
		for(cont2=0;cont2<NPROCS; cont2++){
			close(p[cont2][0]); // Fechando para leitura, pai apenas envia
			write(p[cont2][1], &primos[0], sizeof(int));
		}
		printf("| %d", primos[0]);

		for(cont=3; cont <= finalIntervalo; cont++){
			int flag_primo = 1; //se 1, é primo; se 0, não é
			for(prox=0; prox<pos;prox++){
				if(cont%primos[prox]==0)
					flag_primo = 0;
			}
				
			if(flag_primo==1){  //Número é primo
				//armazena primo encontrado no vetor de primos
				primos[pos]=cont;
				//Enviando os números primos para os processos filhos
				for(cont2=0;cont2<NPROCS-1; cont2++)
					write(p[cont2][1], &primos[pos], sizeof(int));
				pos++;
				printf("| %d", cont);
			}
		}

		//escrevendo o valor 0 no processo filho da posicão 1
		write(p[0][1], &zero, sizeof(int));
		
		//fechando a escrita em todos os outros pipes
		for(cont2=0;cont2<NPROCS; cont2++)
			close(p[cont2][1]);
	}	
	wait(&status);
	return 0;
}
