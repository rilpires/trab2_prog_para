#include <iostream>
#include <omp.h>
#include <cstdlib>
#include <vector>
#include <math.h>
using namespace std;

/**
 * Compilar com:        g++ -fopenpm crivo.cpp -o crivo -O3
 * Executar com:        ./crivo [TAM_CRIVO] [NUM_THREADS]
**/

int conta_primos( int N , int num_threads = 1 ){
    short* crivo = new short[N+1]();
    int i,j,k,p,primos_encontrados=0;
    int primos_conhecidos[5] = {2,3,5,7,11};
    omp_set_num_threads(num_threads);

    // Primeiramente, ja preenchemos os multiplos de primos menores (maior trabalho de preenchimento)
    #pragma omp parallel
    #pragma omp single
    {
        for( i = 0 ; i < 5 && primos_conhecidos[i] < N+1; i++ ){
            p = primos_conhecidos[i];
            #pragma omp task private(j) shared(crivo)
            {
                for( j = 2*p ; j < N ; j += p )
                    crivo[j] = 1;
            }
        }
        #pragma omp taskwait
    }

    int sqrtN = sqrt(N);
    int sqrtsqrtN = sqrt(sqrtN);
    int soma_uns = 0;
    #pragma omp parallel shared(soma_uns) private(j)
    {
        #pragma omp single
        {
            for( i=13 ; i<sqrtN ; i+=2 ){
                // Achou um primo, 'i'
                if(crivo[i]==0){
                    // Preenche o crivo ate sqrtN
                    for( j=2*i ; j<sqrtN ; j+=i )
                        crivo[j]=1;
                    // O resto dos valores a serem preenchidos
                    #pragma omp task
                    {
                        for( ; j<N+1 ; j+=i )
                            crivo[j]=1;
                    }
                }
            }
        }
        
        #pragma omp taskwait
        #pragma omp for reduction(+:soma_uns)
        for( i= 2 ; i<N+1 ; i++ )
            soma_uns += crivo[i];
        
    }
    // Resultado final é N menos somatorio de todos os 1's. 
    primos_encontrados = N - (soma_uns+2);

    delete[] crivo;
    return primos_encontrados;
}

int main( int argc , char** argv ){
    int N=100 , num_threads=1;
    
    if( argc>= 2 ) N = atoi( argv[1] );
    if( argc>= 3 ) num_threads = atoi( argv[2] );

    double initial_t = omp_get_wtime();
    cout << "Quantidade de primos menores ou iguais a " << N << ": " << conta_primos( N , num_threads ) << endl;
    cout << "Duracao: " << omp_get_wtime() - initial_t << "s " << endl;

}