#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <set>
#include <vector>
#include "compiler.h"
#include "lexer.h"

using namespace std;

class Parser
{
private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
    Token peek_ahead();

    struct StatementNode * body();
    struct StatementNode * stmt_list();
    struct StatementNode * stmt();
    struct StatementNode * for_stmt();
    struct StatementNode * switch_stmt();
    struct StatementNode * if_stmt();
    struct StatementNode * while_stmt();
    struct StatementNode *  print_stmt();
    struct StatementNode *  assign_stmt();
    void var_section();
    void id_list();
    StatementNode condition();
    ConditionalOperatorType relop();
    StatementNode expr();
    ValueNode *  primary();
    TokenType op();
    struct StatementNode * case_list(string var);
    struct StatementNode * case_(string id);
    struct StatementNode * default_case();
    struct StatementNode * forLoop_assign_stmt();

public:
    struct StatementNode * program();
};

#endif //PARSER_H
