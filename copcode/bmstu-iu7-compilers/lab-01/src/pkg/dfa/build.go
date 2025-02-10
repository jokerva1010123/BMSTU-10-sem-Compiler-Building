package dfa

import (
	"fmt"

	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/collections"
	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/syntax/tree"
)

type tempDFA struct {
	q  []*state
	t  collections.Set[rune]
	d  []tran
	q0 collections.Set[int]
	f  []collections.Set[int]
}

type tran struct {
	state     collections.Set[int]
	symbol    rune
	destState collections.Set[int]
}

type state struct {
	value  collections.Set[int]
	marked bool
}

func toTreeMap(t *tree.Node) map[int]*tree.Node {
	m := make(map[int]*tree.Node)
	m[t.Pos] = t

	for _, c := range t.Children {
		treeMapRec(c, m)
	}

	return m
}

func treeMapRec(t *tree.Node, m map[int]*tree.Node) {
	m[t.Pos] = t

	for _, c := range t.Children {
		treeMapRec(c, m)
	}
}

func (dfa *tempDFA) hasUnmarked() bool {
	for _, s := range dfa.q {
		if !s.marked {
			return true
		}
	}

	return false
}

func (dfa *tempDFA) getUnmarkedPos() int {
	for i, s := range dfa.q {
		if !s.marked {
			return i
		}
	}

	return -1
}

func (dfa *tempDFA) hasState(s collections.Set[int]) bool {
	for i := range dfa.q {
		if dfa.q[i].value.Equals(s) {
			return true
		}
	}

	return false
}

func (dfa *tempDFA) buildF(endPos int) {
	for _, s := range dfa.q {
		if s.value.Contains(endPos) {
			dfa.f = append(dfa.f, s.value)
		}
	}
}

func buildDFA(t *tree.Node, alphabet collections.Set[rune], followPos map[int]collections.Set[int]) DFA {
	var dfa tempDFA
	dfa.t = alphabet

	treeMap := toTreeMap(t)

	dfa.q0 = t.FirstPos
	dfa.q = append(dfa.q, &state{dfa.q0, false})

	for dfa.hasUnmarked() {
		R := dfa.q[dfa.getUnmarkedPos()]
		R.marked = true

		for _, symbol := range dfa.t {
			var u collections.Set[int]
			for _, p := range R.value {
				if []rune(treeMap[p].Value)[0] == symbol {
					u = u.Unite(followPos[p])
				}
			}

			// remove this to reuse "pseudo node"
			if len(u) == 0 {
				continue
			}

			if !dfa.hasState(u) {
				dfa.q = append(dfa.q, &state{u, false})
			}

			dfa.d = append(dfa.d, tran{R.value, symbol, u})
		}
	}

	dfa.buildF(t.Children[1].Pos)

	return beautifyDFA(dfa)
}

type BuildArgs struct {
	PrintDFA bool

	ExportTree     bool
	ExportTreePath string

	ExportVerboseTree     bool
	ExportVerboseTreePath string

	ExportDFA        bool
	ExportDFAPath    string
	ExportMinDFAPath string
	ExportVerboseDFA bool
}

func Build(re string, args BuildArgs) DFA {
	tree, alphabet, m := tree.New(re, tree.Args{
		Export:            args.ExportTree,
		ExportPath:        args.ExportTreePath,
		ExportVerbose:     args.ExportVerboseTree,
		ExportVerbosePath: args.ExportVerboseTreePath,
	})

	DFA := buildDFA(tree, alphabet, m)

	if args.PrintDFA {
		fmt.Println("DFA of regex:")
		DFA.printConfiguration()

		if args.ExportDFA {
			exportverboseDFA(args.ExportDFAPath, DFA, args.ExportVerboseDFA)
		}
	}

	MinimiseHopcroft(&DFA)

	if args.PrintDFA {
		fmt.Println("Minimized DFA:")
		DFA.printConfiguration()

		if args.ExportDFA {
			exportverboseDFA(args.ExportMinDFAPath, DFA, false)
		}
	}

	return DFA
}
