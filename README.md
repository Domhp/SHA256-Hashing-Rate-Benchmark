# SHA256-Hashing-Rate-Benchmark

**Test hash rate**
```
gcc -Wall main.c sha256.c -o main -lpthread
./main
```
**Test average time to find leading zeros (mining)**
```
gcc -Wall leadingZeros.c sha256.c -o leadingZero -lpthread
./leadingZero
```

# Running with shell script with sensors
```
chmod +x run.sh
./run.sh
```
