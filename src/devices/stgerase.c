#include "copyright.h"
#include <stdio.h>
#include "options.h"
#include "tty.h"
#include "stdgraph.h"
#include "sm.h"
#include "sm_declare.h"
/*
 * stg_erase -- Clear the workstation screen
 */
void
stg_erase()
{
   stg_ctrl("CL");
}

/*
 * stg_page -- Begin a new graphics page
 */
void
stg_page()
{
   stg_ctrl("PG");
}
