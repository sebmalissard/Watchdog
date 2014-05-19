#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define DEBUG 1

typedef struct 
	{
	int     o;
	int     op1;
	int     op2;
	}Parametres;

//Structure sauvegarder en zone de mémoire partagé
typedef struct 
	{
	int PointSauvegarde;
	int taille;
	Parametres info;
	}infoAppli;
infoAppli zmp;

int *adrrZmp;

int initSurveillance(void *param)
	{
	// Tirage de clé
	key_t cle ;
	if((cle=ftok("watchdog.c", 'A'))==-1) 
		{
		printf("Erreur 01 (initsurveillance): ftok\n");
		exit(-1);
		}

	//Ouverture de la zone de mémoire partagée
	int id;
	if((id=shmget(cle, sizeof(infoAppli) , 0))==-1) 
		{
		printf("Erreur 02 (initSurveillance) : shmget\n");
		exit(-1);
		}

	//Attachement de la zone de mémoire partagée
	if((adrrZmp=shmat(id, NULL, 0))==(void*)-1)
		{
		printf("Erreur 03 (initSurveillance) : shmat\n");
		exit(-1);
		}

		//mise à jour des information dans la structure local
		zmp.PointSauvegarde=((infoAppli*)adrrZmp)->PointSauvegarde;
		zmp.taille=((infoAppli*)adrrZmp)->taille;
		zmp.info=((infoAppli*)adrrZmp)->info;

	//Mise à jour des paramètres
	if(zmp.PointSauvegarde!=0)
		{
		//mise à jour des paramètres nécessaire pour l'exécution de l'application à surveiller
		memcpy(param, &zmp.info, zmp.taille);

		//DEBUG : tests
		if(DEBUG==1) printf("DEBUG : test 3, point de sauvegarde reprit=%d\n",zmp.PointSauvegarde);

		return zmp.PointSauvegarde;
		}
	else
		{
		return 0;
		}
	}

void pointReprise(int num , void *param, int taille) //taille correspond à taille des paramètres à sauvegarder et param adresse des donné à sauvegarder
	{
	//Vérification taille des éléments
	if(taille > sizeof(Parametres))
		{
		printf("Erreur 01 (pointReprise) : Taille max dépasser\n");
		zmp.PointSauvegarde=-1;
		memcpy(adrrZmp, &zmp, sizeof(infoAppli));
		exit(-1);
		}

	//sauvegarde des information dans la structure local
	memcpy(&zmp.info, param, taille);
	zmp.PointSauvegarde=num;
	zmp.taille=taille;

	//DEBUG : tests
	if(DEBUG==1) printf("DEBUG : test 3, point de sauvegarde écrite=%d\n",num);	

	//sauvegarde des information dans la zone de mémoire partagé
	memcpy(adrrZmp, &zmp, sizeof(infoAppli));
	}

void finAppli()
	{
	zmp.PointSauvegarde=-2;

	//mise à jour de la zone de mémoire partagé
	*((infoAppli*)adrrZmp) = zmp ;

	exit(0);
	}

