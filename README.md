# Persistent Finder

* Persistent Finder: Fast Detection of Persistent Items in Data Streams 

# Introduction

* The source codes of Persistent Finder and other related algorithms.

# How to run

Suppose you've already cloned the repository.

You just need:

```
$ cmake .
$ make
$ ./bench (-d dataset -m memory -w window's size)
```

**optional** arguments:

- -d: set the path of dataset to run, default dataset is CAIDA15 "./data/caida15.dat"
- -m: set the memory size (KB), default memory is 500KB
- -w: set the size of window, default **w** is 1600
