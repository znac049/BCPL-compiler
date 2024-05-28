/* Copyright (c) 2012 Robert Nordier. All rights reserved. */

/* BCPL compiler backend: generate x86 assembler from OCODE. */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "oc.h"

#define WordSize 2    /* 16-bit code */
#define SDSZ    5000 /* Size of static data array */

#define SEC_DATA 1
#define SEC_CODE 2
#define SEC_BSS  3

static int currentSection = -1;

static const char *opNames[OPMAX+1] = {
    0,          // 0
    0,
    0,
    0,
    "TRUE", 
    "FALSE",    // 5
    0,
    0,
    "RV",
    0,
    "FNAP",     // 10
    "MULT",
    "DIV",
    "REM",
    "PLUS",
    "MINUS",    // 15
    "QUERY",
    "NEG",
    0,
    "ABS",
    "EQ",       // 20
    "NE",
    "LS",
    "GR",
    "LE",
    "LS",       // 25
    0,
    0,
    0,
    0,
    "NOT",      // 30
    "LSHIFT",
    "RSHIFT",
    "LOGAND",
    "LOGOR",
    "EQV",      // 35
    "NEQV",
    "COND",
    0,
    0,
    "LP",       // 40
    "LG",
    "LN",
    "LSTR",
    "LL",
    "LLP",      // 45
    "LLG",
    "LLL",
    "NEEDS",
    "SECCTION",
    0,          // 50
    "RTAP",
    "GOTO",
    0,
    0,
    0,          // 55
    0,
    0,
    0,
    0,
    0,          // 60
    0,
    0,
    0,
    0,
    0,          // 65
    0,
    "RETURN",
    "FINISH",
    0,
    0,          // 70
    0,
    0,
    0,
    0,
    0,          // 75
    "GLOBAL",
    0,
    0,
    0,
    "SP",       // 80
    "SG",
    "SL",
    "STIND",
    0,
    "JUMP",     // 85
    "JT",
    "JF",
    "ENDFOR",
    "BLAB",
    "LAB",      // 90
    "STACK",
    "STORE",
    "RSTACK",
    "ENTRY",
    "SAVE",     // 95
    "FNRN",
    "RTRN",
    "RES",
    "RESLAB",
    "DATALAB",  // 100
    "ITEML",
    "ITEMN",
    "ENDPROC",
    "END",
    0,          // 105
    0,
    0,
    0,
    0,
    0,          // 110
    0,
    0,
    0,
    0,
    0,          // 115
    0,
    0,
    0,
    0,
    "GETBYTE",  // 120
    "PUTBYTE"
};

static int vm_S;

static int sdata[SDSZ][2]; /* static data */
static int dt;             /* data pointer */
static int sp;             /* stack pointer */
static int ltype[2];       /* load stack: type */
static int ldata[2];       /* load stack: data */
static int lp;             /* load stack pointer */
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
static void section(int);

int main(void)
{
    int op;

    section(SEC_CODE);

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

    for (;;)
    {
        op = rdop(0);
	    //emit("# Op=%d", op);
        if (opNames[op]) {
            comment("%s (%d)", opNames[op], op);
        }
        else {
            comment("op %d has no name!", op);
        }

        if (ch == EOF) {
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
            comment("BLAB %d", labNum);
            break;

        case S_DATALAB:
            labNum = rdn();
            comment("DATALAB %d", nlabNum);
            section(SEC_DATA);
            emit("L%d", labNum);
            break;

        case S_DIV:
            comment("DIV");
            break;

        case S_ENDFOR:
            labNum = rdn();
            comment("ENDFOR %d", labNum);
            break;

        case S_ENTRY:
            {
                int nameLen = rdn();
                char name[64];
                int i;
                labNum = rdn();

                if (nameLen >= 64) {
                    fprintf(stderr, "ENTRY - name is too long!");
                }

                for (i=0; i<nameLen; i++) {
                    char ch = rdn();

                    if (i<nameLen)
                        name[i] = ch;
                }
                name[nameLen] = '\0';
                comment("ENTRY %d L%d '%s", nameLen, labNum, name);
            }
            break;

        case S_ENDPROC:
            comment("ENDPROC");
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
            break;

        case S_GOTO:
            break;

        case S_GR:
            break;

        case S_ITEML:
            break;

        case S_ITEMN:
            break;

        case S_JF:
            break;

        case S_JT:
            break;

        case S_JUMP:
            break;

        case S_LAB:
            break;

        case S_LE:
            break;

        case S_LN:
            break;

        case S_LG:
            break;

        case S_LL:
            break;

        case S_LLG:
            break;

        case S_LLL:
            break;

        case S_LLP:
            break;

        case S_LOGAND:
            break;

        case S_LOGOR:
            break;

        case S_LP:
            break;

        case S_LS:
            break;

        case S_LSHIFT:
            break;

        case S_LSTR:
            break;

        case S_MINUS:
            break;

        case S_MULT:
            break;

        case S_NE:
            break;

        case S_NEEDS:
            break;

         case S_NEG:
            break;

        case S_NEQV:
            break;

        case S_NOT:
            break;

        case S_PLUS:
            break;

        case S_PUTBYTE:
            break;

       case S_QUERY:
            break;

        case S_REM:
            break;

        case S_RES:
            break;

        case S_RSHIFT:
            break;

        case S_RSTACK:
            break;

        case S_RTAP:
            break;

        case S_RTRN:
            break;

        case S_RV:
            break;

        case S_SAVE:
            break;
            
        case S_SECTION:
            break;

        case S_SG:
            break;

        case S_SL:
            break;

        case S_SP:
            break;

        case S_STACK:
            n = rdn();
            comment("STACK %d", n);
            section(SEC_CODE);
            break;

        case S_STIND:
            break;

        case S_STORE:
            break;

        case S_SWITCHON:
            break;

        case S_TRUE:
            break;

        default:
            error("Unknown op %d", op);
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

    while (isupper(*fmt) || isdigit(*fmt) || *fmt == ':') {
        putchar(*fmt++);
    }
    if (*fmt) {
        putchar('\t');
        va_start(ap, fmt);
        vprintf(fmt, ap);
    }
    putchar('\n');
    va_end(ap);
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

    printf("; ");
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

static void section (int sec)
{
    if (currentSection != -1) {
        emit("\tendsection");
    }

    if (currentSection != sec) {
        switch (sec) {
            case SEC_DATA:
                emit("\tsection data");
                currentSection = sec;
                break;

            case SEC_CODE:
                emit("\tsection code");
                currentSection = sec;
                break;

            case SEC_BSS:
                emit("\tsection bss");
                currentSection = sec;
                break;

            default:
                fprintf(stderr, "Bad section requested.\n");
                break;
        }
    }
}