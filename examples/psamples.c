#include <stdio.h>
#include <wfdb/wfdb.h>

#define DEFWFDB ". /home/an/physionet.org/files/mitdb/1.0.0"
void main()
{
    int i;
    WFDB_Sample v[2];
    WFDB_Siginfo s[2];

    FILE *fp = fopen("200.txt", "w");
    if (isigopen("200", s, 2) < 2)
	exit(1);
    for (i = 0; i < 1000000; i++) {
	if (getvec(v) < 0)
	    break;
	fprintf(fp, "%d\n", v[0]);
    }
    fclose(fp);
    exit(0);
}
