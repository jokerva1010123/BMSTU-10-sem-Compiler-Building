.PHONY: default

OUTPUT_DIR = .

default:
	$(foreach file, $(wildcard $(OUTPUT_DIR)/*.txt), dot -Tsvg $(file) -o $(basename $(file)).svg; open $(basename $(file)).svg;)

clean:
	@rm -f *.txt *.svg
