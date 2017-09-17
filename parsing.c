#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#include <editline/readline.h>

//struct representing possible values
typedef struct {
    int type;
    long num;
    int err;
} lval;

//consts for lval type
enum { LVAL_NUM, LVAL_ERR };

//consts for error type
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

//lval generators
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

//lval convenience print

void lval_print(lval v) {

    switch(v.type) {

        case LVAL_NUM: printf("%li", v.num); break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) {
                printf("ERROR! Division by zero");
            } else if (v.err == LERR_BAD_OP) {
                printf("ERROR! Invalid operation");                
            } else if (v.err == LERR_BAD_NUM) {
                printf("ERROR! Invalid number");                
            }
            break;
    }

}

void lval_println(lval v) {
    lval_print(v); putchar('\n');
}

lval eval_op(lval x, char* op, lval y) {

    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }    

    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) { 
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    
    //base case
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);    
    }

    //operator is always 2nd child as per our grammar
    char* op = t->children[1]->contents;

    //store the next expr
    lval x = eval(t->children[2]);

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
            lval result = eval(r.output);
            lval_println(result);
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