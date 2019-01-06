#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <vector>
#include "Parser.h"
#include "compiler.h"

using namespace std;

vector<ValueNode*> universalValueNodes;

void Parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

Token Parser::peek_ahead()
{
    Token t = lexer.GetToken();
    Token t1 = lexer.GetToken();
    lexer.UngetToken(t1);
    lexer.UngetToken(t);
    return t1;
}

ValueNode * getValueNode(string id)
{
    for(auto i : universalValueNodes)
    {
        if(i->name == id)
        {
            return i;
        }
    }
}

bool alreadyInUniversalValueNode(string id)
{
    for(auto i : universalValueNodes)
    {
        if(i->name == id)
        {
            return true;
        }
    }
    return false;
}

TokenType Parser::op()
{
    Token t = peek();

    if(t.token_type == PLUS)
    {
        Token tok = expect(PLUS);
        return tok.token_type;
    }
    else if(t.token_type == MINUS)
    {
        Token tok = expect(MINUS);
        return tok.token_type;
    }
    else if(t.token_type == MULT)
    {
        Token tok = expect(MULT);
        return tok.token_type;
    }
    else if(t.token_type == DIV)
    {
        Token tok = expect(DIV);
        return tok.token_type;
    }
    else
    {
        syntax_error();
    }
}

ValueNode * Parser::primary()
{
    Token t = peek();

    if(t.token_type == ID)
    {
        Token tok = expect(ID);
        return getValueNode(tok.lexeme);
    }
    else if(t.token_type == NUM)
    {
        Token tok = expect(NUM);

        struct ValueNode * valNode = new ValueNode;
        valNode->name = tok.lexeme;
        valNode->value = stoi(tok.lexeme);
        universalValueNodes.push_back(valNode);

        return valNode;
    }
    else
    {
        syntax_error();
    }
}

StatementNode Parser::expr()
{
    StatementNode s;
    s.type = ASSIGN_STMT;
    s.assign_stmt = new AssignmentStatement;
    s.assign_stmt->left_hand_side = NULL;
    s.assign_stmt->operand1 = primary();
    TokenType type = op();

    if(type == PLUS)
    {
        s.assign_stmt->op = OPERATOR_PLUS;
    }
    else if(type == MINUS)
    {
        s.assign_stmt->op = OPERATOR_MINUS;
    }
    else if(type == MULT)
    {
        s.assign_stmt->op = OPERATOR_MULT;
    }
    else if(type == DIV)
    {
        s.assign_stmt->op = OPERATOR_DIV;
    }
    s.assign_stmt->operand2 = primary();
    s.next = NULL;

    return s;
}

ConditionalOperatorType Parser::relop()
{
    Token t = peek();

    if(t.token_type == GREATER)
    {
        expect(GREATER);
        return CONDITION_GREATER;
    }
    else if(t.token_type == LESS)
    {
        expect(LESS);
        return CONDITION_LESS;
    }
    else if(t.token_type == NOTEQUAL)
    {
        expect(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    else
    {
        syntax_error();
    }
}

StatementNode Parser::condition()
{
    StatementNode cond;
    cond.type = IF_STMT;
    cond.if_stmt = new IfStatement;

    cond.if_stmt->condition_operand1 = primary();
    cond.if_stmt->condition_op = relop();
    cond.if_stmt->condition_operand2 = primary();

    return cond;
}

void Parser::id_list()
{
    Token t1 = peek();
    Token t2 = peek_ahead();

    if(t1.token_type == ID && t2.token_type == COMMA)
    {
        Token tok = expect(ID);

        struct ValueNode * valNode = new ValueNode;
        valNode->name = tok.lexeme;
        valNode->value = 0;
        universalValueNodes.push_back(valNode);

        expect(COMMA);
        id_list();
    }
    else if(t1.token_type == ID)
    {
        Token tok = expect(ID);

        struct ValueNode * valNode = new ValueNode;
        valNode->name = tok.lexeme;
        valNode->value = 0;
        universalValueNodes.push_back(valNode);
    }
    else
    {
        syntax_error();
    }
}

void Parser::var_section()
{
    id_list();
    expect(SEMICOLON);
}

struct StatementNode * Parser::forLoop_assign_stmt()
{
    Token tok = expect(ID);

    struct StatementNode * assignStmt = new StatementNode;
    assignStmt->type = ASSIGN_STMT;
    assignStmt->assign_stmt = new AssignmentStatement;

    if(alreadyInUniversalValueNode(tok.lexeme))
    {
        assignStmt->assign_stmt->left_hand_side = getValueNode(tok.lexeme);
    }
    else
    {
        struct ValueNode * newValNode = new ValueNode;
        newValNode->name = tok.lexeme;
        newValNode->value = 0;
        universalValueNodes.push_back(newValNode);

        assignStmt->assign_stmt->left_hand_side = getValueNode(tok.lexeme);
    }

    expect(EQUAL);

    Token t1 = peek();
    Token t2 = peek_ahead();

    if((t1.token_type == ID || t1.token_type == NUM )&& (t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV))
    {
        StatementNode tempStmtNode = expr();

        assignStmt->assign_stmt->op = tempStmtNode.assign_stmt->op;
        assignStmt->assign_stmt->operand1 = tempStmtNode.assign_stmt->operand1;
        assignStmt->assign_stmt->operand2 = tempStmtNode.assign_stmt->operand2;
        assignStmt->next = NULL;

        expect(SEMICOLON);
    }
    else if(t1.token_type == ID || t1.token_type == NUM)
    {
        ValueNode * valNode = primary();

        assignStmt->assign_stmt->op = OPERATOR_NONE;
        assignStmt->assign_stmt->operand1 = valNode;
        assignStmt->assign_stmt->operand2 = NULL;
        assignStmt->next = NULL;

        expect(SEMICOLON);
    }
    else
    {
        syntax_error();
    }

    return assignStmt;
}

struct StatementNode * Parser::assign_stmt()
{
    Token tok = expect(ID);

    struct StatementNode * assignStmt = new StatementNode;
    assignStmt->type = ASSIGN_STMT;
    assignStmt->assign_stmt = new AssignmentStatement;
    assignStmt->assign_stmt->left_hand_side = getValueNode(tok.lexeme);

    expect(EQUAL);

    Token t1 = peek();
    Token t2 = peek_ahead();

    if((t1.token_type == ID || t1.token_type == NUM )&& (t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV))
    {
        StatementNode tempStmtNode = expr();

        assignStmt->assign_stmt->op = tempStmtNode.assign_stmt->op;
        assignStmt->assign_stmt->operand1 = tempStmtNode.assign_stmt->operand1;
        assignStmt->assign_stmt->operand2 = tempStmtNode.assign_stmt->operand2;
        assignStmt->next = NULL;

        expect(SEMICOLON);
    }
    else if(t1.token_type == ID || t1.token_type == NUM)
    {
        ValueNode * valNode = primary();

        assignStmt->assign_stmt->op = OPERATOR_NONE;
        assignStmt->assign_stmt->operand1 = valNode;
        assignStmt->assign_stmt->operand2 = NULL;
        assignStmt->next = NULL;

        expect(SEMICOLON);
    }
    else
    {
        syntax_error();
    }

    return assignStmt;
}

struct StatementNode * Parser::print_stmt()
{
    expect(PRINT);
    Token tok = expect(ID);

    struct StatementNode * prntStmt = new StatementNode;
    prntStmt->type = PRINT_STMT;
    prntStmt->print_stmt = new PrintStatement;
    prntStmt->print_stmt->id = getValueNode(tok.lexeme);
    prntStmt->next = NULL;

    expect(SEMICOLON);

    return prntStmt;
}

struct StatementNode * Parser::while_stmt()
{
    expect(WHILE);
    struct StatementNode * whileNode = new StatementNode;
    whileNode->type = IF_STMT;
    whileNode->if_stmt = new IfStatement;

    StatementNode temp = condition();

    whileNode->if_stmt->condition_op = temp.if_stmt->condition_op;
    whileNode->if_stmt->condition_operand1 = temp.if_stmt->condition_operand1;
    whileNode->if_stmt->condition_operand2 = temp.if_stmt->condition_operand2;

    whileNode->if_stmt->true_branch = body();

    struct StatementNode * whileNOOPStmt = new StatementNode;
    whileNOOPStmt->type = NOOP_STMT;
    whileNOOPStmt->next = NULL;

    struct StatementNode * whileGOTOStmt = new StatementNode;
    whileGOTOStmt->type = GOTO_STMT;
    whileGOTOStmt->goto_stmt = new GotoStatement;
    whileGOTOStmt->goto_stmt->target = whileNode;
    whileGOTOStmt->next = whileNOOPStmt;

    struct StatementNode * dummyWhileNode = whileNode->if_stmt->true_branch;
    while(dummyWhileNode->next != NULL)
    {
        dummyWhileNode = dummyWhileNode->next;
    }
    dummyWhileNode->next = whileGOTOStmt;

    whileNode->if_stmt->false_branch = whileNOOPStmt;
    whileNode->next = whileNOOPStmt;

    return whileNode;
}

struct StatementNode * Parser::if_stmt()
{
    expect(IF);
    struct StatementNode * ifNode = new StatementNode;
    ifNode->type = IF_STMT;
    ifNode->if_stmt = new IfStatement;

    StatementNode temp = condition();

    ifNode->if_stmt->condition_op = temp.if_stmt->condition_op;
    ifNode->if_stmt->condition_operand1 = temp.if_stmt->condition_operand1;
    ifNode->if_stmt->condition_operand2 = temp.if_stmt->condition_operand2;

    ifNode->if_stmt->true_branch = body();

    struct StatementNode * noOp_stmt = new StatementNode;
    noOp_stmt->type = NOOP_STMT;
    noOp_stmt->next = NULL;

    //Set end of true branch to point to NOOP
    struct StatementNode * dummyNode = ifNode->if_stmt->true_branch;
    while(dummyNode->next != NULL)
    {
        dummyNode = dummyNode->next;
    }
    dummyNode->next = noOp_stmt;

    ifNode->if_stmt->false_branch = noOp_stmt;
    ifNode->next = noOp_stmt;

    return ifNode;
}

struct StatementNode * Parser::default_case()
{
    expect(DEFAULT);
    expect(COLON);
    struct StatementNode * defaultStmt = body();
    defaultStmt->next = NULL;

    return defaultStmt;
}

struct StatementNode * Parser::case_(string id)
{
    expect(CASE);

    Token tok = expect(NUM);

    //Create a value node(if not already in universalList) for the NUM and push it to universalList
    if(!alreadyInUniversalValueNode(tok.lexeme))
    {
        struct ValueNode * valNode = new ValueNode;
        valNode->name = tok.lexeme;
        valNode->value = stoi(tok.lexeme);
        universalValueNodes.push_back(valNode);
    }

    expect(COLON);

    struct StatementNode * _case = new StatementNode;
    _case->type = IF_STMT;
    _case->if_stmt = new IfStatement;

    _case->if_stmt->condition_op = CONDITION_NOTEQUAL;
    _case->if_stmt->condition_operand1 = getValueNode(id);
    _case->if_stmt->condition_operand2 = getValueNode(tok.lexeme);

    _case->if_stmt->false_branch = body();

    struct StatementNode * switchGOTOStmt = new StatementNode;
    switchGOTOStmt->type = GOTO_STMT;
    switchGOTOStmt->goto_stmt = new GotoStatement;
    //switchGOTOStmt->goto_stmt->target = switchNOOPStmt;
    switchGOTOStmt->next = NULL;

    struct StatementNode * switchCaseNOOPStmt = new StatementNode;
    switchCaseNOOPStmt->type = NOOP_STMT;
    switchCaseNOOPStmt->next = NULL;

    //Set end of true branch to point to GOTO
    struct StatementNode * dummyNode = _case->if_stmt->false_branch;
    while(dummyNode->next != NULL)
    {
        dummyNode = dummyNode->next;
    }
    dummyNode->next = switchGOTOStmt;

    _case->if_stmt->true_branch = switchCaseNOOPStmt;
    _case->next = switchCaseNOOPStmt;

    return _case;
}

struct StatementNode * Parser::case_list(string var)
{
    struct StatementNode * switchCaseList;
    struct StatementNode * switchCase;

    switchCase = case_(var);

    Token t = peek();
    if(t.token_type == CASE)
    {
        switchCaseList = case_list(var);
        switchCase->next->next = switchCaseList;

        return switchCase;
    }
    else if(t.token_type == DEFAULT)
    {
        switchCaseList = default_case();
        //switchCaseList->next = switchNOOPStmt;
        switchCase->next->next = switchCaseList;
        return switchCase;
    }
    else
    {
        return switchCase;
    }
}

struct StatementNode * Parser::switch_stmt()
{
    expect(SWITCH);
    Token tok = expect(ID);
    expect(LBRACE);

    struct StatementNode * switchCase = case_list(tok.lexeme);

    struct StatementNode * switchNOOPStmt = new StatementNode;
    switchNOOPStmt->type = NOOP_STMT;
    switchNOOPStmt->next = NULL;

    struct StatementNode * setIfGOTO = switchCase;
    while(setIfGOTO->next != NULL)
    {
        if(setIfGOTO->type == IF_STMT)
        {
            struct StatementNode * dummyNode = setIfGOTO->if_stmt->false_branch;
            while(dummyNode->next != NULL)
            {
                dummyNode = dummyNode->next;
            }
            dummyNode->goto_stmt->target = switchNOOPStmt;
            dummyNode->next = setIfGOTO->if_stmt->true_branch;
        }
        setIfGOTO = setIfGOTO->next;
    }

    struct StatementNode * checkLast = switchCase;
    while(checkLast->next != NULL)
    {
        checkLast = checkLast->next;
    }
    checkLast->next = switchNOOPStmt;

    expect(RBRACE);

    return switchCase;
}

struct StatementNode * Parser::for_stmt()
{
    struct StatementNode * forWhileNode = new StatementNode;
    forWhileNode->type = IF_STMT;
    forWhileNode->if_stmt = new IfStatement;

    expect(FOR);
    expect(LPAREN);

    struct StatementNode * assignStmt1 = forLoop_assign_stmt();
    assignStmt1->next = forWhileNode;

    StatementNode temp = condition();
    forWhileNode->if_stmt->condition_op = temp.if_stmt->condition_op;
    forWhileNode->if_stmt->condition_operand1 = temp.if_stmt->condition_operand1;
    forWhileNode->if_stmt->condition_operand2 = temp.if_stmt->condition_operand2;

    expect(SEMICOLON);
    struct StatementNode * assignStmt2 = forLoop_assign_stmt();
    expect(RPAREN);

    forWhileNode->if_stmt->true_branch = body();

    struct StatementNode * forWhileNOOPStmt = new StatementNode;
    forWhileNOOPStmt->type = NOOP_STMT;
    forWhileNOOPStmt->next = NULL;

    struct StatementNode * forWhileGOTOStmt = new StatementNode;
    forWhileGOTOStmt->type = GOTO_STMT;
    forWhileGOTOStmt->goto_stmt = new GotoStatement;
    forWhileGOTOStmt->goto_stmt->target = forWhileNode;
    forWhileGOTOStmt->next = forWhileNOOPStmt;

    struct StatementNode * dummyForWhileNode = forWhileNode->if_stmt->true_branch;
    while(dummyForWhileNode->next != NULL)
    {
        dummyForWhileNode = dummyForWhileNode->next;
    }
    dummyForWhileNode->next = assignStmt2;

    assignStmt2->next = forWhileGOTOStmt;

    forWhileNode->if_stmt->false_branch = forWhileNOOPStmt;
    forWhileNode->next = forWhileNOOPStmt;

    return assignStmt1;
}

struct StatementNode * Parser::stmt()
{
    struct StatementNode * stmtNode;
    Token t = peek();

    if(t.token_type == ID)
    {
        stmtNode = assign_stmt();
    }
    else if(t.token_type == PRINT)
    {
        stmtNode = print_stmt();
    }
    else if(t.token_type == WHILE)
    {
        stmtNode = while_stmt();
    }
    else if(t.token_type == IF)
    {
        stmtNode = if_stmt();
    }
    else if(t.token_type == SWITCH)
    {
        stmtNode = switch_stmt();
    }
    else if(t.token_type == FOR)
    {
        stmtNode = for_stmt();
    }
    else
    {
        syntax_error();
    }

    return stmtNode;
}

struct StatementNode * Parser::stmt_list()
{
    struct StatementNode * stmtListNode;
    struct StatementNode * stmtNode;

    stmtNode = stmt();

    Token t = peek();
    if(t.token_type == ID || t.token_type == PRINT || t.token_type == WHILE || t.token_type == IF || t.token_type == SWITCH || t.token_type == FOR)
    {
        stmtListNode = stmt_list();
        if(stmtNode->type == IF_STMT)
        {
            if(stmtNode->if_stmt->true_branch->type == NOOP_STMT)
            {
                struct StatementNode * switchEnd = stmtNode;
                while(switchEnd->next != NULL)
                {
                    switchEnd = switchEnd->next;
                }
                switchEnd->next = stmtListNode;
            }
            else
            {
                stmtNode->next->next = stmtListNode;
            }
        }
        else
        {
            if(stmtNode->next != NULL)
            {
                if(stmtNode->type == ASSIGN_STMT && stmtNode->next->type == IF_STMT)
                {
                    struct StatementNode * dummy = stmtNode->next->if_stmt->true_branch;
                    while(dummy->next->type != GOTO_STMT)
                    {
                        dummy = dummy->next;
                    }
                    if(dummy->next->type == GOTO_STMT)
                    {
                        stmtNode->next->next->next = stmtListNode;
                    }
                    else
                    {
                        stmtNode->next = stmtListNode;
                    }
                }
            }
            else
            {
                stmtNode->next = stmtListNode;
            }
        }
        return stmtNode;
    }
    else
    {
       return stmtNode;
    }
}

struct StatementNode * Parser::body()
{
    expect(LBRACE);
    struct StatementNode * bodyNode = stmt_list();
    expect(RBRACE);

    return bodyNode;
}

struct StatementNode * Parser::program()
{
    var_section();
    struct StatementNode * programNode = body();

    return programNode;
}

struct StatementNode * parse_generate_intermediate_representation()
{
    Parser p;
    struct StatementNode * finalNode = p.program();

    return finalNode;
}
