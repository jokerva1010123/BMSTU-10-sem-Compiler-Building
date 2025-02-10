package dfa

import (
	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/collections"
)

func MinimiseHopcroft(dfa *DFA) {
	P := []collections.Set[int]{dfa.f.Dupl(), dfa.q.Subtract(dfa.f).Dupl()}

	Class := mapClasses(P)

	inv := mapTrans(dfa.d)

	type req struct {
		C collections.Set[int]
		a rune
	}

	queue := collections.Queue[req]{}
	for _, c := range dfa.t {
		queue.Push(req{dfa.f, c})
		queue.Push(req{dfa.q.Subtract(dfa.f), c})
	}

	for !queue.Empty() {
		r := queue.Pop()
		C, a := r.C, r.a

		involved := make(map[int]*collections.Set[int])
		for _, q := range C {
			rs, ok := inv[q][a]
			if !ok {
				continue
			}

			for _, r := range *rs {
				i := Class[r]
				if _, ok := involved[i]; !ok {
					involved[i] = &collections.Set[int]{}
				}
				involved[i].Add(r)
			}
		}

		for i := range involved {
			if involved[i].Size() < len(P[i]) {
				P = append(P, collections.Set[int]{})
				j := len(P) - 1

				for _, r := range *involved[i] {
					P = swapState(P, r, i, j)
				}

				if len(P[j]) > len(P[i]) {
					P[j], P[i] = P[i], P[j]
				}

				for _, r := range P[j] {
					Class[r] = j
				}

				for _, c := range dfa.t {
					queue.Push(req{P[j], c})
				}
			}
		}
	}

	dfa.mapOptimalStates(P)
}

func swapState(P []collections.Set[int], state int, from int, to int) []collections.Set[int] {
	for i, v := range P[from] {
		if v == state {
			P[from] = append(P[from][:i], P[from][i+1:]...)
			break
		}
	}

	P[to] = append(P[to], state)
	return P
}

func mapClasses(P []collections.Set[int]) map[int]int {
	stateClass := make(map[int]int)

	for classIndex := range P {
		for _, v := range P[classIndex] {
			stateClass[v] = classIndex
		}
	}

	return stateClass
}

func mapTrans(trans map[int]map[rune]int) map[int]map[rune]*collections.Set[int] {
	inv := make(map[int]map[rune]*collections.Set[int])

	for from, t := range trans {
		for ch, to := range t {
			if _, ok := inv[to]; !ok {
				inv[to] = make(map[rune]*collections.Set[int])
			}
			if _, ok := inv[to][ch]; !ok {
				inv[to][ch] = &collections.Set[int]{}
			}
			inv[to][ch].Add(from)
		}
	}

	return inv
}

func (dfa *DFA) mapOptimalStates(P []collections.Set[int]) {
	stateMap := make(map[int]int)

	// make q0 state to be zero in P
	for i, q := range P {
		var found bool

		for _, state := range q {
			if state == dfa.q0 {
				P[0], P[i] = P[i], P[0]
				found = true
			}
		}

		if found {
			break
		}
	}

	// States
	newQ := make([]int, 0, len(P))
	for i, q := range P {
		if len(q) == 0 {
			continue
		}

		for _, state := range q {
			stateMap[state] = i
		}

		newQ = append(newQ, i)
	}

	// Finit states
	resultF := collections.Set[int]{}
	for _, f := range dfa.f {
		for k, v := range stateMap {
			if k == f {
				resultF.Add(v)
				break
			}
		}
	}

	// Transitions
	resultTrans := make(map[int]map[rune]int)
	for from, t := range dfa.d {
		for ch, to := range t {
			var (
				rFrom, rTo, founds int
			)

			for k, v := range stateMap {
				if k == from {
					rFrom = v
					founds++
				}
				if k == to {
					rTo = v
					founds++
				}
				if founds == 2 {
					break
				}
			}

			if resultTrans[rFrom] == nil {
				resultTrans[rFrom] = make(map[rune]int)
			}
			resultTrans[rFrom][ch] = rTo
		}
	}

	dfa.q0 = stateMap[dfa.q0]
	dfa.q = newQ
	dfa.f = resultF
	dfa.d = resultTrans
}
