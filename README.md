# Diff Seeker
Compare the contents of two directories. Implemented using handmade [AVL tree](https://en.wikipedia.org/wiki/AVL_tree) based set.

## Build & Usage
```console
$ make -Bj
$ ./ds <dir> <dir>
```

## Sample Output
```console
$ ./ds /tmp/foo /tmp/bar
Comparing directories "/tmp/foo" and "/tmp/bar"
lorem.txt CHANGED
baz/maz/ipsum.txt DELETED
data.bin CHANGED
faz/newfile ADDED
Comparison finished in 0.000302 seconds. 2 files changed, 1 removed, 1 added
```
