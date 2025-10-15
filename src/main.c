#include "Global.h"
#include "LG.h"
#define OSL (42) /* opt_string's length */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Added for memcmp and strdu

FILE *inFILE, *outFILE;

typedef struct {
  int p[SYM_Nmax][VERT_Nmax];
} VPermList;


int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  int ctr = 0;


  outFILE = stdout;
  inFILE = fopen(argv[1], "r");

  CWS* CW = (CWS*)malloc(sizeof(CWS));
  EqList* E = (EqList*)malloc(sizeof(EqList));
  PolyPointList *_P = (PolyPointList*)malloc(sizeof(PolyPointList)),
                *_DP = (PolyPointList*)malloc(sizeof(PolyPointList));
  PairMat* PM = (PairMat*)malloc(sizeof(PairMat));
  VertexNumList V;

  int zinfo;
  while ((zinfo = Read_CWS_PP(CW, _P)) != NULL) {
    int IP;
    int SymNum;
    IP = Find_Equations(_P, &V, E);
    int R = EL_to_PPL(E, _DP, &_P->n);
    int numVerts = V.nv;

    Sort_VL(&V);
    Make_VEPM(_P, &V, E, *PM);

    // Allocate NF as a local array
    Long NF[POLY_Dmax][VERT_Nmax];
    VPermList* VP = (VPermList*)malloc(sizeof(VPermList));
    if (!VP) {
      fprintf(stderr, "Failed to allocate VP\n");
      continue;
    }

    // Get the result matrix
    Make_Poly_Sym_NF(_P, &V, E, &SymNum, VP->p, NF, 0, 0, 1);


    free(VP);
    fflush(outFILE);
  }




  // Cleanup
  free(CW);
  free(E);
  free(_P);
  free(_DP);
  free(PM);
  fclose(inFILE);
  return 0;
}