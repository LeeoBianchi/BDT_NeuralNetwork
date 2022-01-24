# BDT Neural Network
A C++ implementation of a Boosted Decision Tree Neural Network exploiting some ROOT Classes for signal/background separation.

a) plots the histogram of the data separated in signal and background categories.

b) compare the error function resulting from the same algorithm run with different boosting iteration number values. It allows to see the iteration number upper bound for not letting the network fall in overtraining.

The A makefile is included. Compile the program typing "make".
ROOT is required, visit https://root.cern/install/ for intallation details.
