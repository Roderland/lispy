#include <stdio.h>

#include "mpc.h"

enum {
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_SEXPR,
    LVAL_QEXPR,
};

enum {
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM,
};

typedef struct lval {
    int type;
    long num;
    char* err;
    char* sym;
    int count;
    struct lval** cell;
} lval;

lval* lval_num(long x);
lval* lval_err(char* x);
lval* lval_sym(char* x);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
void lval_del(lval* v);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_println(lval* v);
lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* buildtin_op(lval* a, char* op);
lval* buildtin_head(lval* a);
lval* buildtin_tail(lval* a);
lval* buildtin_list(lval* a);
lval* buildtin_eval(lval* a);
lval* lval_join(lval* x, lval* y);
lval* buildtin_join(lval* a);
lval* buildtin(lval* a, char* func);