package main

import (
	"flag"
	"fmt"

	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/dfa"
	"github.com/migregal/bmstu-iu7-compilers/lab-01/pkg/input"
)

var (
	StdinFlag    bool
	FileNameFlag string

	VerboseOne   bool
	VerboseTwo   bool
	VerboseThree bool

	TreePath        string
	VerboseTreePath string

	DFAPath        string
	VerboseDFAPath string

	MinDFAPath        string
	VerboseMinDFAPath string
)

func init() {
	flag.BoolVar(&StdinFlag, "s", true, "read input from stdin")
	flag.StringVar(&FileNameFlag, "f", "", "file path to read from")

	flag.BoolVar(&VerboseOne, "v", false, "display basic info about DFA")
	flag.BoolVar(&VerboseTwo, "vv", false, "v + export regex tree & dfa")
	flag.BoolVar(&VerboseThree, "vvv", false, "vv + export regex tree & dfa in verbose mode")

	flag.StringVar(&TreePath, "t", "./tree.txt", "path to store exported tree")
	flag.StringVar(&VerboseTreePath, "vt", "./verbose_tree.txt", "path to store exported verbose tree")

	flag.StringVar(&DFAPath, "d", "./dfa.txt", "path to store exported DFA")
	flag.StringVar(&VerboseDFAPath, "vd", "./vebose_dfa.txt", "path to store exported verbose DFA")

	flag.StringVar(&MinDFAPath, "md", "./min_dfa.txt", "path to store exported minimized DFA")
	flag.StringVar(&VerboseMinDFAPath, "vmd", "./verbose_min_dfa.txt", "path to store exported minimized verbose DFA")

	flag.Parse()
}

func main() {
	args := flag.Args()
	if len(args) < 1 {
		fmt.Println("Missing regex value")
		flag.Usage()
		return
	}

	re := args[0]

	DFA := dfa.Build(re, dfa.BuildArgs{
		PrintDFA:              VerboseOne || VerboseTwo || VerboseThree,
		ExportTree:            VerboseTwo || VerboseThree,
		ExportTreePath:        TreePath,
		ExportVerboseTree:     VerboseThree,
		ExportVerboseTreePath: VerboseTreePath,
		ExportDFA:             VerboseTwo || VerboseThree,
		ExportDFAPath:         DFAPath,
		ExportMinDFAPath:      MinDFAPath,
		ExportVerboseDFA:      VerboseThree,
	})

	var (
		reader input.REReader
		err    error
	)

	if len(FileNameFlag) > 0 {
		reader, err = input.NewFileREReader(FileNameFlag)
	} else {
		reader = input.NewStdIREReader()
	}

	if err != nil {
		fmt.Printf("something went wrong: %v", err)
		flag.Usage()
		return
	}

	for s, ok := reader.NextRE(); ok; s, ok = reader.NextRE() {
		matched := DFA.Run(s, VerboseOne || VerboseTwo || VerboseThree)
		fmt.Println(s+":", matched)
	}
}
