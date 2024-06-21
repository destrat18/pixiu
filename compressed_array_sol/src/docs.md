### Requirements 

1. g++ 
2. boost 

### Build 

```
make science
```

### The `.gr` format

All graph are described in the DIMACS graph format. The first line of file should contain 
```
p tw $V $E
```

where `$V` is number of vertices in graph, `$E` is number of edges in the grpaph. 

Vertives must have indices from `1` to `$V` inclusive. Next `$E` lines contains list of edges. 

Example (`example.gr`): 

```
p tw 8 7
1 2
1 6
1 7
1 8
2 3
2 4
2 5
```

### Data format 

Two choices: folder or file. If folder given, computations will be calculations will be done on all `.gr` files inside folder.


If file given, it shold follow next format: 
```s
cid $cid1
p tw $V1 $E1
...
cid $cid2
p tw $V2 $E2
...
.
.
.
cid $cidK
p tw $VK $EK
...
```

Example (`graphs.in`):
```
cid 199
p tw 9 8
1 2
1 8
2 3
3 4
4 5
5 6
5 7
8 9
cid 6429
p tw 8 7
1 2
1 6
1 7
1 8
2 3
2 4
2 5
cid 101630131
p tw 20 22
1 2
1 6
2 3
3 4
4 5
4 15
5 6
5 7
7 8
7 20
8 9
8 19
9 10
9 14
10 11
11 12
12 13
13 14
14 15
15 16
16 17
16 18
```

### Usage 

```
./run folder-name|file-name result-file-name naive|tw
```



