package dfa

import (
	"fmt"
	"slices"

	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/collections"
)


type DFA struct {
	q  collections.Set[int]
	f  collections.Set[int]
	q0 int
	t  collections.Set[rune]
	d  map[int]map[rune]int

	qLabels map[int]string
}

// Replace states with uniq serials
func beautifyDFA(dfa tempDFA) DFA {
	result := DFA{t: dfa.t}

	stateMap := make(map[*state]int)

	// States
	curState := 0
	result.qLabels = make(map[int]string)

	for _, q := range dfa.q {
		if _, ok := stateMap[q]; !ok {
			stateMap[q] = curState
			result.q.Add(curState)

			// Labels for graphviz verbose mode
			slices.Sort(q.value)
			for j, t := range q.value {
				result.qLabels[curState] += fmt.Sprint(t)
				if j != len(q.value)-1 {
					result.qLabels[curState] += ","
				}
			}

			curState++
		}

		// Initial state
		if q.value.Equals(dfa.q0) {
			result.q0 = stateMap[q]
		}
	}

	// Finite states
	for _, f := range dfa.f {
		for k, v := range stateMap {
			if k.value.Equals(f) {
				result.f.Add(v)
			}
		}
	}

	// Transitions
	result.d = make(map[int]map[rune]int)
	for _, t := range dfa.d {
		var (
			ch               = t.symbol
			from, to, founds int
		)

		for k, v := range stateMap {
			if k.value.Equals(t.state) {
				from = v
				founds++
			}
			if k.value.Equals(t.destState) {
				to = v
				founds++
			}
			if founds == 2 {
				break
			}
		}

		if result.d[from] == nil {
			result.d[from] = make(map[rune]int)
		}

		result.d[from][ch] = to
	}

	return result
}
