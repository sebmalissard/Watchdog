#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NOM_MAX 20
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

int main(int argc, char **argv)
	{
	//Mise a jour du chemin de l'application
	char application[NOM_MAX];
	if(argc==2)
		{
		if(sizeof(argv[1])>NOM_MAX)
			{
			printf("Erreur 10 (watchdog) : argv[1]>NOM_MAX\n");
			exit(-1);
			}
		strcpy (application, argv[1]);
		}
	else
		{
		strcpy (application, "application");
		}

	// Tirage de clé
	key_t cle ;
	if((cle=ftok("watchdog.c", 'A'))==-1) 
		{
		printf("Erreur 01 (watchdog) : ftok\n");
		exit(-1);
		}

	//Création de la zone de mémoire partagée
	int id;
	if((id=shmget(cle, sizeof(infoAppli) , IPC_CREAT | 0644))==-1) 
		{
		printf("Erreur 02 (watchdog) : shmget\n");
		exit(-1);
		}

	//Attachement de la zone de mémoire partagée
	int *adrrZmp;
	if((adrrZmp=shmat(id, NULL, 0))==(void*)-1)
		{
		printf("Erreur 03 (watchdog) : shmat\n");
		exit(-1);
		}

	//Création ou ouverture du fichier de suivi
	FILE* fic = NULL;
	if((fic = fopen("suivi.txt", "a"))==NULL) // a : mode ajout à la fin du fichier ou création s'il n'existe pas 
		{
		printf("Erreur 04 (watchdog) : fopen\n");
		exit(-1);
		}

	//Création d'un processus fils pour lancer l'application à surveiller
	int pid_fils=0;
	int status; //pour le wait
	if((pid_fils=fork())==-1)
		{
		printf("Erreur 05 (watchdog) : fork()\n");
		exit(-1);
		}
	
	//Executer par le processus fil : application à surveiller
	if(pid_fils==0)
		{
		//Exécution de l'application avec le numéro du point de sauvegarde/reprise
		if((execl(application, "premier lancement",NULL))==-1)
			{
			printf("Erreur 06 (watchdog) : argument non valide ou erreur execl\n");
			int processus_pere = getppid(); 
			kill(processus_pere,SIGKILL);
			exit(1);
			}
		}

	//Executer par le processus père : watchdog
	else
		{
		int fin=0;
		while(fin==0)
			{
			//Attente de la fermeture de l'application
			wait(&status); 

			//Date et heure à laquelle l'application s'est fermé
			time_t secondes;
			struct tm instant;
			time(&secondes);
			instant=*localtime(&secondes);

			//Mise à jour de la structure 
   			zmp.PointSauvegarde=((infoAppli*)adrrZmp)->PointSauvegarde;

			//DEBUG : tests		
			if(DEBUG==1) printf("DEBUG : test 3, point de sauvegarde lu=%d\n",zmp.PointSauvegarde);

			//Relancer le processus suite à une erreur
			if(zmp.PointSauvegarde!=-2 && zmp.PointSauvegarde!=-1) 
				{

				//Mise à jour du fichier suivi
				fprintf(fic,"Date : %d/%d à %dh %dmin %dsec : Fin anormale de %s, reprise au point numéros : %d\n", instant.tm_mday, instant.tm_mon+1, instant.tm_hour, instant.tm_min, instant.tm_sec, application, zmp.PointSauvegarde);

				//Création d'un processus fils pour lancer l'application à surveiller
				if((pid_fils=fork())==-1)
					{
					printf("Erreur 07 (watchgog) : fork()\n");
					exit(-1);
					}

				if(pid_fils==0)
					{
					//Exécution de l'application avec le numéro du point de sauvegarde/reprise
					if((execl(application, "reprise", NULL))==-1) 
						{
						printf("Erreur 08 (watchdog) : argument non valide ou erreur execl\n");
						zmp.PointSauvegarde=-1;
						return(-1);
						}
					}
				}

			//Fin de l'application normal
			else 
				{
				fin=1;
				if (zmp.PointSauvegarde==-2)
					{
					fprintf(fic,"Date : %d/%d à %dh %dmin %dsec : Fin normale de %s\n", instant.tm_mday, instant.tm_mon+1, instant.tm_hour, instant.tm_min, instant.tm_sec, application); 
					}

				//Supression de la zone de mémoire partagée
					if((shmctl(id, IPC_RMID, NULL))==-1)
						{
						printf("Erreur 09 (watchdog) : shmctl\n");
						exit(-1);
						}

				//Fermeture du fichier suivi
					fclose(fic);
				}
			}
		}	

	return 0;
	}


