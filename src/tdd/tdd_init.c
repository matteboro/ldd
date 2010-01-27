#include "util.h"
#include "tddInt.h"

static int bin_false (lincons_t, lincons_t);

LddManager * 
Ldd_Init (DdManager *cudd, theory_t * t)
{
  LddManager* ldd;
  int i;
  
  ldd = ALLOC(LddManager, 1);
  if (ldd == NULL) return 0;
  

  /* set CUDD and theory */
  ldd->cudd = cudd;
  ldd->theory = t;

  ldd->be_bddlike = 0;

  /* allocate the map from DD nodes to linear constraints*/
  ldd->varsSize = cudd->maxSize;
  ldd->ddVars = ALLOC(lincons_t,ldd->varsSize);
  if (ldd->ddVars == NULL)
    {
      FREE(ldd);
      return 0;
    }
  for (i = 0; i < ldd->varsSize; i++)
    ldd->ddVars [i] = NULL;

  /* add a hook to fix MTR tree after variable reordering */
  Cudd_AddHook (CUDD, &Ldd_fix_mtr_tree, CUDD_POST_REORDERING_HOOK);
  
  return ldd;
}

void 
Ldd_Quit (LddManager * ldd)
{
  if (ldd->ddVars != NULL)
    {
      int i;
      for (i = 0; i < ldd->varsSize; i++)
	if (ldd->ddVars [i] != NULL)
	  {
	    THEORY->destroy_lincons (ldd->ddVars [i]);
	    ldd->ddVars [i] = NULL;
	  }
     
      FREE (ldd->ddVars);
      ldd->ddVars = NULL;
    }
  FREE (ldd);
}


LddNode* 
Ldd_FromCons (LddManager *ldd, lincons_t l)
{
  return THEORY->to_ldd(ldd, l);
}


/**
 * Converts a given theory t into a theory in which all implications
 * are syntactic.
 */
theory_t *
Ldd_SyntacticImplicationTheory (theory_t *t)
{
  t->is_stronger_cons = bin_false;
  return t;
}

LddManager *
Ldd_BddlikeManager (LddManager *ldd)
{
  ldd->be_bddlike = 1;
  return ldd;
}


static int 
bin_false (lincons_t l1, lincons_t l2)
{
  return 0;
}
