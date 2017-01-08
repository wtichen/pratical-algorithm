all:
	g++-6 main.cpp -o viterbi

test:
	./viterbi ./data/input_10.txt out10; diff out10 ./data/output_10.txt
	./viterbi ./data/input_200.txt out200; diff out200 ./data/output_200.txt

clean:
	rm viterbi out10 out200
