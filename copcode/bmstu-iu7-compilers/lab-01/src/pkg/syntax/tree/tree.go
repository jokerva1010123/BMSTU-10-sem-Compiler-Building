package tree

import (
	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/collections"
)

const (
	concatOperation      = '.'
	alternativeOperation = '|'
	iterationOperation   = '*'
	openBracket          = '('
	closeBracket         = ')'
)

var ops = map[rune]int{
	alternativeOperation: 1,
	concatOperation:      2,
	iterationOperation:   3,
	openBracket:          0,
	closeBracket:         0,
}

type Node struct {
	Value    string
	Pos      int
	Nullable bool
	FirstPos collections.Set[int]
	LastPos  collections.Set[int]
	Children []*Node
}

func isBinaryOperation[T rune](c T) bool {
	return c == concatOperation || c == alternativeOperation
}

func isUnaryOperation(c rune) bool {
	return c == iterationOperation
}

func oprtPrior(oprt rune) int {
	return ops[oprt]
}

func hangUp[T rune, R *Node](st1 *collections.Stack[T], st2 *collections.Stack[R], curPos int) {
	value := st1.Pop()
	p := &Node{Value: string(value), Pos: curPos}

	right := st2.Pop()
	if isBinaryOperation(value) {
		left := st2.Pop()
		p.Children = append(p.Children, left, right)
	} else {
		p.Children = append(p.Children, right)
	}

	st2.Push(p)
}
