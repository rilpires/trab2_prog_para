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

long long conta_primos( long long N , int num_threads = 1 ){
    short* crivo = new short[N+1]();
    long long i,j,k,p,primos_encontrados=0;
    int primos_conhecidos[5] = {2,3,5,7,11};
    double t , t0 = omp_get_wtime();
    omp_set_num_threads(num_threads);

    // Primeiramente, ja preenchemos os multiplos de primos menores (maior trabalho de preenchimento)
    t = omp_get_wtime();
    #pragma omp parallel
    #pragma omp master
    {
        for( i = 0 ; i < 5 && primos_conhecidos[i] < N+1; i++ ){
            p = primos_conhecidos[i];
            #pragma omp task firstprivate(j,p) shared(crivo)
            {
                for( j = 2*p ; j < N ; j += p )
                    crivo[j] = 1;
            }
        }
        #pragma omp taskwait
    }
    cout << "Duracao do preenchimento dos primeiros primos: " << omp_get_wtime() - t << "s" << endl;

    long long sqrtN = sqrt(N);
    long long soma_uns = 0;
    t = omp_get_wtime();

    #pragma omp parallel private(i,j)
    #pragma omp master
    {
        for( i=13 ; i<sqrtN ; i+=2 ){
            // Achou um primo, 'i'
            if(crivo[i]==0){
                // Preenche o crivo ate sqrtN
                for( j=2*i ; j<sqrtN ; j+=i )
                    crivo[j]=1;

                // O resto dos valores a serem preenchidos podem ficar para depois
                #pragma omp task firstprivate(j) firstprivate(i)
                for( ; j<N+1 ; j+=i )
                    crivo[j]=1;

            }
        }
    }
    cout << "Duracao do preenchimento do crivo até sqrt(N): " << omp_get_wtime() - t << "s" << endl;
    
    t = omp_get_wtime();
    #pragma omp parallel for reduction(+:soma_uns)
    for( i= 2 ; i<N+1 ; i++ )
        soma_uns += crivo[i];
    cout << "Duracao do reduction dos ultimos primos: " << omp_get_wtime() - t << "s" << endl;
    
    // Resultado final é N menos somatorio de todos os 1's. 
    primos_encontrados = N - (soma_uns+2);

    delete[] crivo;
    cout << "Duracao total: " << omp_get_wtime() - t0 << "s" << endl;
    return primos_encontrados;
}

int main( int argc , char** argv ){
    long long N = 100 , result;
    int num_threads = 1;
    
    if( argc>= 2 ) N = atoll( argv[1] );
    if( argc>= 3 ) num_threads = atoi( argv[2] );

    cout << "Numero de threads: " << num_threads << endl;

    double initial_t = omp_get_wtime();
    result = conta_primos( N , num_threads );
    cout << "Quantidade de primos menores ou iguais a " << N << ": " << result << endl;

}