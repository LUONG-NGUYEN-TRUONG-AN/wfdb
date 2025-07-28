#include <stdio.h>
#include <wfdb/wfdb.h>

void main()
{
    int i, nsig;
    WFDB_Siginfo *siarray;

    nsig = isigopen("200", NULL, 0);
    if (nsig < 1)
        exit(1);
    siarray = (WFDB_Siginfo *)malloc(nsig * sizeof(WFDB_Siginfo));
    nsig = isigopen("200", siarray, nsig);
    for (i = 0; i < nsig; i++)
        printf("signal %d gain = %g\n", i, siarray[i].gain);
    exit(0);
}
