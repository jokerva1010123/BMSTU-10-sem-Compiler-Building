package dfa

import "fmt"

func (dfa *DFA) Run(s string, verbose bool) bool {
	curState := dfa.q0

	var matched bool
	for _, ch := range s {
		if verbose {
			fmt.Printf("cur state (%d): check for '%s':\n", curState, string(ch))

			for via, to := range dfa.d[curState] {
				fmt.Printf("| %s -> %d\n", string(via), to)
			}
		}

		if curState, matched = dfa.d[curState][ch]; !matched {
			return false
		}
	}

	matched = false
	for _, f := range dfa.f {
		if curState == f {
			matched = true
			break
		}
	}

	return matched
}
