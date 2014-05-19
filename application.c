/*
   BE Programmation système et concurrente
   IENAC 12 L

   Surveillance du fonctionnement d'une application

   Application de test

   Rev. : PRGCON-PTP-100-131220-131220-01
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "fonction.h"

#define DEBUG 1

typedef
  struct 
  {
    int     o;
    int     op1;
    int     op2;
  } Parametres;

typedef
  enum 
  { 
    OPERATION_0, 
    OPERATION_1, 
    OPERATION_2 
  } Operations;

int operation0( int a, int b )
{
  printf( "- CRASHTEST > %s realisee\n", __FUNCTION__ );
  return( a + b );
}

int operation1( int a, int b, int c )
{
  unsigned int   * p = NULL; /* Ne pas modifier ! */
  unsigned int     r;


  srand( time( (time_t *) p ) );
  if ((rand()%3)<2) /* L'application de test s'arretera sur une erreur avec une probabilite de 1/3 */
  {
    p = &r;
    puts( "- CRASHTEST > l'application ne posera pas de probleme..." );
  }
  else
  {
    puts( "! CRASHTEST > l'application va s'arreter sur une erreur..." );
  }
  printf( "- CRASHTEST > P = %x\n", *p );

  printf( "- CRASHTEST > %s realisee\n", __FUNCTION__ );
  return( a * b + c );
}

int operation2( int a )
{
  printf( "- CRASHTEST > %s realisee\n", __FUNCTION__ );
  return( a * a );
}


int main( int argc, char ** argv )
{
  Operations     point_reprise = OPERATION_0;  
  Parametres     parametres = { 1, 2, 3};    /* Valeurs modifiables a souhait */

  if(DEBUG==1) printf("DEBUG : test 6, argument=%s\n",argv[0]);

  point_reprise = initSurveillance(&parametres,sizeof(parametres));
  puts( "_ CRASHTEST > Demarrage de la surveillance" );

  switch( point_reprise )
  {
    case OPERATION_0 : printf( "_ CRASHTEST > Debut operation %d\n", point_reprise );
                       pointReprise(0,&parametres,sizeof(Parametres)); 
                       parametres.o = operation0( parametres.op1, parametres.op2 );
    case OPERATION_1 : printf( "_ CRASHTEST > Debut operation %d\n", point_reprise );
                       pointReprise(1,&parametres,sizeof(Parametres)); 
                       parametres.o = operation1( parametres.op1, parametres.op2, parametres.o );
    case OPERATION_2 : printf( "_ CRASHTEST > Debut operation %d\n", point_reprise );
                       pointReprise(2,&parametres,sizeof(Parametres)); 
                       parametres.o = operation2( parametres.o );
    default :          pointReprise(3,&parametres,sizeof(Parametres)); 
                       printf( "_ CRASHTEST > O = %d\n", parametres.o );
  }

  finAppli( ); 
  puts( "_ CRASHTEST > Fin de la surveillance" );

  return( EXIT_SUCCESS );
}


/*
   Fin code
   Application de test
*/



