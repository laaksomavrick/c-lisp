#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#include <editline/readline.h>

long eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}

long eval(mpc_ast_t* t) {
    
    //base case
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    //operator is always 2nd child as per our grammar
    char* op = t->children[1]->contents;

    //store the next expr
    long x = eval(t->children[2]);

    //iterate remaining children
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
    
}

int main(int argc, char** argv) {

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Clisp = mpc_new("clisp");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                     \
          number   : /-?[0-9]+/ ;                             \
          operator : '+' | '-' | '*' | '/' ;                  \
          expr     : <number> | '(' <operator> <expr>+ ')' ;  \
          clisp    : /^/ <operator> <expr>+ /$/ ;             \
        ",
        Number, Operator, Expr, Clisp);

    puts("c-lisp version 0.0.1");
    puts("Press CTRL+c to Exit\n");

    while (1) {

        char* input = readline("clisp> ");

        add_history(input);

        mpc_result_t r;

        if (mpc_parse("<stdin>", input, Clisp, &r)) {
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }

    mpc_cleanup(4, Number, Operator, Expr, Clisp);    

    return 0;

}