
- Description

	run `Search --help` for detailed description.

- Extention

	- new searching strategy from class StrategyBase
	- register it in StrategyFactory.cpp

- Parameters:

	run `Search --help` for detailed description.

	- example 

	```
	# ADHD200
	--prefix=../data_adhd --prefix-graph=data/graph --all-data --shared-input --typePos 1 2 3 --typeNeg 0 --n=115 --npi=10 --nni=10 --ns=10 --strategy ofg-para 10 0.7 diff:1 sd-no net dces log:../data_adhd/try/score.txt --out=try/result
	# ABIDE
	--prefix=../data_abide --prefix-graph=data/graph --all-data --shared-input --typePos 1 --typeNeg 2 --n=115 --npi=10 --nni=10 --ns=10 --strategy ofg-para 10 0.7 diff:1 sd net dces log:../data_adhd/try/score.txt --out=try/result
	```

