
   /**------ ( ----------------------------------------------------------**
    **       )\                      CAnDL                               **
    **----- /  ) --------------------------------------------------------**
    **     ( * (                    candl.c                              **
    **----  \#/  --------------------------------------------------------**
    **    .-"#'-.        First version: september 8th 2003               **
    **--- |"-.-"| -------------------------------------------------------**
    |     |
    |     |
 ******** |     | *************************************************************
 * CAnDL  '-._,-' the Chunky Analyzer for Dependences in Loops (experimental) *
 ******************************************************************************
 *                                                                            *
 * This program is a test to check if the functions named                     *
 * candl_dependence_is_loop_carried and candl_dependence_is_loop_independent  *
 * work correctly.                                                            *
 *                                                                            *
 * It takes for argument the path of a .scop file and a path to the expected  *
 * results of the 2 functions.                                                *
 * The results file format is the following :                                 *
 * 1st line : The number of iterator to check.                                *
 * 2nd line : The expected retured value of is_loop_carried for each          *
 *            loop index, separeted by a space.                               *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <osl/scop.h>
#include <osl/macros.h>
#include <osl/util.h>
#include <osl/extensions/dependence.h>
#include <candl/macros.h>
#include <candl/dependence.h>
#include <candl/violation.h>
#include <candl/options.h>
#include <candl/scop.h>
#include <candl/util.h>
#include <candl/piplib.h>

int main(int argc, char * argv[])
{
    osl_scop_p scop = NULL;
    osl_scop_p orig_scop = NULL;
    osl_dependence_p candl_dep = NULL, candl_dep_cursor = NULL;
    int num_scops = 0,  i= 0;
    candl_options_p options = candl_options_malloc();
    FILE *input_scop, *input_results;
    int * expected_results = NULL;
    int res = 0;
    unsigned int nb_iterator = 0;
    bool should_be_loop_independent = 0;

    int precision;
#if defined(CANDL_LINEAR_VALUE_IS_INT)
      precision = OSL_PRECISION_SP;
#elif defined(CANDL_LINEAR_VALUE_IS_LONGLONG)
      precision = OSL_PRECISION_DP;
#elif defined(CANDL_LINEAR_VALUE_IS_MP)
      precision = OSL_PRECISION_MP;
#endif

    if(argc < 3)
    {
        fprintf(stderr,"The .scop file or the result files has not been given");
        fprintf(stderr,"Usage : ./%s <scop file> <result file>", argv[0]);
        exit(1);
    }

    input_scop = fopen(argv[1], "r");
    input_results = fopen(argv[2], "r");


    fscanf(input_results, "%d\n", &nb_iterator);
    expected_results = (int*) malloc(nb_iterator * sizeof(int));
    for(int i=0; i<nb_iterator ; i++)
    {
        fscanf(input_results,"%d", &expected_results[i]);
    }


    osl_interface_p registry = osl_interface_get_default_registry();
    scop = osl_scop_pread(input_scop, registry, precision);
    if(scop == NULL)
    {
        CANDL_error("Fail to open scop");
        exit(1);
    }

    //Initiating candl, then using it to generate the dependences,
    //and finally extracting them to use later.
    candl_scop_usr_init(scop);
    candl_dependence_add_extension(scop, options);
    candl_dep = osl_generic_lookup(scop->extension, OSL_URI_DEPENDENCE);

    //For every loop, from outer to inner, we check if there is a loop carried dependences.
    //If that's the case, the statement can't be parallelized for this loop.
    for(i=0 ; i<nb_iterator ; i++)
    {
        //If the candl_dep is NULL, it means that there is no dependences whatsoever in the statement,
        //we don't have to check with candl_dependence_is_loop_carried.
        //res.loop_carried_dependences is already set to false for every loop index.
        candl_dep_cursor = candl_dep;
        while(candl_dep_cursor != NULL)
        {
            candl_dependence_is_loop_carried( candl_dep_cursor, i);
            candl_dep_cursor = candl_dep_cursor->next;
        }
    }


    candl_options_free(options);
    candl_scop_usr_cleanup(scop);
    osl_scop_free(scop);
    fclose(input_scop);
    fclose(input_results);

    return res;
}
