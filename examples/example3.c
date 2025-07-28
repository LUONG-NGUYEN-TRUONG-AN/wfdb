#include <stdio.h>
#include <wfdb/wfdb.h>

void main(int argc, char *argv[]) {
    WFDB_Anninfo a;
    WFDB_Annotation annot;

    if (argc < 3) {
        fprintf(stderr, "usage: %s annotator record\n", argv[0]);
        exit(1);
    }
    a.name = argv[1]; a.stat = WFDB_READ;
    (void)sampfreq(argv[2]);
    if (annopen(argv[2], &a, 1) < 0) exit(2);
    FILE *fp = fopen("output.txt", "w");
    if (!fp) {
        fprintf(stderr, "Error opening output file.\n");
        exit(3);
    }
    while (getann(0, &annot) == 0)
        fprintf(fp, " %s %s\n",
                annstr(annot.anntyp),
                (annot.aux != NULL && *annot.aux > 0) ?
                (char *) annot.aux+1 : "");
    fclose(fp);
    exit(0);
}
