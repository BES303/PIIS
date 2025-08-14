# PIIS - Personal Identifiable Information Scanner

A lightweight command-line tool for detecting personally identifiable information (PII) in text and PDF files.

## Features

- Detects emails, phone numbers, IP addresses, credit cards, passports, and URLs
- Two scanning strategies:
  - **Regex**: Precise pattern matching
  - **Keyword**: Case-insensitive keyword search
- Supports TXT and PDF formats
- Export results to JSON
- Custom pattern configuration
- Recursive directory scanning

## Build Requirements
- nlohmann_json
- cxxopts
- RE2
- poppler-cpp

## Build

```bash
git clone https://github.com/BES303/PIIS.git
cd PIIS
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

Scan a single file:
```bash
./PIIScanner -f document.txt
```

Scan directory recursively:
```bash
./PIIScanner -d /path/to/docs -r
```

Export results to JSON:
```bash
./PIIScanner -d /path/to/docs -j
```
