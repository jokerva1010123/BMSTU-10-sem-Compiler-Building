package dfa

import (
	"fmt"
	"io"
	"os"
	"strings"
)

func exportverboseDFA(path string, dfa DFA, verbose bool) {
	f, err := os.Create(path)
	defer f.Close()

	if err != nil {
		fmt.Printf("failed to export dfa: %v\n", err)
		return
	}

	printDFA(f, dfa, verbose)
}

func printDFA(out io.Writer, dfa DFA, verbose bool) {
	var sb strings.Builder

	sb.WriteString("digraph {\n")
	sb.WriteString("rankType=sink\n")
	sb.WriteString("rankType=sink\n")
	sb.WriteString("node [shape=circle margin=0 fontcolor=black fontsize=14 width=0.5]\n")

	for _, f := range dfa.f {
		if verbose {
			sb.WriteString(fmt.Sprintf("\"%s\" [shape=doublecircle]\n", dfa.qLabels[dfa.q.ToArray()[f]]))
		} else {
			sb.WriteString(fmt.Sprintf("%d [shape=doublecircle]\n", f))
		}
	}

	for from, t := range dfa.d {
		for ch, to := range t {
			if verbose {
				sb.WriteString(fmt.Sprintf("\"%s\" -> \"%s\" [label=%s];\n", dfa.qLabels[from], dfa.qLabels[to], string(ch)))
			} else {
				sb.WriteString(fmt.Sprintf("\"%d\" -> \"%d\" [label=%s];\n", from, to, string(ch)))
			}
		}
	}

	sb.WriteString("\n}")

	out.Write([]byte(sb.String()))
}

func (dfa *DFA) printConfiguration() {
	var out strings.Builder
	dfa.printStates(&out)
	dfa.printTable(&out)

	fmt.Println(out.String())
}

func (dfa *DFA) printStates(out io.StringWriter) {
	out.WriteString("Q0: ")
	out.WriteString(fmt.Sprint(dfa.q0))
	out.WriteString("\nF:  {")

	for i, f := range dfa.f {
		out.WriteString(fmt.Sprint(f))
		if i != len(dfa.f)-1 {
			out.WriteString(", ")
		}
	}

	out.WriteString("}\n")
}

func (dfa *DFA) printTable(out io.StringWriter) {
	out.WriteString("Transition table:\n")
	out.WriteString("\t")

	for _, state := range dfa.q {
		out.WriteString(fmt.Sprint(state))
		out.WriteString("\t")
	}
	out.WriteString("\n")

	for _, state := range dfa.q {
		out.WriteString(fmt.Sprint(state))
		out.WriteString("\t")

		for _, toState := range dfa.q {
			printed := false
			for ch, to := range dfa.d[state] {
				if toState == to {
					if printed {
						out.WriteString(",")
					}

					out.WriteString(string(ch))
					printed = true
				}
			}
			out.WriteString("\t")
		}
		out.WriteString("\n")
	}
}
