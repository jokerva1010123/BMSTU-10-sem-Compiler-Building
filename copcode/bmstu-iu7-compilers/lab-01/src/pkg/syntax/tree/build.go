package tree

import (
	"strings"

	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/collections"
)

const (
	augmentChar = rune(0)
)

type Args struct {
	Export     bool
	ExportPath string

	ExportVerbose     bool
	ExportVerbosePath string
}

func New(re string, args Args) (*Node, collections.Set[rune], map[int]collections.Set[int]) {
	re = preProcessRE(re)

	tree := reToTree(re)
	if args.Export {
		exportTree(args.ExportPath, tree)
	}

	m := prepareTree(tree)
	if args.ExportVerbose {
		exportVerboseTree(args.ExportVerbosePath, tree)
	}

	return tree, getAlphabet(re), m
}

func preProcessRE(re string) string {
	builder := strings.Builder{}
	builder.WriteString("(")

	runes := []rune(re)
	for i := 0; i < len(runes)-1; i++ {
		builder.WriteByte(re[i])
		if !isBinaryOperation(runes[i]) && runes[i] != openBracket &&
			!isBinaryOperation(runes[i+1]) && re[i+1] != closeBracket && !isUnaryOperation(runes[i+1]) {
			builder.WriteByte('.')
		}
	}

	builder.WriteRune(runes[len(runes)-1])
	builder.WriteString(").")
	builder.WriteRune(augmentChar)

	return builder.String()
}

func reToTree(expr string) *Node {
	curPos := 0

	var (
		st1 collections.Stack[rune]
		st2 collections.Stack[*Node]
	)

	p := &Node{}
	for _, ch := range expr {
		switch {
		case ch == closeBracket:
			for st1.Top() != openBracket {
				hangUp(&st1, &st2, curPos)
				curPos++
			}
			st1.Pop()
		case ch == openBracket:
			st1.Push(ch)
		case isBinaryOperation(ch) || isUnaryOperation(ch):
			for !st1.Empty() && (oprtPrior(st1.Top()) >= oprtPrior(ch)) {
				hangUp(&st1, &st2, curPos)
				curPos++
			}
			st1.Push(ch)
		default:
			p = &Node{Value: string(ch), Pos: curPos}
			curPos++
			st2.Push(p)
		}
	}

	for !st1.Empty() {
		hangUp(&st1, &st2, curPos)
	}

	return st2.Top()
}

func prepareTree(t *Node) map[int]collections.Set[int] {
	m := make(map[int]collections.Set[int])
	prepareTreeRecursive(t, m)
	return m
}

func prepareTreeRecursive(t *Node, m map[int]collections.Set[int]) {
	for _, child := range t.Children {
		prepareTreeRecursive(child, m)
	}

	t.Nullable = nullable(t)
	t.FirstPos = firstPos(t)
	t.LastPos = lastPos(t)
	followPos(t, m)
}

func nullable(t *Node) bool {
	switch t.Value {
	case "e":
		return false
	case string(alternativeOperation):
		for _, child := range t.Children {
			if child.Nullable {
				return true
			}
		}
		return false
	case string(concatOperation):
		for _, child := range t.Children {
			if !child.Nullable {
				return false
			}
		}
		return true
	case string(iterationOperation):
		return true
	default:
		return false
	}
}

func firstPos(t *Node) collections.Set[int] {
	var s collections.Set[int]

	switch t.Value {
	case string(alternativeOperation):
		u := t.Children[0]
		v := t.Children[1]
		s = u.FirstPos.Unite(v.FirstPos)
	case string(concatOperation):
		u := t.Children[0]
		v := t.Children[1]
		if u.Nullable {
			s = u.FirstPos.Unite(v.FirstPos)
		} else {
			s = u.FirstPos
		}
	case string(iterationOperation):
		u := t.Children[0]
		s = u.FirstPos
	default:
		s.Add(t.Pos)
	}

	return s
}

func lastPos(t *Node) collections.Set[int] {
	var s collections.Set[int]

	switch t.Value {
	case string(alternativeOperation):
		u := t.Children[0]
		v := t.Children[1]
		s = u.LastPos.Unite(v.LastPos)
	case string(concatOperation):
		u := t.Children[0]
		v := t.Children[1]
		if v.Nullable {
			s = u.LastPos.Unite(v.LastPos)
		} else {
			s = v.LastPos
		}
	case string(iterationOperation):
		u := t.Children[0]
		s = u.LastPos
	default:
		s.Add(t.Pos)
	}

	return s
}

func followPos(t *Node, m map[int]collections.Set[int]) {
	switch t.Value {
	case string(concatOperation):
		for _, i := range t.Children[0].LastPos.ToArray() {
			m[i] = m[i].Unite(t.Children[1].FirstPos)
		}
	case string(iterationOperation):
		for _, i := range t.LastPos.ToArray() {
			m[i] = m[i].Unite(t.FirstPos)
		}
	}
}

func getAlphabet(re string) collections.Set[rune] {
	res := make(collections.Set[rune], 0, (len(re)+1)/2)
	for _, c := range re {
		if isSymbol(c) {
			res.Add(c)
		}
	}

	return res
}

func isSymbol(s rune) bool {
	return !isUnaryOperation(s) && !isBinaryOperation(s) &&
		s != openBracket &&
		s != closeBracket &&
		s != augmentChar
}
