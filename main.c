#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "mpc.h"
#include "lval.h"


int main(int argc, char** argv) {

    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    String = mpc_new("string");
    Comment = mpc_new("comment");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    Expr = mpc_new("expr");
    Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                   \
        number      :   /-?[0-9]+/;                                     \
        symbol      :   /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;               \
        string      :   /\"(\\\\.|[^\"])*\"/;                           \
        comment     :   /;[^\\r\\n]*/;                                  \
        sexpr       :   '('<expr>*')';                                  \
        qexpr       :   '{'<expr>*'}';                                  \
        expr        :   <number>|<symbol>|<sexpr>|<qexpr>|<string>|<comment>;     \
        lispy       :   /^/<expr>*/$/;                                  \
    ",
    Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

    lenv* e = lenv_new();
    lenv_add_buildtins(e);

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval* x = buildtin_load(e, args);

            lval_println(x);
            if (x->type == LVAL_ERR) { lval_println(x); }
            lval_del(x);
        }
    } else {
        puts("Lispy Version 0.0.0.1");
        puts("Press Ctrl+c to Exit\n");



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
    }
    
    lenv_del(e);

    mpc_cleanup(8, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}