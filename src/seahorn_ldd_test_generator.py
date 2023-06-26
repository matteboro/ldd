import random

VAR_NUM=15

code = f"""
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "cudd.h"
#include "util.h"
#include "ldd.h"
#include "tvpi.h"
#include <sys/time.h>

#define AND_T(c,n) and_t->create_linterm ((c),(n))
#define AND_C(n) and_t->create_int_cst(n)
#define AND_CONS(tm,n,c) and_t->create_cons (AND_T(tm,n),0,AND_C(c))

#define SPLIT_T(c,n) split_t->create_linterm ((c),(n))
#define SPLIT_C(n) split_t->create_int_cst(n)
#define SPLIT_CONS(tm,n,c) split_t->create_cons (SPLIT_T(tm,n),0,SPLIT_C(c))

#define False 0
#define True 1

void write_ldd_to_file(LddManager* ldd, LddNode* f, const char *filename, bool verbose)  {"{"}
    FILE *outfile; 
    outfile = fopen(filename,"w");
    if (verbose)
      Ldd_DumpDotVerbose(ldd, f,  outfile);
    else
      Ldd_DumpDot(ldd, f,  outfile);
    fclose (outfile); 
{"}"}

int total_test = 0;
int test_passed = 0;
long unsigned int and_tot_time = 0;
long unsigned int split_tot_time = 0;

struct timeval stop, start;

"""

NUM_TEST= 10
test_func_counter=0
INTERM_LOOPS = 50
TEST_LOOPS = 10
BASIC_LOOPS = 50
SPLIT_TEST_LOOPS = 100

for f in range(NUM_TEST):
    test_code = f"""
void test{test_func_counter}() {"{"}

  LddManager* and_ldd;
  LddManager* split_ldd;

  DdManager *and_cudd;
  DdManager *split_cudd;

  theory_t *and_t;
  theory_t *split_t;

  and_cudd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, 127, 0);
  split_cudd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, 127, 0);

  and_t = tvpi_create_theory({VAR_NUM+1});
  split_t = tvpi_create_theory({VAR_NUM+1});

  and_ldd = Ldd_Init (and_cudd, and_t);
  split_ldd = Ldd_Init (split_cudd, split_t);
"""

    var_coeff_dec = ""

    for i in range(VAR_NUM):
        var_coeff_dec += f"  int x{i}[{VAR_NUM}] = {'{'}"
        for j in range(VAR_NUM):
            if i == j:
                var_coeff_dec += "1"
            else:
                var_coeff_dec += "0"
            if j != VAR_NUM-1:
                var_coeff_dec += ","
        var_coeff_dec += "};\n"

    test_code += var_coeff_dec + "\n\n"

    # generate basic ldds
    test_code += "  //______________ BASIC LDDS GENERATION __________________\n\n"
    name_counter = 0
    ldds = []

    for i in range(BASIC_LOOPS):
        var = random.randint(1, VAR_NUM-1)
        cst = random.randint(-20, 20)
        ldd_dec = f"  LddNode* and_l{name_counter} = Ldd_FromCons(and_ldd, AND_CONS(x{var},{VAR_NUM},{cst}));\n"
        ldd_dec += f"  Ldd_Ref(and_l{name_counter});\n"
        ldd_dec += f"  LddNode* split_l{name_counter} = Ldd_FromCons(split_ldd, SPLIT_CONS(x{var},{VAR_NUM},{cst}));\n" 
        ldd_dec += f"  Ldd_Ref(split_l{name_counter});\n"
        if random.choice([True, False]):
            ldd_dec += f"  and_l{name_counter} = Ldd_Not(and_l{name_counter});\n"
            ldd_dec += f"  split_l{name_counter} = Ldd_Not(split_l{name_counter});\n"
        ldds.append(f"l{name_counter}")
        name_counter += 1
        test_code += ldd_dec + "\n"

    # generate intermidiate ldds
    tmp_counter = 0
    test_code += "  //______________ INTERM LDDS GENERATION __________________\n\n"

    for i in range(INTERM_LOOPS):
        op1 = random.choice(ldds)
        op2 = random.choice(ldds)

        operation = random.choice(["And", "Or", "Xor"])

        ldd_dec = f"  LddNode* and_tmp{tmp_counter} = Ldd_{operation}(and_ldd, and_{op1}, and_{op2});\n"
        ldd_dec += f"  Ldd_Ref(and_tmp{tmp_counter});\n"
        ldd_dec += f"  LddNode* split_tmp{tmp_counter} = Ldd_{operation}(split_ldd, split_{op1}, split_{op2});\n"
        ldd_dec += f"  Ldd_Ref(split_tmp{tmp_counter});\n"
        ldds.append(f"tmp{tmp_counter}")
        tmp_counter += 1
        test_code += ldd_dec + "\n"


    # generate the testing ldd
    test_counter = 0
    test_code += "  //______________ TEST LDD GENERATION __________________\n\n"
    for i in range(TEST_LOOPS):
        op1 = random.choice(ldds) if test_counter == 0 else f"test{test_counter-1}"
        op2 = random.choice(ldds)

        operation = random.choice(["And", "Or", "Xor"])

        ldd_dec = f"  LddNode* and_test{test_counter} = Ldd_{operation}(and_ldd, and_{op1}, and_{op2});\n"
        ldd_dec += f"  Ldd_Ref(and_test{test_counter});\n"
        ldd_dec += f"  LddNode* split_test{test_counter} = Ldd_{operation}(split_ldd, split_{op1}, split_{op2});\n"
        ldd_dec += f"  Ldd_Ref(split_test{test_counter});\n"
        test_counter += 1
        test_code += ldd_dec + "\n"

    #test_code += f'  write_ldd_to_file(and_ldd, and_test{test_counter-1}, "./deep_compare/and_ldd_{test_func_counter}.dot", True);\n'
    #test_code += f'  write_ldd_to_file(split_ldd, split_test{test_counter-1}, "./deep_compare/split_ldd_{test_func_counter}.dot", True);\n'
    
    for i in range(SPLIT_TEST_LOOPS):

      var = random.randint(1, VAR_NUM-1)
      cst = random.randint(-20, 20)
      and_cons = f"lincons_t and_c = AND_CONS(x{var},{VAR_NUM},{cst});\n"
      split_cons = f"lincons_t split_c = SPLIT_CONS(x{var},{VAR_NUM},{cst});\n"
    
      test_code += f"""

  /*########################################  TEST{test_func_counter}__{i}  ################################################*/ 

  {"{"}
  const char fn_split_pos[100] = "./deep_compare/{test_func_counter}_{i}_pos_split.dot";
  const char fn_split_neg[100] = "./deep_compare/{test_func_counter}_{i}_neg_split.dot";
  const char fn_and_pos[100] = "./deep_compare/{test_func_counter}_{i}_pos_and.dot";
  const char fn_and_neg[100] = "./deep_compare/{test_func_counter}_{i}_neg_and.dot";
  const char fn_and_bef[100] = "./deep_compare/{test_func_counter}_{i}_before_and.dot";
  const char fn_split_bef[100] = "./deep_compare/{test_func_counter}_{i}_before_split.dot";

  fprintf(stdout, "test with constraint: x{var} <= {cst}, started...\\n");

  {and_cons}
  {split_cons}
  
  write_ldd_to_file(and_ldd, and_test{test_counter-1}, fn_and_bef, True);
  write_ldd_to_file(split_ldd, split_test{test_counter-1}, fn_split_bef, True);

  // MY_SPLIT_IMPL

  gettimeofday(&start, NULL);

  LddNode* split_cons_node = Ldd_FromCons(split_ldd, split_c);
  Ldd_Ref(split_cons_node);

  LddNode** nodes = Ldd_Split(split_ldd, split_test{test_counter-1}, split_cons_node);

  Cudd_Ref(nodes[0]);
  Cudd_Ref(nodes[1]);

  gettimeofday(&stop, NULL);
  split_tot_time += (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
  
  // LDD_AND_SPLIT

  gettimeofday(&start, NULL);

  LddNode* cons_node = Ldd_FromCons(and_ldd, and_c);
  Ldd_Ref(cons_node);

  LddNode* pos_ldd = Ldd_And(and_ldd, and_test{test_counter-1}, cons_node);
  Cudd_Ref(pos_ldd);

  LddNode* neg_ldd = Ldd_And(and_ldd, and_test{test_counter-1}, Cudd_Not(cons_node));
  Cudd_Ref(neg_ldd);
  
  gettimeofday(&stop, NULL);
  and_tot_time += (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;

  bool pos_passed, neg_passed;
  bool ref_pos_passed, ref_neg_passed;

  if (!(pos_passed = Ldd_EqualRefLeq(and_ldd, pos_ldd, split_ldd, nodes[0]))) {"{"}
    fprintf(stdout, "  test number {test_func_counter}.{i} did not pass on positive result\\n");
    write_ldd_to_file(split_ldd, nodes[0], fn_split_pos, True);
    write_ldd_to_file(and_ldd, pos_ldd, fn_and_pos, True);
  {"}"}

  if (!(neg_passed = Ldd_EqualRefLeq(and_ldd, neg_ldd, split_ldd, nodes[1]))) {"{"}
    fprintf(stdout, "  test number {test_func_counter}.{i} did not pass on negative result\\n");
    write_ldd_to_file(split_ldd, nodes[1], fn_split_neg, True);
    write_ldd_to_file(and_ldd, neg_ldd, fn_and_neg, True);
  {"}"}

  if (!(ref_pos_passed = Ldd_EqualRefLeq(split_ldd, nodes[0], and_ldd, pos_ldd))) {"{"}
    // fprintf(stdout, "  there is a difference in refs count on positive result \\n");
    write_ldd_to_file(split_ldd, nodes[0], fn_split_pos, True);
    write_ldd_to_file(and_ldd, pos_ldd, fn_and_pos, True);
  {"}"}

  if (!(ref_neg_passed = Ldd_EqualRefLeq(split_ldd, nodes[1], and_ldd, neg_ldd))) {"{"}
    // fprintf(stdout, "  there is a difference in refs count on negative result \\n");
    write_ldd_to_file(split_ldd, nodes[1], fn_split_neg, True);
    write_ldd_to_file(and_ldd, neg_ldd, fn_and_neg, True);
  {"}"}
  
  if (ref_pos_passed && ref_neg_passed) {"{"}
    remove(fn_and_bef);
    remove(fn_split_bef);
  {"}"}
  
  if (neg_passed && pos_passed) {"{"}
    fprintf(stdout, "  test number {test_func_counter}.{i} passed\\n");
    ++test_passed;
  {"}"}

  ++total_test;
  free(nodes);
  {"}"}
"""

    test_code += """
  Ldd_Quit(and_ldd);
  Ldd_Quit(split_ldd);
"""
    test_code += "}\n"

    code += test_code
    test_func_counter += 1

code += """
int main(int argc, char** argv) {
"""

for i in range(NUM_TEST):
    code += f"  test{i}();\n"
  
code += """
  
  fprintf(stdout, "out of a total of %d tests, %d have passed\\n", total_test, test_passed);
  fprintf(stdout, "it took %lu us to execute the splits\\n", split_tot_time);
  fprintf(stdout, "it took %lu us to execute the ands\\n", and_tot_time);
}
"""

print(code)
