/** 
 * TVPI Theory. Constraints of the form   x<=k and -x<= k
 */

#ifndef __TVPI__H_
#define __TVPI__H_
#include "ldd.h"
#include "gmp.h"

typedef mpq_t* tvpi_cst_t;



#ifdef __cplusplus
extern "C" {
#endif

  theory_t *tvpi_create_theory (size_t vn);
  theory_t *tvpi_create_utvpiz_theory (size_t vn);
  theory_t *tvpi_create_tvpiz_theory (size_t vn);
  theory_t *tvpi_create_box_theory (size_t vn);
  theory_t *tvpi_create_boxz_theory (size_t vn);
  
  void tvpi_destroy_theory (theory_t*);
  
  tvpi_cst_t tvpi_create_cst(mpq_t k);
  void tvpi_cst_set_mpq (mpq_t res, tvpi_cst_t k);

  int Ldd_CountMaxConsecutiveVariableBoxTheory(LddManager * ldd , LddNode *f, int var);
  int Ldd_NumberOfDifferentVariablesBoxTheory(LddManager *ldd, LddNode *f);
  int Ldd_LongestPath(LddManager *ldd, LddNode *f);
  float Ldd_AverageDepthOfVarBoxTheory(LddManager *ldd, LddNode *f, int var);
  void Ldd_DumpDot( LddManager * ldd , LddNode *f , FILE * fp );
  void Ldd_DumpDotVerbose( LddManager * ldd , LddNode *f , FILE * fp );
  int *Ldd_CountSigns(LddManager * ldd , LddNode *f);
  int* Ldd_CountOp(LddManager * ldd , LddNode *f);
  void Ldd_DumpBoxTheoryMap(FILE *fp, LddManager *m);
  int Ldd_AllLddsInMapAreBasic(LddManager *m);
  bool Ldd_AtLeastOneSameVariableTriad(LddManager *ldd, LddNode* f, int var);
  bool Ldd_AtLeastOneOnlyThenChild(LddManager *ldd, LddNode* f, int var);
  bool Ldd_AtLeastOneOnlyElseChild(LddManager *ldd, LddNode* f, int var);

  LddNode **Ldd_SplitBoxTheory(LddManager *m, LddNode* f, lincons_t cons);

#ifdef __cplusplus
}
#endif
#endif
