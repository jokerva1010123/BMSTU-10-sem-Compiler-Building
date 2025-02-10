from lexer import Lexer, Token


class Parser:
    def __init__(self, lexer: Lexer) -> None:
        self.lexer = lexer
        self.rpn_output = []  # Выходной список для RPN

    def parse(self):
        self.expression()
    def expression(self):
        self.s_expression()
        self.expression_prime()

    def expression_prime(self):
        if self.lexer.current_token.type == 'RELATION_OP':
            self.relation_operation()
            self.s_expression()

    def s_expression(self):
        if self.lexer.current_token.value in ['+', '-']:
            sign = self.lexer.current_token.value
            self.lexer.advance()

        self.term()
        self.term_rest()

    def term_rest(self):
        self.s_expression_prime()

    def s_expression_prime(self):
        if self.lexer.current_token.type == 'ADD_OP':
            op = self.addition_operation()
            self.term()
            self.rpn_output.append(op.value)
            self.s_expression_prime()

    def term(self):
        self.factor()
        self.term_prime()

    def term_prime(self):
        if self.lexer.current_token.type == 'MULT_OP':
            op = self.multiplication_operation()
            self.factor()
            self.rpn_output.append(op.value)
            self.term_prime()

    def factor(self):
        if self.lexer.current_token.type == 'IDENT':
            self.rpn_output.append(self.identifier().value)
        elif self.lexer.current_token.type == 'CONST':
            self.rpn_output.append(self.constant().value)
        elif self.lexer.current_token.type == 'LPAREN':
            self.lexer.advance()
            self.s_expression()
            self.lexer.expect('RPAREN')
        elif self.lexer.current_token.type == 'NOT':
            kw = self.keyword()
            self.factor()
            self.rpn_output.append(kw.value)

    def relation_operation(self):
        terminal = self.lexer.expect('RELATION_OP')
        return terminal

    def addition_operation(self):
        terminal = self.lexer.expect('ADD_OP')
        return terminal

    def multiplication_operation(self):
        terminal = self.lexer.expect('MULT_OP')
        return terminal

    def keyword(self):
        terminal = self.lexer.expect('KEYWORD')
        return terminal

    def constant(self):
        terminal = self.lexer.expect('CONST')
        return terminal

    def identifier(self):
        terminal = self.lexer.expect('IDENT')
        return terminal


#Парсим:
#<выражение> -> <простое выражение> <выражение'>
#<выражение'> -> <операция отношения> <простое выражение> | ε
#<простое выражение> -> <терм> <продолжение терма> | <знак> <терм> <продолжение терма>
#<продолжение терма> -> <простое выражение'> | ε
#<простое выражение'> -> <операция типа сложения> <терм> <простое выражение'> | ε
#<терм> -> <фактор> <терм'>
#<терм'> -> <операция типа умножения> <фактор> <терм'> | ε
#<фактор> -> <идентификатор> | <константа> | ( <простое выражение> ) | not <фактор>
#<операция отношения> -> = | <> | < | <= | > | >=
#<знак> -> + | -
#<операция типа сложения> -> + | - | or
#<операция типа умножения> -> * | / | div | mod | and

tests = [
    '''a + b + c * d - e''',
    '''a + (b + c) * d - e''',
    '''3 + 4 + 2 / 1 - 5  / 2 - 3''',
    '''a + b''',
    '''a - b * c''',
    '''(1 * 2 * 3 * 4 * 5) / 5''',
    '''1 * 2 * 3 * 4 * 5 / 5'''
]

for test in tests:
    lexer = Lexer(test)
    parser = Parser(lexer)
    parser.parse()
    print(' '.join([token.value for token in lexer.tokens]) + ' --> ' + ' '.join(parser.rpn_output))

