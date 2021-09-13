#include "lval.h"

#define LASSERT(args, cond, err)    \
    if (!(cond)) { lval_del(args); return lval_err(err); }

lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(x) + 1);
    strcpy(v->err, x);
    return v;
}

lval* lval_sym(char* x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(sizeof(x) + 1);
    strcpy(v->sym, x);
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*)*v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_read(mpc_ast_t* t) {

    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);

    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        if (i != (v->count-1)) {
            putchar(' ');
        }
    }

    putchar(close);
}

void lval_print(lval* v) {
    switch(v->type) {
        case LVAL_NUM:      printf("%li", v->num);           break;
        case LVAL_ERR:      printf("Error: %s", v->err);     break;
        case LVAL_SYM:      printf("%s", v->sym);            break;
        case LVAL_SEXPR:    lval_expr_print(v, '(', ')');    break;
        case LVAL_QEXPR:    lval_expr_print(v, '{', '}');    break;
    }
}

void lval_println(lval* v) {
    lval_print(v);
    putchar('\n');
}

lval* lval_eval_sexpr(lval* v) {

    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    if (v->count == 0) return v;
    if (v->count == 1) return lval_take(v, 0);

    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression Dose not start with symbol!");
    }

    lval* result = buildtin(v, f->sym);
    lval_del(f);

    return result;
}

lval* lval_eval(lval* v) {
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(v);
    }
    return v;
}

lval* lval_pop(lval* v, int i) {
    lval* x = v->cell[i];
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*)*(v->count-i-1));
    v->count--;
    v->cell = realloc(v->cell, sizeof(lval*)*v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* buildtin_op(lval* a, char* op) {

    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_NUM, "Cannot operate on non-number!");
    }

    lval* x = lval_pop(a, 0);
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    while (a->count > 0) {
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) { 
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division By Zero!");
                break;
            }
            x->num /= y->num; 
        }

        lval_del(y);
    }

    lval_del(a);

    return x;
}

lval* buildtin_head(lval* a) {

    LASSERT(a, a->count == 1,                   "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,  "Function 'head' passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0,          "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }

    return v;
}

lval* buildtin_tail(lval* a) {

    LASSERT(a, a->count == 1,                   "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,  "Function 'head' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0,          "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);

    lval_del(lval_pop(v, 0));

    return v;
}

lval* buildtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval* buildtin_eval(lval* a) {
    LASSERT(a, a->count == 1,                   "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,  "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval* lval_join(lval* x, lval* y) {

    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);

    return x;
}

lval* buildtin_join(lval* a) {

    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type!");
    }

    lval* x = lval_pop(a, 0);

    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);

    return x;
}

lval* buildtin(lval* a, char* func) {
    if (strcmp("list", func) == 0) { return buildtin_list(a); }
    if (strcmp("join", func) == 0) { return buildtin_join(a); }
    if (strcmp("head", func) == 0) { return buildtin_head(a); }
    if (strcmp("tail", func) == 0) { return buildtin_tail(a); }
    if (strcmp("eval", func) == 0) { return buildtin_eval(a); }
    if (strstr("+-*/", func)) { return buildtin_op(a, func); }
    lval_del(a);
    return lval_err("Unknow Function!");
}
