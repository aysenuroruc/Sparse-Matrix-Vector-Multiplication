#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h> 
#define WIDTH=100000000

//PARALEL OLARAK SPARSE MATRİS ÜRETME 

_global_void csrmul_kernel(int *Rp, int *C, float *D, int num_rows, float *x, float *result)
{
	
int row = blockIdx.x*blockDim.x + threadIdx.x;
if( row<num_rows )
{

int row_begin = Rp[row];
int row_end = Rp[row+1];

result[row] = multiply_row(row_end - row_begin, C+row_begin, D+row_begin, x);
 }
}

//SERİ OLARAK SPARCE MATRIS OLUSTURMA
void csrmul_serial(int *Rp, int *C, float *D, int num_rows,float *x, float *y)
{
for(int row=0; row<num_rows; ++row)
{
int row_begin = Rp[row];
int row_end = Rp[row+1];
y[row] = multiply_row(row_end - row_begin, C+row_begin,D+row_begin, x);
 }
}
//SERİ ÇARPMA FONKSİYONU
float multiply_row(int WIDTH,int *result,float *vector,float *sonuc) 
{
float sum = 0;
for(int column=0; column < WIDTH; ++column)
sum += D[column] * x[C[column]];
return sum;
}

//PARALEL ÇARPMA FONKSİYONU

__global__ void MatrixMulSh( float *Md , float *Nd , float *Pd , const int WIDTH=100000000 )
{
          __shared__ float Mds [10000][1] ;

           __shared__ float Nds [1][10000] ;

         // Threadlerin id lerinin hesaplanması
          unsigned int col = 10000*blockIdx.x + threadIdx.x ;
          unsigned int row = 10000*blockIdx.y + threadIdx.y ;

        for (int m = 0 ; m<WIDTH/10000 ; m++ ) 
       {
            Mds[threadIdx.y][threadIdx.x] =  Md[row*WIDTH + (m*10000 + threadIdx.x)]  ;
            Nds[threadIdx.y][threadIdx.x] =  Nd[ ( m*10000 + threadIdx.y) * WIDTH + col] ;
         __syncthreads() ; // threadlerin senkronizasyonu

           for ( int k = 0; k<10000 ; k++ )
          Pd[row*WIDTH + col]+= Mds[threadIdx.x][k] * Nds[k][threadIdx.y] ;
         __syncthreads() ; 
  }
}

int main()
{
	//CDR Matrislerinin dosyadan okunması
	
    FILE *myFileC,*myFileD,*myFileRp;
    myFileC = fopen("C.txt", "r");
    myFileD =fopen("D.txt","rt");
    myFileRp=fopen("Rp.txt","r");
       
    int *numberArrayC,*numberArrayRp;
    float *numberArrayD;

	//Bellekte büyük diziler için yer ayrılması
	
    numberArrayC=(int *) malloc(99911*sizeof(int));
    numberArrayD=(float *) malloc(99911*sizeof(float));
    numberArrayRp=(int *) malloc(10001*sizeof(int));
    vector=(float *) malloc(10000*sizeof(float));
	
//Dosyadaki değerleri diziye atama işlemi
    while (!feof (myFileC))
{
     for (int i = 0; i < 99911;  i++)
    {	
        fscanf(myFileC, "%d", &numberArrayC[i]);
       
    }
}

  while (!feof (myFileD)){

    for (int i = 0; i < 99911;  i++){
	
    fscanf(myFileD, "%f", &numberArrayD[i]);
  }
}  

  while (!feof (myFileRp)){

 for (int i = 0; i < 10001;  i++)
    {	
        fscanf(myFileRp, "%d", &numberArrayRp[i]);
        
    }
}
//testi yapıldı ekranda gösterildi.
     for (i = 0; i < 99911; i++)
   {
        printf("%d\n", numberArrayC[99910]);
        printf("%f", numberArrayD[99910]);
        printf("%d\n", numberArrayRp[10000]);
        printf("%d\n", i);
    }


//VECTÖR DİZİSİNE 1.0 ATAMA 
    
for(int i=0; i<10000; i++ )
{
vector[i]=1.0;
}


//gridlere bölme

  dim3 dimGrid ( WIDTH/TILE_WIDTH , WIDTH/TILE_WIDTH ,1 ) ;

  dim3 dimBlock( TILE_WIDTH, TILE_WIDTH, 1 ) ;
  
 //kopyalama işlemi yapılır
 
 cudaMemcpy ( result,Cresult , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;

 cudaMemcpy ( vector , Cvector , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;
   
 cudaMemcpy(sonuc , Csonuc , WIDTH*WIDTH*sizeof(int) ,cudaMemcpyDeviceToHost) ;

   
  cudaMalloc((void **) &result_array_d , WIDTH*WIDTH*sizeof (int) ) ;

  cudaMalloc((void **) &M_result_array_d , WIDTH*WIDTH*sizeof (int) ) ;

   //Paralel çarpma fonksiyonu cağrılır
  #if 1

  MatrixMulSh<<<dimGrid,dimBlock>>> ( result , vector ,sonuc , WIDTH) ;

#endif

  #if 2
//seri hesaplama için sparce matris uretme fonksıyonunun cagrılması
csrmul_serial(int *Rp, int *C, float *D, int num_rows,float *x, float *y);

//seri çarpma fonsıyonu çağrılır.
multiply_row(int WIDTH,int *result,float *vector,float *sonuc);
 

#endif

// sonuç matrisini ekrana yazdırma

printf("......İlk 10 Sonuc......");
 
 for ( i = 0 ; i<10 ; i++ )
  {
      for ( j = 0 ; j < 10 ; j++ )
     {
        printf ("%f   ",sonuc[i][j] ) ;
     }
 printf ("\n") ;
}
 

}
