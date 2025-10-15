# Polytopes Process

A program for processing polytope data using the PALP (Package for Analyzing Lattice Polytopes) library.

## Dependencies

### Required Dependencies

- **Apache Parquet**: Required for data processing and storage operations.
  - Installation instructions are available at: https://arrow.apache.org/install/
  - Make sure to install the appropriate version for your system

### Build Dependencies

- GCC compiler with C99 support
- Make
- Standard C libraries

## Building

To build the program:

```bash
make
```

To build with debug symbols:

```bash
make debug
```

To clean build artifacts:

```bash
make clean
```

To clean everything including PALP objects:

```bash
make cleanall
```

## Usage

### Main Program
```bash
./polytopes-process <input_file>
```

Where `<input_file>` is a file containing polytope data in the expected format.

### Parquet File Reader
```bash
make parquet-reader
./parquet-reader <parquet_file>
```

The parquet reader utility can read and display the contents of Apache Parquet files line by line. It shows the schema information and prints the first few rows of each column.

## Project Structure

- `src/` - Main source code
- `thirdparty/PALP/` - PALP library source code
- `testing/` - Test files

## Notes

This program uses the PALP library for polytope computations. The PALP source code is included in the `thirdparty/PALP/` directory and will be automatically compiled as part of the build process.