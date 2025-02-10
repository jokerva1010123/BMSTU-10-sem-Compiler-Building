

from graphviz import Digraph


class Node:
    def __init__(self, v = None, l = None, r = None):
        self.left = l
        self.right = r
        self.value = v
        self.followpos = []
        self.label_i = ''
    def copy(cur):
        return Node(cur.value, cur.left, cur.right)
    left = None
    right = None
    value = None

    def show_tree(pt, dot = None, show_params=False):
        if not dot:
            dot = Digraph()
        label = str(pt.value) + ("; " + str(pt.label_i) if pt.label_i else "")
        def nodes_to_i(nodes):
            return [node.i for node in nodes]
        if show_params:
            label = label + "(" + ("Nullable, " if pt.nullable else "") + str(nodes_to_i(pt.firstpos)) + ", " + str(
                nodes_to_i(pt.lastpos)) + ", " + str(nodes_to_i(pt.followpos)) + ")"
        dot.node(str(pt.i), label)
        if pt.left:
            dot.edge(str(pt.i), str(pt.left.i))
            pt.left.show_tree(dot, show_params=show_params)
        if pt.right:
            dot.edge(str(pt.i), str(pt.right.i))
            pt.right.show_tree(dot, show_params=show_params)
        return dot

    def count_i(self, i = 0):
        self.i = i
        if self.left:
            self.left.count_i(i*2+1)
        if self.right:
            self.right.count_i(i*2+2)

    def count_nullable(self):
        if self.left:
            self.left.count_nullable()
        if self.right:
            self.right.count_nullable()

        if self.value == '*':
            self.nullable = True
        elif self.value== '|':
            self.nullable = self.left.nullable or self.right.nullable
        else:
            self.nullable = False
        return self.nullable

    def count_firstpos(self):
        if self.left:
            self.left.count_firstpos()
        if self.right:
            self.right.count_firstpos()

        if self.value in ['*', '+']:
            self.firstpos = self.left.firstpos.copy()
        elif self.value == '':
            self.firstpos = self.left.firstpos.copy()
            if self.left.nullable:
                self.firstpos += self.right.firstpos
        elif self.value== '|':
            self.firstpos = self.left.firstpos + self.right.firstpos 
        else:
            self.firstpos = [self]
        return self.firstpos

    def count_lastpos(self):
        if self.left:
            self.left.count_lastpos()
        if self.right:
            self.right.count_lastpos()

        if self.value in ['*', '+']:
            self.lastpos = self.left.lastpos.copy()
        elif self.value == '':
            self.lastpos = self.right.lastpos.copy()
            if self.right.nullable:
                self.lastpos += self.left.lastpos
        elif self.value== '|':
            self.lastpos = self.left.lastpos + self.right.lastpos 
        else:
            self.lastpos = [self]
        return self.lastpos
    
    def count_followpos(self):
        if self.left:
            self.left.count_followpos()
        if self.right:
            self.right.count_followpos()
        if self.value == '':
            for i in self.left.lastpos:
                i.followpos += self.right.firstpos
        elif self.value == '*':
            for i in self.lastpos:
                i.followpos += self.firstpos

    def foreach_deep(self, funcIn = None, FuncOut = None):
        if funcIn:
            funcIn(self)
        if self.left:
            self.left.foreach_deep(funcIn, FuncOut)
        if self.right:
            self.right.foreach_deep(funcIn, FuncOut)
        if FuncOut:
            FuncOut(self)
        

def buildParseTree(INITexpr):
    def get_expr(exp):
        def cat_exprs(left, right):
            return Node('', left, right) 
        s = 0
        tree = None
        if exp[s] == '(':
            s += 1
            e, n = get_expr(exp[s:])
            s += n
            tree = e
            while exp[s] != ')':
                e, n = get_expr(exp[s:])
                s += n
                tree = cat_exprs(tree, e)
        elif exp[s] not in ['+', '*', '|', ')']:
            tree = Node(exp[s])
        else:
            raise Exception('Waiting for "(", term: ' +  "".join(exp) + ' '+ str(s))
        s += 1
        if s < len(exp)and exp[s] in ['+', '*']:
                tree = Node(exp[s], tree)
                s+=1

        if s < len(exp) and exp[s] == '|':
                next_expr, l = get_expr(exp[s+1:])
                return Node('|', tree, next_expr), s + 1 + l
        
        return tree, s

    x, r =  get_expr(['('] + INITexpr + [')'])
    return x




def make_ka(root: Node, symbols, end_symb='END'):
    def  make_state(nodes):
        return tuple(sorted([node.label_i for node in nodes]))
    
    def is_end(nodes):
        for node in nodes:
            if node.value == end_symb:
                return True
        return False
    Dstates = []
    
    table = {}
    queue = [root.firstpos]

    last = []
    while len(queue):
        state = queue.pop()
        Dstates += [make_state(state)]
        for c in symbols:
            U = []
            for p in state:
                if p.value == c:
                    U += p.followpos
            U = list(set(U))
            if not len(U): 
                continue
            
            if make_state(U) not in Dstates:
                queue.append(U)

           

            table[make_state(state), c] = [make_state(U)]

            #print(make_state(state), c, [make_state(U)])

        if is_end(state):
                last += [make_state(state)]


    return [Dstates[0]], last, table    




def fix_names(first, last, table):
    names = []
    for f,v in table:
        if f not in names:
            names += [f]
        for t in table[f,v]:
            if not t in names:
                names += [t]
    newTable = {}
    for f,v in table:
        newTable[(names.index(f), v)]  = [names.index(t) for t in table[f,v]]

    return [names.index(t) for t in first], [names.index(t) for t in last], newTable

def print_ka(first, end, Dtran):
    g = Digraph()

    g.node('S', 'Start')
    g.node('E', 'End')

    for i in first:
        g.edge('S', str(i))
    
    for i in end:
        g.edge(str(i), 'E')
        

    for f,v in Dtran:
        for t in Dtran[(f,v)]:
            g.node(str(f))
            g.node(str(t))
            g.edge(str(f), str(t), v )

    return g





def reverseKa(first, last, Dtran):
    newDtran = {}
    for f,v in Dtran:
        for t in Dtran[(f,v)]:
            if (t,v) not in newDtran:
                newDtran[(t, v)] = []     
            newDtran[(t, v)] += [f]
    return last, first, newDtran


def toDFA(first, last, table):
    """
    Алгоритм Томпсона строит по НКА эквивалентный ДКА следующим образом:

    Начало.
    Шаг 1. Помещаем в очередь Q
        множество, состоящее только из стартовой вершины.
    Шаг 2. Затем, пока очередь не пуста выполняем следующие действия:
        Достаем из очереди множество, назовем его q
        Для всех c∈Σ
        посмотрим в какое состояние ведет переход по символу c из каждого состояния в q.
        Полученное множество состояний положим в очередь Q
        только если оно не лежало там раньше. Каждое такое множество в итоговом
        ДКА будет отдельной вершиной, в которую будут вести переходы по соответствующим символам.
        Если в множестве q хотя бы одна из вершин была терминальной в НКА, то
        соответствующая данному множеству вершина в ДКА также будет терминальной.

    Конец.
    """
    def nodes_to_state_name(l):
        return tuple(set(l))

    symbols = []
    for _, v in table:
        symbols += [v]
    symbols = list(set(symbols))

    queue = [first.copy()]

    newTable = {}
    states = []

    while len(queue):
        nodes = queue.pop()
        states.append(nodes_to_state_name(nodes))
        for c in symbols:
            state = []
            for node in nodes:
                if (node, c) in table:
                    state += table[(node,c)]
            state = nodes_to_state_name(state)

            if not len(state):
                continue
            newTable[(nodes_to_state_name(nodes), c)] = [state]
            
            if state not in states:
                queue.append(state)
            
       
    def intersection(lst1, lst2):
        lst3 = [value for value in lst1 if value in lst2]
        return lst3


    return [nodes_to_state_name(first)], [
        state for state in states if intersection(state, last)
    ], newTable





def check_ka(st, fir, las, table):
    state = fir[0]
    for c in st:
        if (state, c) not in table:
            return False
        state = table[(state,c)][0]

    return state in las



def print_firstpos_info(node, indent=""):
    if node:
        print(indent + "Value:", node.value)
        print(indent + "Firstpos:", [n.value for n in node.firstpos])
        print()
        if node.left:
            print(indent + "Left:")
            print_firstpos_info(node.left, indent + "  ")
        if node.right:
            print(indent + "Right:")
            print_firstpos_info(node.right, indent + "  ")

def print_node_info(node, indent=""):
    if node:
        print(indent + "Value:", node.value)
        print(indent + "Nullable:", node.nullable)
        print(indent + "Firstpos:", [n.value for n in node.firstpos])
        print(indent + "Lastpos:", [n.value for n in node.lastpos])
        print(indent + "Followpos:", [n.value for n in node.followpos])
        print()
        if node.left:
            print(indent + "Left:")
            print_node_info(node.left, indent + "  ")
        if node.right:
            print(indent + "Right:")
            print_node_info(node.right, indent + "  ")


def prepare_all(regK):
    end_symb = 'END'
    pt = buildParseTree(list(regK) + [end_symb])


    pt.count_i()

    pt.count_nullable()
    pt.count_firstpos()
    pt.count_lastpos()

    pt.count_followpos()

    #print_node_info(pt)
    #print_firstpos_info(pt)
    nodes = []
    def make_nodes(node):
        if node.value not in ('+', '|', '', '*'):
            nodes.append(node)

    pt.foreach_deep(FuncOut=make_nodes)
    for i, node in enumerate(nodes, 1):
        node.label_i = i

    symbols = list(set([node.value for node in nodes]))

    dfa = make_ka(pt, symbols, end_symb=end_symb)

    steps_V = []

    modified = dfa



    def step(ins):
        fix = fix_names(*ins)
        r = reverseKa(*fix)
        dfa = toDFA(*r)
        fix2 = fix_names(*dfa)
        steps_V.append([fix, r, dfa, fix2])
        return fix2

    modified = step(step(modified))

    return pt, dfa, modified, steps_V



regK = "(a|b)*aabb"
z = prepare_all(regK)

import traceback
def change_reg():
    global regK, z
    print("""
Допустимые терминальные символы: *, +, |, (, )
Иные символы воспринимаются как нетерминальные
          """)
    x = input("Введите новое регулярное выражение: ")
    
    try:
        z = prepare_all(x)
    except Exception as e:
        print(traceback.format_exc())
        input("Произошла ошибка" + str(e))
        return change_reg()
    regK = x

def show_dla():
    z[0].show_tree().view()
def show_dfa():
    print_ka(*z[1]).view()
def show_mdfa():
    print_ka(*z[2]).view()

def show_mdfa_steps():
    for s in z[3]:
        for i in s:
            print_ka(*i).view()
            input()



def init_dfa():

    s = input("Введите строку (для выхода 'quit'): ")
    if s != "quit":
        res = check_ka(s, *z[2])
        print("Соответствует" if res else "Не соответствует")
        init_dfa()
    

while True:
    x = -1
    try:
        x = int(input("""
Текущее регулярное выражение {:}

0. Указать рег. выражение  
1. Показать дерево
2. Показать ДКА
3. Показать МДКА
4. Проверить входную цепочку на соответствие рег. выражению

Введите пункт меню: """.format(regK)))
    except:
        pass
    
    actions = [
        change_reg,
        show_dla,
        show_dfa,
        show_mdfa,
        init_dfa,
        show_mdfa_steps,
    ]

    if x < 0 or x > len(actions):
        input("Input error")
        continue
    
    actions[x]()

   

    

    
    
    