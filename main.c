#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "lval.h"

int main(int argc, char** argv) {

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number      :   /-?[0-9]+/;                         \
        symbol      :   /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;   \
        sexpr       :   '('<expr>*')';                      \
        qexpr       :   '{'<expr>*'}';                      \
        expr        :   <number>|<symbol>|<sexpr>|<qexpr>;  \
        lispy       :   /^/<expr>*/$/;                      \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    puts("Lispy Version 0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    lenv* e = lenv_new();
    lenv_add_buildtins(e);

    

    while (1) {
        char* input = readline("lispy> ");
        add_history(input);
        
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            lval* tmp = lval_read(r.output);
            lval* result = lval_eval(e, tmp);
            lval_println(result);
            lval_del(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}