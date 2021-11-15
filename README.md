# Diff Seeker
Compare the contents of two directories. Implemented using handmade BST-based set.

## Build
```console
$ make -Bj
```

## Usage
```console
$ ./diffs <dir> <dir>
```

## Sample Output
```console
$ ./diffs /tmp/foo /tmp/bar
Comparing directories "/tmp/foo" and "/tmp/bar"
lorem.txt CHANGED
baz/maz/ipsum.txt DELETED
data.bin CHANGED
faz/newfile ADDED
Comparison finished in 0.000302 seconds. 2 files changed, 1 removed, 1 added
```
