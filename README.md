
**Note:** This repository is part of a work currently under review entitled *"xxxxx"*.

This repository provides a Apple M1/M4 implementation of HQC, a code-based Key Encapsulation Mechanism (KEM) whose security is based on the hardness of solving the Quasi-Cylic Syndrome Decoding (QCSD) problem. HQC is one of the selected algorithms from the NIST's Post-Quantum Cryptography Standardization Project. This implementation is based on the implementation available on (https://github.com/ChunTaoPengim/HQC_with_addFFT_tches2026/). 



## Contents

#### HQC implementations: 

This directory contains HQC optimization for Apple M1/M4.

#### Benchmark tools : benchmarktool/

## Makefile guide

This guide explains how to use the Makefile to build and test different configurations of the implementation.

### Basic Usage

```bash
make PROJ=<parameter>
```

- The default setting is `PROJ=hqc-1`.

### Configuration Options

#### PROJ - Parameter Sets

Choose one of three HQC parameter sets:

- `hqc-1` - HQC parameter set 1
- `hqc-3` - HQC parameter set 3
- `hqc-5` - HQC parameter set 5

#### Testing

After compiling, run the following command to test and benchmark defferent components.

```bash
sudo ./bin/gf2x-test
sudo ./bin/code-test
sudo ./bin/code-bench
sudo ./bin/hashBench
sudo ./bin/hqc-test
sudo ./bin/kem2
sudo ./bin/pkeBench
```


