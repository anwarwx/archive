all: extra latex

extra:
	mkdir -p output
	python3 main.py

latex:
	pdflatex report.tex -o report.pdf
	make .PHONY

.PHONY:
	rm -rf *.aux *.log *.out

clean:
	rm -rf *.aux *.log *.out *.txt *.pdf *.png
	rm -rf output
