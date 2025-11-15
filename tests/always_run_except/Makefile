all: out1.txt out2.txt out3.txt
	@echo "Building all"
	@cat out1.txt out2.txt out3.txt > all.txt

out1.txt: in1.txt
	@echo "Building out1.txt from in1.txt"
	@cp in1.txt out1.txt

out2.txt: in2.txt
	@echo "Building out2.txt from in2.txt"
	@cp in2.txt out2.txt

out3.txt: in3.txt
	@echo "Building out3.txt from in3.txt"
	@cp in3.txt out3.txt

clean:
	rm -f out1.txt out2.txt out3.txt all.txt

.PHONY: all clean
