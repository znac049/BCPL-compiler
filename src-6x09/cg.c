/* Copyright (c) 2012 Robert Nordier. All rights reserved. */

/* BCPL compiler backend: generate x86 assembler from OCODE. */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oc.h"

#define StaticDataSize 5000 /* Size of static data array */
#define BytesPerWord 4

/* data types used in load stack */
#define X_R  0
#define X_P  1
#define X_G  2
#define X_L  3
#define X_N  4
#define X_LP 5
#define X_LG 6
#define X_LL 7

/* 6309 instructions in table */
#define X_LD   0
#define X_ST   1
#define X_LEA  2
#define X_BRA  3
#define X_CALL 4
#define X_IMUL 5
#define X_IDIV 6
#define X_SUB  7
#define X_CMP  8
#define X_ADD  9
#define X_NEG  10
#define X_NOT  11
#define X_AND  12
#define X_OR   13
#define X_XOR  14


/* used in table below */
#define XCJ 8 /* call/jump instruction */
#define XI1 4 /* eax assumed */
#define XNA 3 /* mask */

static const char *const xistr[] =
{
    "ld", "st", "lea", "bra", "eee", "fff", "hhh",
    "sub", "cmp", "add", "neg", "not", "and", "or",
    "xor"
};

static const int xitab[] =
{
    2, 2, 2, XCJ | 1, XCJ | 1, XI1 | 2, XI1 | 2,
    2, 2, 2, 1, 1, 2, 2, 
    2
};

/* machine registers */
static const char *const reg[] =
{
    "a", "w"
};

/* relational operators */
static const char *relstr[][2] =
{
    {"ne", "e"},
    {"e", "ne"},
    {"ge", "l"},
    {"le", "g"},
    {"g", "le"},
    {"l", "ge"}
};

typedef struct lsi {
    int type;
    int data;
} loadStackItem;

// Replacing this with a typedef'd structure
static int ltype[2];       /* load stack: type */
static int ldata[2];       /* load stack: data */
static int lp;             /* load stack pointer */
static loadStackItem loadStack[2];

static int strNum = 1;

static int sdata[StaticDataSize][2]; /* static data */
static int dt;             /* data pointer */
static int sp;             /* stack pointer */
static int labno;          /* label counter */
static int loff;           /* label offset */
static int ch;             /* last char read */

static int  gencode(void);
static void codelab(int);
static void emit(const char *, ...);
static char *label(int);
static int  rdop(int);
static int  rdn(void);
static void error(const char *, ...);
static void comment(const char *, ...);

static void load(int, int);
static void save(int, int);
static void force(int);
static void loadreg(int, int);
static void codex(int);
static void code(int, ...);

static void defdata(int, int);
static void outdata(int, int);


int main(void)
{
    int op;

    do
    {
        labno = 1000;
        op = gencode();
        loff += 1000;
    } while (op != S_END);

    return 0;
}

static int gencode(void)
{
    int op;
    int n;
    int labNum;

    int s1 = 0;     // number of pops (0,1,2)
    int s2 = 0;     // number of pushes (0,1,2 - set to n; 3 - set to n+)
    int s3 = 0;     // ?????

    int sn = 0;

    int ro = 0;

    dt = sp = lp = 0;

    for (;;)
    {
        op = rdop(0);
        if (ch == EOF) {
            return S_END;
        }

        if (op < 0 || op > OPMAX) {
            error("Bad op %d", op);
        }

        s1 = optab1(op);
        s2 = optab2(op);
        s3 = optab3(op);

        comment("sn=%d, s1=%d, s2=%d, s3=%d", sn, s1, s2, s3);

        if (s3 <= 7) {
            force(s1);
            if (s3) {
                loadreg(0, s3 == 1 ? 0 : s3 == 2 ? ltype[0] != X_N : 1);
            }

            if (s3 >= 4) {
                loadreg(1, s3 == 4 ? 0 : s3 == 5 ? ltype[1] == X_N : 1);
            }
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
                codelab(labNum);
                emit("puls\ta");
                emit("exg\ta,s");
                emit("sta\t2,s");
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
            load(X_N, 0);
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
                emit("rts");
                emit("section data");

                // output the static data
                for (i=0; i<dt; i++) {
                    outdata(sdata[i][0], sdata[i][1]);
                }

                // Now output the globals
                for (i=0; i<numGlobs; i++) {
                    globNum = rdn();
                    labNum = rdn();

                    emit("export G%d", globNum);
                    sprintf(line, "G%d\tequ\t%s", globNum, label(labNum));
                    emit(line);
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
            emit("bra\tL%d", labNum);
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
            load(X_G, n);
            break;

        case S_LL:
            labNum = rdn();
            comment("LL L%d", labNum);
            load(X_L, n);
            break;

        case S_LLG:
            n = rdn();
            comment("LLG %d", n);
            load(X_LG, n);
            break;

        case S_LLL:
            labNum = rdn();
            comment("LLL L%d", labNum);
            load(X_LL, n);
            break;

        case S_LLP:
            n = rdn();
            comment("LLP %d", n);
            load(X_LP, n);
            break;

        case S_LN:
            n = rdn();
            comment("LN %d", n);
            load(X_N, n);
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
            load(X_P, n);
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
            sn = rdn();
            comment("SAVE %d", sn);
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
            sn = rdn();
            comment("STACK %d", sn);
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
            load(X_N, 0xffff);
            break;

        default:
            error("Unknown op %d", op);
        }

        /* adjust stack pointer */
        sp = s2 & 2 ? sn : sp - s1;
        if (s2 & 1)
            sp++;
        /* adjust load stack pointer */
        if (s3 <= 7) {
            lp = s2 & 1;
        } else if (s3 == 8 && lp < 2) {
            lp++;
        }
    }

    return op;
}

static void codelab(int n)
{
    char buf[32];

    sprintf(buf, "%s:", label(n));
    emit(buf);
}

static void emit(const char *fmt, ...)
{
    va_list ap;
    char line[100];
    char *sp;
    int len;

    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);

    sp = line;

    while (isupper(*sp) || isdigit(*sp) || (*sp == ':')) {
        putchar(*sp++);
    }

    while ((*sp == ' ') || (*sp == '\t')) {
        sp++;
    }

    if (*sp) {
        putchar('\t');
    }

    len = strlen(sp);
    if (sp[len-1] == '\n') {
        sp[len-1] = '\0';
    }
    puts(sp);
}

static char *label(int n)
{
    static char buf[32];

    sprintf(buf, "L%d", loff + n);
    return buf;
}

static int rdop(int peek)
{
    static int got, op;

    if (!got) {
        op = rdn();
    }
    got = peek;
    return op;
}

static int rdn(void)
{
    int neg, n;

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
    return neg ? -n : n;
}

static void error(const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

static void comment(const char *msg, ...)
{
    va_list ap;

    printf("\t\t\t\t; ");
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    fputc('\n', stdout);
    va_end(ap);
}






/*
 ***************************************************************
 */



static void load(int t, int d)
{
    comment("load(%d, %d)", t, d);
    assert(lp >= 0 && lp <= 2);
    if (lp == 2) {
        force(1);
    }
    ltype[lp] = t;
    ldata[lp] = d;
}

static void save(int k, int v)
{
    comment("save(%d, %d)\n", k, v);
    code(X_ST, k, v, ltype[0], ldata[0]);
}

static void force(int st)
{
    comment("force(%d)", st);
    comment(" lp=%d, sp=%d", lp, sp);
    assert(lp >= 0 && lp <= 2);
    assert(st >= 0 && st <= 2);
    if (lp == st) {
        return;
    }
    if (lp == 0) {
        ltype[0] = X_R;
        ldata[0] = 0;
        code(X_LD, ltype[0], ldata[0], X_P, sp - 1);
        lp = 1;
        if (lp == st) {
            return;
        }
    }
    assert(lp != 0);
    if (st == 2) {
        assert(ltype[0] == X_R && ldata[0] == 0);
        emit("movvvv %%eax,%%ecx");
        code(X_LD, ltype[0], ldata[0], X_P, sp - 2);
        ltype[1] = X_R;
        ldata[1] = 1;
        lp = 2;
    } else {
        comment("wooo!");
        do
        {
            loadreg(0, 1); //ltype[0] != X_N);
            code(X_ST, X_P, sp - lp, ltype[0], ldata[0]);
            if (lp == 2) {
                ltype[0] = ltype[1];
                ldata[0] = ltype[1] == X_R ? 0 : ldata[1];
            }
        } while (--lp != st);
    }
}

static void loadreg(int i, int must)
{
    int t, p;

    comment("loadreg(%d, %d)", i, must);

    t = ltype[i];
    if (t == X_R) {
        comment("  nope");
        return;
    }
    p = t == X_LP || t == X_LG || t == X_LL;
    if (p || must) {
        comment("  yep");
        code(p && t != X_LL ? X_LEA : X_LD, t, ldata[i], X_R, i);
        if (p) {
            emit("shr $2,%%%s", reg[i]);
        }
        ltype[i] = X_R;
        ldata[i] = i;
    }
    else {
        comment("  nope");
    }
}

static void codex(int xi)
{
    code(xi, ltype[0], ldata[0], ltype[1], ldata[1]);
}

static void code(int xi, ...)
{
    char buf[64], *s;
    int typ[2], dat[2];
    va_list ap;
    int cj, i1, na, x, i, t, d;

    va_start(ap, xi);
    x = xitab[xi];
    cj = x & XCJ;
    i1 = x & XI1 ? 1 : 0;
    na = x & XNA;
    comment("    na=%d, cj=%d, i1=%d", na, cj, i1);
    for (i = 0; i < na; i++)
    {
        typ[i] = va_arg(ap, int);
        dat[i] = va_arg(ap, int);
    }
    s = buf;
    s += sprintf(s, xistr[xi]);
    for (i = na - 1; i >= i1; i--)
    {
        if (i != na-1)
            *s++ = '\t';
        //*s++ = i == na - 1 ? ' ' : ',';
        t = typ[i];
        d = dat[i];
        comment("    i=%d, t=%d, d=%d", i, t, d);
        
        if (t <= 3) {
            if (cj) {
                *s++ = '*';
            }
        } else if (t != X_LP && t != X_LG && !cj) {
            *s++ = '#';
        }
        switch (t)
        {
        case X_R:
            sprintf(s, "%s", reg[d]);
            break;
        case X_P:
        case X_LP:
            sprintf(s, "%d,s", d * BytesPerWord);
            break;
        case X_G:
        case X_LG:
            sprintf(s, "%d,u", d * BytesPerWord);
            break;
        case X_L:
        case X_LL:
            sprintf(s, "%s", label(d));
            break;
        case X_N:
            sprintf(s, "%d", d);
            break;
        }
        while (*s) {
            s++;
        }
    }
    va_end(ap);
    emit("%s", buf);
}






static void defdata(int k, int v)
{
    if (dt >= StaticDataSize) {
        error("Too many constants");
    }
    sdata[dt][0] = k;
    sdata[dt][1] = v;
    dt++;
}

static void outdata(int k, int n)
{
    switch (k)
    {
    case S_DATALAB:
        //emit(".align 4");
        codelab(n);
        return;
    case S_ITEMN:
        emit("fdb %d", n);
        return;
    case S_ITEML:
        emit("fdb %s", label(n));
        return;
    case S_LSTR:
        emit("fcb %d", n);
        return;
    }
}
