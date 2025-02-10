package tree

import (
	"fmt"
	"io"
	"os"
	"strings"
)

func exportTree(path string, tree *Node) {
	f, err := os.Create(path)
	defer f.Close()

	if err != nil {
		fmt.Printf("failed to export tree: %v", err)
		return
	}

	printTree(f, tree, false)
}

func exportVerboseTree(path string, tree *Node) {
	f, err := os.Create(path)
	defer f.Close()

	if err != nil {
		fmt.Printf("failed to export verbose tree: %v", err)
		return
	}

	printTree(f, tree, true)
}

func printTree(out io.Writer, t *Node, verbose bool) {
	var sb strings.Builder

	sb.WriteString("digraph {\n")
	sb.WriteString("rankType=sink\n")
	sb.WriteString("rankType=sink\n")
	sb.WriteString("node [shape=none margin=0 fontcolor=black fontsize=14 width=0.5]\n")

	printTreeRec(&sb, t, verbose)

	sb.WriteString("\n}")

	out.Write([]byte(sb.String()))
}

func printTreeRec(out io.StringWriter, t *Node, verbose bool) {
	label := t.Value
	if label == string(augmentChar) {
		label = "Ø"
	}

	if verbose {
		out.WriteString(fmt.Sprintf(
			"\"%d\" [label=\"%d: {'%s'; %t; %v; %v}\"];\n", t.Pos, t.Pos, label, t.Nullable, t.FirstPos, t.LastPos,
		))
	} else {
		out.WriteString(fmt.Sprintf("\"%d\" [label=\"%s\"];\n", t.Pos, label))
	}

	for _, node := range t.Children {
		printTreeRec(out, node, verbose)

		out.WriteString(fmt.Sprintf("\"%d\" -> \"%d\";\n", t.Pos, node.Pos))
	}
}
