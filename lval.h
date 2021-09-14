#include <stdio.h>

#include "mpc.h"

enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_FUN,
    LVAL_SEXPR,
    LVAL_QEXPR,
};

enum {
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM,
};

// struct lval;
// struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval*(*lbuildtin)(lenv*, lval*);

struct lval {
    int type;
    long num;
    char* err;
    char* sym;
    lbuildtin fun;
    int count;
    struct lval** cell;
};

struct lenv {
    int count;
    char** syms;
    lval** vals;
};

char* ltype_name(int t);

lval* lval_num(long x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* x);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
void lval_del(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);
lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_eval(lenv* e, lval* v);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* buildtin_op(lenv* e, lval* a, char* op);
lval* buildtin_head(lenv* e, lval* a);
lval* buildtin_tail(lenv* e, lval* a);
lval* buildtin_list(lenv* e, lval* a);
lval* buildtin_eval(lenv* e, lval* a);
lval* lval_join(lval* x, lval* y);
lval* buildtin_join(lenv* e, lval* a);
lval* buildtin(lenv* e, lval* a, char* func);

lval* lval_fun(lbuildtin func);

lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);

lval* buildtin_def(lenv* e, lval* a);

lenv* lenv_new(void);
void lenv_del(lenv* e);

lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);

void lenv_add_buildtin(lenv* e, char* name, lbuildtin func);
void lenv_add_buildtins(lenv* e);

