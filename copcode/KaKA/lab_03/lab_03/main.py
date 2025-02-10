from lexer import Lexer, Token
from tree import TreeNode



class Parser:

    def __init__(self, lexer: Lexer) -> None:
        self.lexer = lexer

    def parse(self) -> TreeNode:
        return self.program()

    def program(self) -> TreeNode:
        '''
        <программа> -> <блок>
        '''
        #print('Парсим программу')
        tree = TreeNode('<программа>')
        tree.add_child(self.block())
        return tree

    def block(self) -> TreeNode:
        '''
        <блок> -> { <список операторов> }
        '''
        #print('Парсим блок')
        block = TreeNode('<блок>')

        lbrace = self.lexer.expect('LBRACE')
        block.add_child(TreeNode.from_token(lbrace))

        ops = self.operator_list()
        block.add_child(ops)

        rbrace = self.lexer.expect('RBRACE')
        block.add_child(TreeNode.from_token(rbrace))

        return block

    def operator_list(self) -> TreeNode:
        '''
        <список операторов> -> <оператор> <хвост>
        '''
        #print('Парсим список операторов')
        oplist = TreeNode('<список операторов>')

        operator = self.operator()
        oplist.add_child(operator)

        tail = self.tail()
        if tail: oplist.add_child(tail)

        return oplist

    def operator(self) -> TreeNode:
        '''
        <оператор> -> <идентификатор> = <выражение> | <блок>
        '''
        #print('Парсим оператор')
        operator = TreeNode('оператор')
        if self.lexer.current_token.type == 'IDENT':
            ident = self.identifier()
            operator.add_child(ident)

            assign = self.lexer.expect('ASSIGN')
            operator.add_child(TreeNode.from_token(assign))

            expression = self.expression()
            operator.add_child(expression)
            return operator
        elif self.lexer.current_token.type == 'LBRACE':
            #print('Распознан блок')
            return self.block()
        else:
            raise Exception(f'Некорректный токен: {self.lexer.current_token.type}')

    def tail(self) -> TreeNode | None:
        '''
        Было: <хвост> -> ; <оператор> <хвост> | ε
        Стало: <хвост> -> ; <оператор> <хвост> | ε | ; <хвост>
        '''
        tail = TreeNode('<хвост>')
        if self.lexer.current_token.type == 'SEMICOLON':
            semicolon = self.lexer.expect('SEMICOLON')
            tail.add_child(TreeNode.from_token(semicolon))
            if self.lexer.current_token.type == 'SEMICOLON':
                rec_tail = self.tail()
                if rec_tail: tail.add_child(rec_tail)
                return tail

            if self.lexer.current_token.type == 'RBRACE':
                return tail

            operator = self.operator()
            tail.add_child(operator)

            rec_tail = self.tail()
            if rec_tail: tail.add_child(rec_tail)

            return tail

    def expression(self) -> TreeNode:
        '''
        <выражение> -> <простое выражение> <выражение'>
        '''
        #print('Парсим выражение')
        expression = TreeNode('<выражение>')
        s_expression = self.s_expression()
        expression.add_child(s_expression)
        expression_prime = self.expression_prime()
        if expression_prime: expression.add_child(expression_prime)
        return expression

    def expression_prime(self) -> TreeNode | None:
        '''
        <выражение'> -> <операция отношения> <простое выражение> | ε
        '''
        #print("Парсим выражение'")
        expression_prime = TreeNode("<выражение'>")
        if self.lexer.current_token.type == 'RELATION_OP':
            rel_op = self.relation_operation()
            expression_prime.add_child(rel_op)

            s_expression = self.s_expression()
            expression_prime.add_child(s_expression)

            return expression_prime

    def s_expression(self) -> TreeNode:
        '''
        <простое выражение> ->
                |   <терм> <продолжение терма>
                |   <знак> <терм> <продолжение терма>
        '''
        #print('Парсим простое выражение')
        s_expression = TreeNode('<простое выражение>')
        if self.lexer.current_token.value in ['+', '-']:
            sign = self.lexer.current_token.value
            self.lexer.advance()

        term = self.term()
        s_expression.add_child(term)
        term_rest = self.term_rest()
        if term_rest: s_expression.add_child(term_rest)
        return s_expression

    def term_rest(self) -> TreeNode | None:
        '''
        <продолжение терма> -> <простое выражение'> | ε
        '''
        #print('Парсим продолжение терма (хвост его)')
        term_rest = TreeNode('<терм хвост>')
        s_expression_prime = self.s_expression_prime()
        if s_expression_prime:
            term_rest.add_child(s_expression_prime)
            return term_rest

    def s_expression_prime(self) -> TreeNode | None:
        '''
        <простое выражение'> -> <операция типа сложение> <терм> <простое выражение'> | ε
        '''
        #print("Парсим простое выражение'")
        s_expression_prime = TreeNode("<простое выражение'>")
        if self.lexer.current_token.type == 'ADD_OP':
            add_op = self.lexer.expect(self.lexer.current_token.type)
            s_expression_prime.add_child(TreeNode.from_token(add_op))

            term = self.term()
            s_expression_prime.add_child(term)

            rec_s_expression = self.s_expression_prime()
            if rec_s_expression: s_expression_prime.add_child(rec_s_expression)

            return s_expression_prime

    def term(self) -> TreeNode:
        '''
        <терм> -> <Фактор> <терм'>
        '''
        #print('Парсим терм')
        term = TreeNode('<терм>')

        factor = self.factor()
        term.add_child(factor)

        term_prime = self.term_prime()
        if term_prime: term.add_child(term_prime)

        return term

    def term_prime(self) -> TreeNode | None:
        '''
        <терм'> -> <операция типа умножения> <фактор> <терм'> | ε
        '''
        #print("Парсим терм'")
        term_prime = TreeNode("<терм'>")
        if self.lexer.current_token.type == 'MULT_OP':
            mult_op = self.multiplication_operation()
            term_prime.add_child(mult_op)

            factor = self.factor()
            term_prime.add_child(factor)

            rec_term_prime = self.term_prime()
            if rec_term_prime: term_prime.add_child(rec_term_prime)

    def factor(self) -> TreeNode:
        '''
        <фактор> ->
                |   <идентификатор>
                |   <константа>
                |   ( <простое выражение> )
                |   not <фактор>
        '''
        #print('Парсим фактор')
        factor = TreeNode('<фактор>')
        if self.lexer.current_token.type == 'IDENT':
            ident = self.identifier()
            factor.add_child(ident)
            return factor
        elif self.lexer.current_token.type == 'CONST':
            const = self.constant()
            factor.add_child(const)
        elif self.lexer.current_token.type == 'LPAREN':
            lparen = self.lexer.expect(self.lexer.current_token.type)
            factor.add_child(TreeNode.from_token(lparen))

            s_expression = self.s_expression()
            factor.add_child(s_expression)

            rparen = self.lexer.expect('RPAREN')
            factor.add_child(TreeNode.from_token(rparen))
        elif self.lexer.current_token.type == 'KEYWORD':
            nt = self.keyword()
            factor.add_child(nt)

            rec_factor = self.factor()
            if rec_factor: factor.add_child(rec_factor)

        return factor

    def relation_operation(self) -> TreeNode:
        '''
        <операция отнощения> -> = | <> | < | <= | > | >=
        '''
        rel_op = TreeNode('<операция отношения>')
        terminal = self.lexer.expect('RELATION_OP')
        #print(f'Распознана операция отношения: {terminal.value}')
        rel_op.add_child(TreeNode.from_token(terminal))
        return rel_op

    def addition_operation(self) -> TreeNode:
        '''
        <операция типа сложения> -> + | - | or
        '''
        add_op = TreeNode('<операция типа сложения>')
        terminal = self.lexer.expect('ADD_OP')
        #print(f'Распознана оперция сложения: {terminal.value}')
        add_op.add_child(TreeNode.from_token(terminal))
        return add_op

    def multiplication_operation(self) -> TreeNode:
        '''
        <операция типа умножения> -> * | / | div | mod | and
        '''
        mult_op = TreeNode('<операция типа умножения>')
        terminal = self.lexer.expect('MULT_OP')
        #print(f'Распознана оперция умножения: {terminal.value}')
        mult_op.add_child(TreeNode.from_token(terminal))
        return mult_op

    def keyword(self) -> TreeNode:
        terminal = self.lexer.expect('KEYWORD')
        #print(f'Распознано ключевое слово: {terminal.value}')
        return TreeNode(terminal.value)

    def constant(self) -> TreeNode:
        const = TreeNode('<константа>')
        terminal = self.lexer.expect('CONST')
        #print(f'Распознана константа: {terminal.value}')
        const.add_child(TreeNode.from_token(terminal))
        return const

    def identifier(self) -> TreeNode:
        ident = TreeNode('<идентификатор>')
        terminal = self.lexer.expect('IDENT')
        #print(f'Распознан идентификатор: {terminal.value}')
        ident.add_child(TreeNode.from_token(terminal))
        return ident


#Парсим:
#<программа> -> <блок>
#<блок> -> { <список операторов> }
#<список операторов> -> <оператор> <хвост>
#<оператор> -> <идентификатор> = <выражение> | <блок>
#<хвост> -> ; <оператор> <хвост> | ε
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


# '{ x = 10; y = x + 20 }',
# '{ a = 5; { b = a + 10 }; c = b * 2 }',

# '{ x = 100; y = 200; z = (x < y) or not (x = 200) }',
# '{ x = ( 1 + 2 ) }',
# '{ x = 10 / 0; y = (x + 100) and (x - 200) }',

# '{ a = 5 + 3; b = a - 2; c = b * 4; d = c / 2; e = d mod 2; f = 20 div 3; g = (a + b) and (c - d); h = not e == 0 }',
# '{ x = - y + 5; z = (x * (y - 2)) div 3; w = not (z + 10) or (x - 0) }',

code = '''
{ 
    x = - y + 5; 
    z = (x * (y - 2)) div 3; 
    w = not (z + 10) or (x - 0);;;;;;;
}
'''

lexer = Lexer(code)
print([token.value for token in lexer.tokens])
print([token.type for token in lexer.tokens])
parser = Parser(lexer)
tree = parser.parse()
tree.print().render()
print("Парсинг заверщен.")
