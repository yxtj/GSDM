
- Description

- Extention

	- new searching strategy

	- new search method

- Parameters:

	- test

	```
		--n=50 --npi=10 --nni=10 --ns=10 --method freq 2 2 0.3 --strategy candidatePN 20 0.7
	```

	- adhd200

	```
		--prefix=../data_adhd/ --n=115 --npi=50 --nni=50 --ns=0 --method freq 1 10 0.3 --strategy candidatePN 20 0.3
		--prefix=../data_adhd --prefix-graph=graph-0.8 --n=115 --npi=20 --nni=20 --ns=0 --method freq 1 4 0.3 --strategy candidatePN 50 0.6 0.3 0.5 --blacklist 50 72 288 --out=test/
		--prefix=../data_adhd --prefix-graph=graph-0.8 --n=115 --npi=20 --nni=20 --ns=0 --method freq 1 4 0.3 --strategy DUG 50 conf exp 0.3 1 4 --blacklist 50 72 288 --out=test/
	```
