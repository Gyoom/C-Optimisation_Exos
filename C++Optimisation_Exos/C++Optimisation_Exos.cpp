
#include <chrono>
#include <random>
#include <intrin.h>
#include <iostream>
#include <iterator>
#include <immintrin.h>
#include <omp.h>

using namespace std;

// Méthodes & Mesures ----------------------------------------------------------------------------------------------------------
// Filling array - chrono test
void chrono_test()
{
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();

    int t[1024];

    for (size_t i = 0; i < size(t); i++)
    {
        t[i] = i;
    }

    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
	// time taken in seconds
    cout << "total chrono: " << time_taken * 1000 << " ms" << endl;
	// 1 ms = 1/1000 s
}

// CPU Cycling - cycle count test
void asmtest_outer_only()
{
    unsigned long long start0 = __rdtsc();
    int repeats = 50000000;
    for (size_t i = 0; i < repeats; i++)
    {
        //func
        for (size_t j = 0; j < 50; j++)
        {
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
        }
    }
    __int64 endfinal = __rdtsc();

    cout << endfinal - start0 << endl;
}

void asmtest()
{
    double min = 99999999;
    double max = 0;
    __int64 start0 = __rdtsc();
    int repeats = 50000000;
    for (size_t i = 0; i < repeats; i++)
    {

        __int64 start = __rdtsc();
        for (size_t j = 0; j < 50; j++)
        {
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
            __nop();
        }
        __int64 end = __rdtsc();

        double innerlooptime = end - start;
        min = innerlooptime < min ? innerlooptime : min;
        max = innerlooptime > max ? innerlooptime : max;

    }
    __int64 endfinal = __rdtsc();
    //cout << "" << end - start << " " << (end - start) / 2800000000.0f <<  endl;
    double avg = (double)(endfinal - start0) / repeats;
    cout << "with_inner_timing " << repeats << " repeats" << endl;
    cout << "min: " << min << " ipc: " << 49 / min << endl;
    cout << "max: " << max << " ipc: " << 49 / max << endl;
    cout << "avg: " << avg << " ipc: " << 49 / avg << endl << endl;
}

const int array_size = 131072;
int my_array[array_size];
void fill_int_array()
{
    double min = 999999999;
    double max = 0;
    int outer_repeats = 1000;

    //int inner_repeats = 50000;

    cout << endl << "filling array test running..." << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();
    for (size_t j = 0; j < outer_repeats; j++)
    {
        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();

        //not unrolled
        //for (size_t i = 1; i < array_size; i++)
        //{
        //	my_array[i] = i;
        //	//check that the compiler doesn't just run this loop ahead of time
        //}

        //unrolled 2x
        for (size_t i = 1; i < array_size - 1; i += 8)
        {
            my_array[i] = i;
            my_array[i + 1] = i + 1;
            my_array[i + 2] = i + 2;
            my_array[i + 3] = i + 3;
            my_array[i + 4] = i + 4;
            my_array[i + 5] = i + 5;
            my_array[i + 6] = i + 6;
            my_array[i + 7] = i + 7;
            //check that the compiler doesn't just run this loop ahead of time
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;
    }

    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;

    int data_volume = array_size * sizeof(int);
    double bandwidth = data_volume / time_taken;
    double gigabyte = (uint64_t)2 << 29; //gibi
    //Gb: giga BIT. GB: giga BYTE / octet.
    //GIGA: 1000*1000*1000. GIBI: 1024*1024*1024.
    //gibi: 1<<30
    // 0001 << 1 == 0010
    // 0100 >> 1 == 0010
    //bit shifting: décaler de n bits
    //warning: bit shifts arithmétiques et des bit shifts logiques
    //chaque compilo décide si le bit shifting c'est l'un ou l'autre
    cout << "total chrono: " << time_taken * 1000 << "ms bandwidth: " << bandwidth / gigabyte << " gigabytes/s, or " << bandwidth << " bytes/s" << endl << endl;
    cout << gigabyte << endl;
}

// Reduction de Latence ----------------------------------------------------------------------------------------------------------

// Prediction Testing
void branch_pred_random()
{
    double min = 999999999;
    double max = 0;
    int outer_repeats = 30000;

    int a = 0;
    int inner_repeats = 50000;

    cout << endl << "random branch pred test running " << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();
    for (size_t j = 0; j < outer_repeats; j++)
    {
        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();
        for (size_t i = 0; i < inner_repeats; i++)
        {
            //#include <random>
            int r = rand(); //on run rand() avant la condition pour générer les mêmes instructions asm que l'autre test
            if (r % 1000 == 999)
            {
                a += 1;
            }
            else
            {
                a -= 1;
            }
            //a += (r % 1000 == 999 ? 1 : -1); //version sans if
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;
    }


    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << a << endl;
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;
    //cout << "rand() condition: " << time_taken *1000.f << "ms for " << inner_repeats << " iterations" << endl;
}
void branch_pred_facile()
{
    double min = 999999999;
    double max = 0;
    int outer_repeats = 30000;

    int a = 0;
    int inner_repeats = 50000;

    cout << endl << "easy branch pred test running " << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();
    for (size_t j = 0; j < outer_repeats; j++)
    {
        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();
        for (int i = 0; i < inner_repeats; i++)
        {
            int r = rand(); //on le run quand même pour pas fausser la comparaison, on veut que tester la branch pred elle-même
            //et on run rand() avant la condition pour générer les mêmes instructions asm que l'autre test
            if (i % 1000 == 999)
            {
                a += 1;
            }
            else
            {
                a -= 1;
            }
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;
    }


    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << a << endl;
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;
    //cout << "rand() condition: " << time_taken *1000.f << "ms for " << inner_repeats << " iterations" << endl;
}

// Cohérence de Cache
void cache_cohesion_test()
{
    double min = 999999999;
    double max = 0;
    const int outer_repeats = 500;

    double outer_times[outer_repeats];

    //int count = (2 << 29) /64; //max signed int
    long int count = (1024 * 1024 * 1024) / 32;

    struct s
    {
        int a;
        int t[15]; //comment this out/in to test cache coherency
    };

    s* s1 = (s*)malloc(count * sizeof(s)); //peut techniquement renvoyer null en cas d'échec d'alloc

    //sus, à check
    for (long int i = 0; i < count; i++)
    {
        s1[i] = s();
    }

    cout << endl << "cache_coherency_test running " << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();

    for (size_t j = 0; j < outer_repeats; j++)
    {

        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();
        for (int i = 0; i < count * ((double)j / outer_repeats); i++)
        {

            s1[i].a = i; //peut techniquement être null en cas d'échec d'alloc
            //cout << "";
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;

        outer_times[j] = inner_time_taken;
    }

    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;

    for (size_t i = 1; i < outer_repeats; i++)
    {
        cout << outer_times[i] << endl;
    }

    free(s1);
}

// Alignement des données
void alignment_test()
{
    double min = 999999999;
    double max = 0;
    int outer_repeats = 500;
    int inner_repeats = 5;

    struct s
    {
        alignas(64) char t[64];
        int a = 7;
    };
    s* s1 = (s*)malloc(8192 * sizeof(s));

    cout << endl << "alignment_test running " << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();

    for (size_t j = 0; j < outer_repeats; j++) {
        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();
        for (int i = 0; i < inner_repeats; i++) {
            for (size_t si = 0; si < 8192; si++) {
                s1[si] = s();
                for (size_t k = 0; k < 64; k++)
                {
                    s1[si].t[k] = k;
                }
            }
            cout << "";
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;
    }
    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;
    free(s1);
}

// Page Faults 
void page_faults_test()
{
    double min = 999999999;
    double max = 0;
    const int outer_repeats = 1000;

    double outer_times[outer_repeats];

    unsigned long count = (1024 * 1024 * 1024);

    char* mem = (char*)malloc(count);
    if (mem == nullptr)
    {
        cout << "malloc fail" << endl;
        return;
    }

    cout << "with warmup" << endl;
    for (unsigned long i = 0; i < count / 4096; i++)
    {
        mem[i * 4096] = i;
    }

    cout << endl << "page_faults_test running " << outer_repeats << " times" << endl;
    chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();
    for (unsigned long j = 0; j < outer_repeats; j++)
    {
        chrono::high_resolution_clock::time_point inner_start_chrono = chrono::high_resolution_clock::now();
        for (unsigned long i = 0; i < count / 4096; i++)
        {
            mem[i * 4096] = 2;
        }
        double inner_time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - inner_start_chrono).count();
        min = inner_time_taken < min ? inner_time_taken : min;
        max = inner_time_taken > max ? inner_time_taken : max;

        //outer_times[j] = inner_time_taken;
    }

    double time_taken = chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - start_chrono).count();
    double avg = (time_taken) / (outer_repeats);
    cout << "min: " << min * 1000.f << "ms" << endl;
    cout << "max: " << max * 1000.f << "ms" << endl;
    cout << "avg: " << avg * 1000.f << "ms" << endl << endl;

    /*for (size_t i = 1; i < outer_repeats; i++)
    {
        cout << outer_times[i] << endl;
    }*/

    free(mem);
}

// Other ----------------------------------------------------------------------------------------------------------
// simd
const int taille = 10000000;
float t[taille];

void simd_float() {
    for (size_t i = 0; i < taille; i++)
    {
		t[i] = rand() % 10;
    }
	float total = 0;    

    for (size_t i = 0; i < taille; i++)
    {
        total += t[i];
	}
    std::cout << total << std::endl;
}

__m128 t2[taille / 4];

void simd_simd() 
{
    for (size_t i = 0; i < taille/4; i++)
    {
		t2[i] = _mm_set_ps(rand() % 10, rand() % 10, rand() % 10, rand() % 10);
    }
	__m128 total2 = _mm_set_ps(0, 0, 0, 0);

    for (size_t i = 0; i < taille/4; i++)
    {
		total2 = _mm_add_ps(total2, t2[i]);
    }
	float z = total2.m128_f32[0] + total2.m128_f32[1] + total2.m128_f32[2] + total2.m128_f32[3];
    std::cout << z << std::endl;
}

// openmp
void openm()
{
    // Define the size of the array
    const int SIZE = 100000000;

    // Initialize the array
    int* array = new int[SIZE];
    for (int i = 0; i < SIZE; ++i)
    {
        array[i] = i;
    }

    // Define a variable for the sum
    long long sum = 0;

    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Use OpenMP to calculate the sum in parallel
#pragma omp parallel for reduction(+:sum) schedule(static, 1000)
    for (int i = 0; i < SIZE; ++i)
    {
        sum += array[i];
    }

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();

    // Print the sum
    std::cout << "Sum: " << sum << std::endl;

    // Calculate and print the time taken
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time elapsed: " << elapsed.count() << " seconds" << std::endl;


    // Clean up the array
    delete[] array;
}


// Main ----------------------------------------------------------------------------------------------------------

int main()
{
    // CPU : Intel(R) Core(TM) i9-14900KF, 3200 MHz, 24 cœur(s), 32 processeur(s) logique(s)
    // Manual : 
    //      https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    // cache size : 
    //      Task manager -> Performance -> CPU
    //      https://www.techpowerup.com/cpu-specs/core-i9-14900k.c3269
    //      https://www.intel.com/content/www/us/en/products/sku/236787/intel-core-i9-processor-14900kf-36m-cache-up-to-6-00-ghz/specifications.html?wapkw=intel%20core%20i9-14900k
	// cache line size : 64 bytes
	//      https://valid.x86.fr/7s8pff 

	//chrono_test();
	//asmtest_outer_only();

	// Méthodes & Mesures ---
    ///asmtest();
    //fill_int_array();

	// Reduction de Latence ---
	// Prediction Testing
	//branch_pred_random();
	//branch_pred_facile();

	// Cohérence de Cache
    //cache_cohesion_test();

	// Alignement des données
	//alignment_test();

    // SIMD
    //simd_float();
    //simd_simd();

	// OpenMP
	//openm();

	return 0;


}