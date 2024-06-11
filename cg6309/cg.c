/* Copyright (c) 2012 Robert Nordier. All rights reserved. */

/* BCPL compiler backend: generate x86 assembler from OCODE. */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oc.h"

#if !defined(true)
# define true 1
# define false 0
#endif

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

static const char *typeNames[] = {
    "X_R", "X_P", "X_G", "X_L",
    "X_N", "X_LP", "X_LG", "X_LL"
};

/* 6309 instructions in table */
#define X_LD   0
#define X_ST   1
#define X_LEA  2
#define X_BRA  3
#define X_CALL 4
//#define X_IMUL 5
//#define X_IDIV 6
#define X_SUB  7
#define X_CMP  8
#define X_ADD  9
#define X_NEG  10
#define X_NOT  11
#define X_AND  12
#define X_OR   13
#define X_XOR  14

typedef struct inst_s {
    int instNum;
    const char *mnemonic;
    int numArgs;
    int isEax;
    int isJump;
} instruction;

instruction instructions[] = {
    {X_LD,   "ld",  2, false, false},
    {X_ST,   "st",  2, false, false},
    {X_LEA,  "lea", 2, true,  false},
    {X_BRA,  "bra", 1, false, true},
    {X_CALL, "bsr", 1, false, false},
    {X_SUB,  "sub", 2, false, false},
    {X_CMP,  "cmp", 2, false, false},
    {X_ADD,  "add", 2, false, false},
    {X_NEG,  "neg", 1, false, false},
    {X_NOT,  "not", 1, false, false},
    {X_AND,  "and", 2, false, false},
    {X_OR,   "or",  2, false, false},
    {X_XOR,  "xor", 2, false, false},
    {-1, "", 0, 0, 0}
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
static int lp;             /* load stack pointer */
static loadStackItem loadStack[2]; /* Load stack */

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
static int findInstruction(int);

static void defdata(int, int);
static void outdata(int, int);
static void dumpLoadStack();


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

    int numPops = 0;        // number of pops (0,1,2)
    int numPushes = 0;      // number of pushes (0,1,2 - set to n; 3 - set to n+)
    int s3 = 0;             // ?????

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

        numPops = optab1(op);
        numPushes = optab2(op);
        s3 = optab3(op);

        comment("sn=%d, numPops=%d, numPushes=%d, s3=%d", sn, numPops, numPushes, s3);

        if (s3 <= 7) {
            force(numPops);
            if (s3) {
                loadreg(0, s3 == 1 ? 0 : s3 == 2 ? loadStack[0].type != X_N : 1);
            }

            if (s3 >= 4) {
                loadreg(1, s3 == 4 ? 0 : s3 == 5 ? loadStack[1].type == X_N : 1);
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
            emit("cmpd\t,x");
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
            emit("bne\tL%d", labNum);
            break;

        case S_JT:
            labNum = rdn();
            comment("JT L%d", labNum);
            emit("beq\tL%d", labNum);
            break;

        case S_JUMP:
            labNum = rdn();
            comment("JUMP L%d", labNum);
            emit("bra\tL%d", labNum);
            break;

        case S_LAB:
            labNum = rdn();
            comment("LAB L%d", labNum);
            emit("L%d:", labNum);
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
        sp = numPushes & 2 ? sn : sp - numPops;
        if (numPushes & 1)
            sp++;
        
        /* adjust load stack pointer */
        if (s3 <= 7) {
            lp = numPushes & 1;
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

    if (neg) {
        n = -n;
    }

    //comment("rdn() -> %d", n);

    return n;
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


static void load(int targetType, int d)
{
    switch (targetType) {
        case X_R:
            break;
            
        case X_P:
            emit("leax\t%d,s", d*2);
            break;
            
        case X_G:
            break;
            
        case X_L:
            break;
            
        case X_N:
            emit("ldd\t#%d", d);
            emit("std\t%d,s", sn*2);
            break;
            
        case X_LP:
            break;
            
        case X_LG:
            break;
            
        case X_LL:
            break;

        default:
            error("Bad target type %d in load()", targetType);
            break;            
    }
    // comment("load(%d, %d): lp=%d", t, d, lp);
    // assert(lp >= 0 && lp <= 2);
    // if (lp == 2) {  
    //     // Make room
    //     force(1);
    // }

    // loadStack[lp].type = t;
    // loadStack[lp].data = d;
}

static void save(int k, int v)
{
    // comment("save(%d, %d)\n", k, v);
    code(X_ST, k, v, loadStack[0].type, loadStack[0].data);
}

/*
 * I think this code is about making some room in the load stack by
 * generating code for one or more of it's entries
 */
static void force(int targetLp)
{
    // comment("force(%d): lp=%d, sp=%d", st, lp, st);
    assert(lp >= 0 && lp <= 2);
    assert(targetLp >= 0 && targetLp <= 2);

    // Nothing to do - we are at the target 
    if (lp == targetLp) {
        return;
    }

    if (lp == 0) {
        loadStack[0].type = X_R;
        loadStack[0].data = 0;
        code(X_LD, loadStack[0].type, loadStack[0].data, X_P, sp - 1);
        lp = 1;
        if (lp == targetLp) {
            return;
        }
    }

    assert(lp != 0);

    if (targetLp == 2) {
        assert(loadStack[0].type == X_R && loadStack[0].data == 0);

        emit("movvvv %%eax,%%ecx");
        code(X_LD, loadStack[0].type, loadStack[0].data, X_P, sp - 2);
        loadStack[1].type = X_R;
        loadStack[1].data = 1;
        lp = 2;
    } else {
        comment("wooo!");

        do {
            loadreg(0, loadStack[0].type != X_N);
            code(X_ST, X_P, sp - lp, loadStack[0].type, loadStack[0].data);
            if (lp == 2) {
                loadStack[0].type = loadStack[1].type;
                loadStack[0].data = loadStack[1].type == X_R ? 0 : loadStack[1].data;
            }
        } while (--lp != targetLp);
    }
}

static void loadreg(int i, int must)
{
    int t, p;

    // comment("loadreg(%d, %d)", i, must);

    t = loadStack[i].type;
    if (t == X_R) {
        comment("  nope");
        return;
    }

    p = t == X_LP || t == X_LG || t == X_LL;
    if (p || must) {
        comment("  must!");
        code(p && t != X_LL ? X_LEA : X_LD, t, loadStack[i].data, X_R, i);
        if (p) {
            emit("shr $2,%%%s", reg[i]);
        }
        loadStack[i].type = X_R;
        loadStack[i].data = i;
    }
    else {
        comment("  don't have to");
    }
}

static void codex(int xi)
{
    code(xi, loadStack[0].type, loadStack[0].data, loadStack[1].type, loadStack[1].data);
}

static void code(int xi, ...)
{
    char buf[64], *s;
    int typ[2], dat[2];
    va_list ap;
    int cj, i1, na, i, t, d;
    instruction *inst;
    int instNum;
    
    // comment("code(%d, ...)", xi);

    instNum = findInstruction(xi);
    if (instNum == -1) {
        error("Much weirdness in code(%d, ....)", xi);
    }
  
    inst = &instructions[instNum];
    cj = inst->isJump;
    i1 = inst->isEax;
    na = inst->numArgs;
    // comment("    code(): xi=%d, inst=%s, na=%d, cj=%d, i1=%d", xi, inst->mnemonic, na, cj, i1);

    va_start(ap, xi);
    for (i = 0; i < na; i++)
    {
        typ[i] = va_arg(ap, int);
        dat[i] = va_arg(ap, int);

        // comment("  %d: %d, %d", i, typ[i], dat[i]);
    }
    s = buf;

    if (inst->numArgs == 1) {
        printf("code() with one arg: (%s, %d)\n", typeNames[typ[0]], dat[0]);
    }
    else if (inst->numArgs == 2) {
        printf("code() with two args: (%s, %d), (%s, %d)\n", typeNames[typ[0]], dat[0], typeNames[typ[1]], dat[1]);
    }
    else {
        error("Error in code(%d, ...): numargs set to impossible value %d", inst->numArgs);
    }
    // s += sprintf(s, "%s", inst->mnemonic);

    // for (i = na - 1; i >= i1; i--)
    // {
    //     if (i != na-1)
    //         *s++ = '\t';
    //     //*s++ = i == na - 1 ? ' ' : ',';
    //     t = typ[i];
    //     d = dat[i];
    //     // comment("    i=%d, t=%d, d=%d", i, t, d);
        
    //     if (t <= 3) {
    //         if (cj) {
    //             *s++ = '*';
    //         }
    //     } else if (t != X_LP && t != X_LG && !cj) {
    //         *s++ = '#';
    //     }
    //     switch (t)
    //     {
    //     case X_R:
    //         sprintf(s, "%s", reg[d]);
    //         break;
    //     case X_P:
    //     case X_LP:
    //         sprintf(s, "%d,s", d * BytesPerWord);
    //         break;
    //     case X_G:
    //     case X_LG:
    //         sprintf(s, "%d,u", d * BytesPerWord);
    //         break;
    //     case X_L:
    //     case X_LL:
    //         sprintf(s, "%s", label(d));
    //         break;
    //     case X_N:
    //         sprintf(s, "%d", d);
    //         break;
    //     }
    //     while (*s) {
    //         s++;
    //     }
    // }
    // va_end(ap);
    // emit("%s", buf);
}

static int findInstruction(int instructionNumber)
{
    int i;

    for (i=0; instructions[i].instNum != -1; i++) {
        if (instructions[i].instNum == instructionNumber) {
            return i;
        }
    }

    return -1;
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

static void dumpLoadStackItem(int i)
{
    comment("  item@%d", i);

    switch(loadStack[i].type) {
        case X_R:
            comment("    type: register, data=%d", loadStack[i].data);
            break;

        case X_P:
            comment("    type: X_P, data=%d", loadStack[i].data);
            break;

        case X_G:
            comment("    type: global, data=%d", loadStack[i].data);
            break;
            
        case X_L:
            comment("    type: X_L, data=%d", loadStack[i].data);
            break;
            
        case X_N:
            comment("    type: number, data=%d", loadStack[i].data);
            break;
            
        case X_LP:
            comment("    type: X_LP, data=%d", loadStack[i].data);
            break;
            
        case X_LG:
            comment("    type: X_LG, data=%d", loadStack[i].data);
            break;
            
        case X_LL:
            comment("    type: X_LL, data=%d", loadStack[i].data);
            break;
            
        default:
            comment("    type: unknown (%d), data=%d", loadStack[i].type, loadStack[i].data);
            break;
    }
}

static void dumpLoadStack()
{
    int i;

    comment("-----------------------------");
    comment("Load Stack:");
    comment("  lp=%d", lp);

    for (i=0; i<lp; i++) {
        dumpLoadStackItem(i);
    }
    comment("-----------------------------");
}