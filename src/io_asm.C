#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "proto_asm.h"
#include "allvars_asm.h"

#define FLOAT 1
#define DOUBLE 2
#define STRING 3
#define INT 4
#define MAXTAGS 300

void ReadParameterFile()
{

  FILE *fd, *fdout;
  char buf[200], buf1[200], buf2[200], buf3[400];
  char fname[256];
  int i, j, nt;
  int id[MAXTAGS];
  void *addr[MAXTAGS];
  char tag[MAXTAGS][50];

  sprintf(fname,clParameters.Paramfile);

  nt=0;
  
  strcpy(tag[nt], "Omegam");
  addr[nt] = &Parameters.Omegam;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "Omegab");
  addr[nt] = &Parameters.Omegab;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "Omegal");
  addr[nt] = &Parameters.Omegal;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "h");
  addr[nt] = &Parameters.h;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "ns");
  addr[nt] = &Parameters.ns;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "Sigma8");
  addr[nt] = &Parameters.Sigma8;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "w");
  addr[nt] = &Parameters.w;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "BoxSize");
  addr[nt] = &Parameters.BoxSize;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "N");
  addr[nt] = &Parameters.N;
  id[nt++] = INT;

  strcpy(tag[nt], "zInit");
  addr[nt] = &Parameters.zInit;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "DeltaFile");
  addr[nt] = &Parameters.DeltaFile;
  id[nt++] = STRING;  

  strcpy(tag[nt], "fov");
  addr[nt] = &Parameters.fov;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "theta");
  addr[nt] = &Parameters.theta;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "phi");
  addr[nt] = &Parameters.phi;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "NPixels");
  addr[nt] = &Parameters.NPixels;
  id[nt++] = INT;

  strcpy(tag[nt], "NSide");
  addr[nt] = &Parameters.NSide;
  id[nt++] = INT;  

  strcpy(tag[nt], "NRedshifts");
  addr[nt] = &Parameters.NRedshifts;
  id[nt++] = INT;

  strcpy(tag[nt], "InitialRedshift");
  addr[nt] = &Parameters.InitialRedshift;
  id[nt++] = FLOAT;

  strcpy(tag[nt], "FinalRedshift");
  addr[nt] = &Parameters.FinalRedshift;
  id[nt++] = FLOAT;

  if((fd = fopen(fname, "r")))
    {
      sprintf(buf, "%s%s", fname, "-usedvalues");
      if(!(fdout = fopen(buf, "w")))
	{
	  printf("error opening file '%s' \n", buf);
	}
      else
	{
	  while(!feof(fd))
	    {
	      *buf = 0;
	      fgets(buf, 200, fd);
	      if(sscanf(buf, "%s%s%s", buf1, buf2, buf3) < 2)
		continue;
	      
	      if(buf1[0] == '%' || buf1[0] == '#')
		continue;
	      
	      for(i = 0, j = -1; i < nt; i++)
		if(strcmp(buf1, tag[i]) == 0)
		  {
		    j = i;
		    tag[i][0] = 0;
		    break;
		  }
	      
	      if(j >= 0)
		{
		  switch (id[j])
		    {
		    case FLOAT:
		      *((float *) addr[j]) = atof(buf2);
		      fprintf(fdout, "%-35s%f\n", buf1, *((float *) addr[j]));
		      break;
		    case DOUBLE:
		      *((float *) addr[j]) = atof(buf2);
		      fprintf(fdout, "%-35s%f\n", buf1, *((float *) addr[j]));
		      break;
		    case STRING:
		      strcpy((char *)addr[j], buf2);
		      fprintf(fdout, "%-35s%s\n", buf1, buf2);
		      break;
		    case INT:
		      *((int *) addr[j]) = atoi(buf2);
		      fprintf(fdout, "%-35s%d\n", buf1, *((int *) addr[j]));
		      break;
		    }
		}
	      else
		{
		  fprintf(stdout, 
			  "Error in %s: Tag '%s' not allowed or multiple defined.\n",
			  fname, buf1);
		}
	    }
	  fclose(fd);
	  fclose(fdout);
	  
	}
    }
  else
    {
      printf("\nParameter file %s not found.\n\n", fname);
    }
  
   if(myid==0) printf("\n Parameter file read...");

#undef DOUBLE
#undef STRING
#undef INT
#undef MAXTAGS
  
}

void ReadReionFile(char *fname)
{
    
  int N=Parameters.N;

  float *slab = new float[N*N];

  long offset = (long)myid*(long)N*(long)N*(long)Nlocal*(long)sizeof(float);
  long bsize = N*N*sizeof(float);

  FILE *fd = fopen(fname,"rb");

  for(long i=0;i<Nlocal;i++){
    
    // Read this slab from the input file

    long int offset_local = i*N*N*sizeof(float) + offset;
    parallel_read(fname, bsize, offset_local, slab);

    for(long j=0;j<N;j++){
      for(long k=0;k<N;k++){
	long index = i*N*N+j*N+k;
	zreion[index] = slab[j*N+k]; 
      }
    }

  }
  
  delete[] slab;

  if(myid==0) printf("\n Input data read...");

}

void ReadDeltaFile(char *fname)
{
    
  int N=Parameters.N;

  float *slab = new float[N*N];

  long offset = (long)myid*(long)N*(long)N*(long)Nlocal*(long)sizeof(float);
  long bsize = N*N*sizeof(float);

  FILE *fd = fopen(fname,"rb");
  if(fd==NULL){
    printf("could not open file %s\n",fname);
    exit(0);
    MPI_Finalize();
  }

  for(long i=0;i<Nlocal;i++){
    
    // Read this slab from the input file

    long int offset_local = i*N*N*sizeof(float) + offset;
    parallel_read(fname, bsize, offset_local, slab);

    for(long j=0;j<N;j++){
      for(long k=0;k<N;k++){
	long index = i*N*(N+2)+j*(N+2)+k;
	delta[index] = slab[j*N+k]; 
      }
    }

  }

  /* For testing purposes, make delta = distance from center 
  for(long i=0;i<Nlocal;i++){
    for(long j=0;j<N;j++){
      for(long k=0;k<N;k++){
	float r = (i-N/2)*(i-N/2)+(j-N/2)*(j-N/2)+(k-N/2)*(k-N/2);
	r=sqrt(r)/N;
	long index = i*N*(N+2)+j*(N+2)+k;
	delta[index] = r;
      }
    }
  }
  */
  
  delete[] slab;

  if(myid==0) printf("\n Density data read...");

}

void WriteMaps()
{

  char fnametau[256], fnameksz[256], fnamedtb[256], fnamekap[256];

  FILE *fouttau, *foutksz, *foutdtb, *foutkap;

  if(myid==0) {
    printf("\n Writing maps...");
  } else {
    return;
  }

  if(myid==0){
    
    sprintf(fnametau,"%s_tau.bin",clParameters.BaseOut);
    sprintf(fnameksz,"%s_ksz.bin",clParameters.BaseOut);
    sprintf(fnamedtb,"%s_dtb.bin",clParameters.BaseOut);
    sprintf(fnamedtb,"%s_kap.bin",clParameters.BaseOut);

    fouttau = fopen(fnametau,"wb");
    foutksz = fopen(fnameksz,"wb");
    foutdtb = fopen(fnamedtb,"wb");
    foutkap = fopen(fnamekap,"wb");
    
    int npx = Parameters.NSide*Parameters.NSide*12;
    fwrite(taumap,4,npx,fouttau);
    fwrite(kszmap,4,npx,foutksz);
    fwrite(dtbmap,4,npx,foutdtb);
    fwrite(kapmap,4,npx,foutkap);
    
    fclose(fouttau);
    fclose(foutksz);
    fclose(foutdtb);
    fclose(foutkap);

    sprintf(fnametau,"!%s_tau.fits",clParameters.BaseOut);
    sprintf(fnameksz,"!%s_ksz.fits",clParameters.BaseOut);
    sprintf(fnamedtb,"!%s_dtb.fits",clParameters.BaseOut);
    sprintf(fnamekap,"!%s_kap.fits",clParameters.BaseOut);
    
    char coord[1];
    sprintf(coord,"C");

    write_healpix_map(taumap, Parameters.NSide, fnametau, 1, coord);
    write_healpix_map(kszmap, Parameters.NSide, fnameksz, 1, coord);
    write_healpix_map(dtbmap, Parameters.NSide, fnamedtb, 1, coord);
    write_healpix_map(kapmap, Parameters.NSide, fnamekap, 1, coord);

    if(myid==0) printf("\n Maps written...");

  }
}

