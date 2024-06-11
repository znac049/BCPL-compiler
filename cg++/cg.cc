#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "ocode.h"
#include "machine.h"

machine mach;

void error(const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

void comment(const char *msg, ...)
{
    va_list ap;

    printf("\t\t\t\t; ");
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    fputc('\n', stdout);
    va_end(ap);
}

int rdn(void)
{
    int neg, n;
    char ch;

    neg = n = 0;
    do
    {
        ch = getchar();
    } while (ch == ' ' || ch == '\n');

    if (ch == 'L') {
        ch = getchar();
    } else if (ch == '-') {
        neg = 1;
        ch = getchar();
    }

    while (isdigit(ch))
    {
        n = n * 10 + ch - '0';
        ch = getchar();
    }

    if (neg) {
        n = -n;
    }

    //comment("rdn() -> %d", n);

    return n;
}

int rdop(int peek)
{
    static int got, op;

    if (!got) {
        op = rdn();
    }
    got = peek;
    return op;
}

int genCode()
{
    int op;
    int n;
    int labNum;

    for (;;)
    {
        op = rdop(0);
        if (op == EOF) {
            return S_END;
        }

        if (op < 0 || op > OPMAX) {
            error("Bad op %d", op);
        }

        switch (op) {
        case S_ABS:
            comment("ABS");
            break;

        case S_BLAB:
            labNum = rdn();
            comment("BLAB L%d", labNum);
            break;

        case S_DATALAB:
            labNum = rdn();
            comment("DATALAB L%d", labNum);
            break;

        case S_DIV:
            comment("DIV");
            break;

        case S_ENDFOR:
            labNum = rdn();
            comment("ENDFOR L%d", labNum);
            break;

        case S_ENTRY:
            {
                int nameLen = rdn();
                char *name = malloc(nameLen+1);
                int i;
                labNum = rdn();

                for (i=0; i<nameLen; i++) {
                    char ch = rdn();

                    if (i<nameLen)
                        name[i] = ch;
                }
                name[nameLen] = '\0';
                printf("\n");
                comment("ENTRY %d L%d '%s", nameLen, labNum, name);
                //codelab(labNum);
                free(name);
            }
            break;

        case S_ENDPROC:
            n = rdn();
            comment("ENDPROC %d ???", n);
            break;

        case S_EQ:
            comment("EQ");
            break;

        case S_EQV:
            comment("EQV");
            break;

        case S_FALSE:
            comment("FALSE");
            break;

        case S_FINISH:
            comment("FINISH");
            return op;

        case S_FNAP:
            n = rdn();
            comment("FNAP %d", n);
            break;

        case S_FNRN:
            comment("FNRN");
            break;

        case S_GE:
            comment("GE");
            break;

        case S_GETBYTE:
            comment("GETBYTE");
            break;

        case S_GLOBAL:
            {
                int numGlobs = rdn();
                int globNum;
                int i;
                char line[100];

                comment("GLOBAL %d ...", numGlobs);

                // output the static data
                // Now output the globals
                for (i=0; i<numGlobs; i++) {
                    globNum = rdn();
                    labNum = rdn();

                    // emit("export G%d", globNum);
                    // sprintf(line, "G%d\tequ\t%s", globNum, label(labNum));
                    // emit(line);
                }
            }
            break;

        case S_GOTO:
            comment("GOTO");
            break;

        case S_GR:
            comment("GR");
            break;

        case S_ITEML:
            labNum = rdn();
            comment("ITEML L%d", labNum);
            break;

        case S_ITEMN:
            labNum = rdn();
            comment("ITEMN L%d", labNum);
            break;

        case S_JF:
            labNum = rdn();
            comment("JF L%d", labNum);
            break;

        case S_JT:
            labNum = rdn();
            comment("JT L%d", labNum);
            break;

        case S_JUMP:
            labNum = rdn();
            comment("JUMP L%d", labNum);
            break;

        case S_LAB:
            labNum = rdn();
            comment("LAB L%d", labNum);
            break;

        case S_LE:
            comment("LE");
            break;

        case S_LG:
            n = rdn();
            comment("LG %d", n);
            break;

        case S_LL:
            labNum = rdn();
            comment("LL L%d", labNum);
            break;

        case S_LLG:
            n = rdn();
            comment("LLG %d", n);
            break;

        case S_LLL:
            labNum = rdn();
            comment("LLL L%d", labNum);
            break;

        case S_LLP:
            n = rdn();
            comment("LLP %d", n);
            break;

        case S_LN:
            n = rdn();
            comment("LN %d", n);
            break;

        case S_LOGAND:
            comment("LOGAND");
            break;

        case S_LOGOR:
            comment("LOGOR");
            break;

        case S_LP:
            n = rdn();
            comment("LP %d", n);
            break;

        case S_LS:
            comment("LS");
            break;

        case S_LSHIFT:
            comment("LSHIFT");
            break;

        case S_LSTR:
            {
                int strLen = rdn();
                char *str = malloc(strLen+1);
                int i;

                for (i=0; i<strLen; i++) {
                    str[i] = rdn();
                }
                str[strLen] = '\0';
                comment("LSTR %d '%s'", strLen, str);
                free(str);
            }
            break;

        case S_MINUS:
            comment("MINUS");
            break;

        case S_MULT:
            comment("MULT");
            break;

        case S_NE:
            comment("NE");
            break;

        case S_NEEDS:
            {
                int strLen = rdn();
                char *str = malloc(strLen+1);
                int i;

                for (i=0; i<strLen; i++) {
                    str[i] = rdn();
                }
                str[strLen] = '\0';
                comment("NEEDS %d '%s'", strLen, str);
                free(str);
            }
            break;

         case S_NEG:
            comment("NEG");
            break;

        case S_NEQV:
            comment("NEQV");
            break;

        case S_NOT:
            comment("NOT");
            break;

        case S_PLUS:
            comment("PLUS");
            break;

        case S_PUTBYTE:
            comment("PUTBYTE");
            break;

        case S_QUERY:
            comment("QUERY");
            break;

        case S_REM:
            comment("REM");
            break;

        case S_RES:
            labNum = rdn();
            comment("RES L%d", labNum);
            break;

        case S_RSHIFT:
            comment("RSHIFT");
            break;

        case S_RSTACK:
            n = rdn();
            comment("RSTACK %d", n);
            break;

        case S_RTAP:
            n = rdn();
            comment("RTAP %d", n);
            break;

        case S_RTRN:
            comment("RTRN");
            break;

        case S_RV:
            comment("RV");
            break;

        case S_SAVE:
            n = rdn();
            comment("SAVE %d", n);
            break;
            
        case S_SECTION:
            {
                int strLen = rdn();
                char *str = malloc(strLen+1);
                int i;

                for (i=0; i<strLen; i++) {
                    str[i] = rdn();
                }
                str[strLen] = '\0';
                comment("SECTION %d '%s'", strLen, str);
                free(str);
            }
            break;

        case S_SG:
            comment("SG");
            break;

        case S_SL:
            labNum = rdn();
            comment("SL L%d", n);
            break;

        case S_SP:
            n = rdn();
            comment("SP %d", n);
            break;

        case S_STACK:
            n = rdn();
            comment("STACK %d", n);
            break;

        case S_STIND:
            comment("STIND");
            break;

        case S_STORE:
            comment("STORE - ignore");
            break;

        case S_SWITCHON:
            {
                int numCases = rdn();
                int defLab = rdn();
                int i;

                printf("SWITCHON %d L%d ", numCases, defLab);
                for (i=0; i<numCases; i++) {
                    int val = rdn();
                    int lab = rdn();

                    printf("%d->L%d ", val, lab);
                }
                printf("\n");
            }
            break;

        case S_TRUE:
            comment("TRUE");
            break;

        default:
            error("Unknown op %d", op);
        }
    }

    return op;
}

int main(int argc, char **argv)
{
    int op = genCode();

    return 0;
}