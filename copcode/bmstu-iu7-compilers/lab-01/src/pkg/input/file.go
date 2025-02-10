package input

import (
	"bufio"
	"fmt"
	"os"
)

type FileREReader struct {
	file    *os.File
	scanner *bufio.Scanner
}

func NewFileREReader(filename string) (*FileREReader, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("open file: %w", err)
	}

	return &FileREReader{
		file:    file,
		scanner: bufio.NewScanner(file),
	}, nil
}

func (reader *FileREReader) NextRE() (string, bool) {
	scanned := reader.scanner.Scan()
	if !scanned {
		_ = reader.file.Close()
		return "", false
	}

	re := reader.scanner.Text()
	return re, true
}
