# Problem 3

> Please note that this is a simple **design** problem, in the sense that there
  are many possible solutions. You are to design one and implement it. As long
  as your approach satisfies the requirements, you are good!

> **However**, your design should only use tools **that we have discussed in
  class**. In other words, anything you use that was not discussed in class will
  receive a 0 grade.

> Please note that for this problem, you should submit a text file, called
  `problem3.txt` in which you detail your approach and what you used to implement
  it.

> A good design, even if not fully functional, will earn you a good chunk of
  credit. Therefore, be detailed and explain what you were trying to do, even
  if you were not able to fully implement it.


## The mystery function

In this problem, we are given a mystery function called `is_good_child(int,
in)`. The source code for this function is not accessible to you as you are
provided with an object file (`mystery.o`). **Please make sure to not delete
mystery.o**. If you do so by mistake, please recover it from the exam zip file
as your problem will not compile without it.

The function `is_good_child` takes two arguments:

  1. A given integer, represent a process ID.
  2. A verbose flag. If that flag is `1`, `is_good_child` will print some
     verbose information, while if it is `0`, it will not!

Given a `pid`, `is_good_child` returns a Boolean (`1` for being a good child,
`0` for not). However, `is_good_child` contains a randomness component. That
means that calling `is_good_child` twice with the **same arguments** is not
guaranteed to generate the same result! It might or might not, depending on the
randomness state.

## The problem

Our goal in this problem is to **generate a good child process** after which
both parent and the good child will _enter infinite loops_. If, while trying to
spawn a good child, we encounter a bad child (i.e., `is_good_child` returns
`0`), that child should immediately exit and the parent should make sure that
no zombies from bad children are generated.

In other words, the parent should keep generating child processes. Each child
process will check if it is a good child using (`is_good_child`).

  - If it is a good child, it should print `Good child <pid> found, will go
    into an infinite loop!` where `<pid>` is its own process ID, and then go
    into an infinite loop (i.e., `while(1);`).

  - If it is not a good child, it should print `Bad child <pid>, too bad!`
    where `<pid>` is its own process ID, and then __immediately exit__ so that
    the parent can try again.

    The parent in this case should make sure that no zombies are generated from
    this step.

Note that we cannot know beforehand how many bad children we might generate
before finding a good child, as `is_good_child` contains an element of
randomness.

### The challenge

The challenge here is that calling `is_good_child(child_pid, 1)` from the child
and `is_good_child(child_pid, 1)` from the parent is **NOT GUARANTEED** to
return the same value, even though `child_pid` is the same in both!

Your design must find a way to synchronize the decision of both the parent and
the child so that the parent can decide if it has found a good child or not.

Again, there are many ways to achieve this. It is up to you to use the tools we
have learned in this class to implement it.

### The requirements

- The parent should keep creating children until it finds a good child.

- A good child should print `Good child <pid> found, will go into an infinite
  loop!` and then enter into an infinite loop.

- After finding a good child, the parent should print `Parent <pid> found a
  good child <child pid>, will go into an infinite loop!` and then enter the
  infinite loop.

- A bad child should print `Bad child <pid>, too bad!` and then exit.

- Your implementation **should not generate any zombies**.

### Sample run

Here is a sample run of my implementation (with `is_good_child` printing
verbose information):

```sh
$ ./problem3.bin
Process 25758 is not good!
Bad child 25758, too bad!
Process 25759 is not good!
Bad child 25759, too bad!
Process 25760 is not good!
Bad child 25760, too bad!
Process 25761 is good!
Good child 25761 found, will go into an infinite loop!
Parent 25757 found a good child 25761, will go into an inifnite loop!
```

Here's another run to see that the behavior depends on the randomness of
`is_good_child`:

```sh
$ ./problem3.bin
Process 26419 is not good!
Bad child 26419, too bad!
Process 26420 is not good!
Bad child 26420, too bad!
Process 26421 is not good!
Bad child 26421, too bad!
Process 26422 is not good!
Bad child 26422, too bad!
Process 26423 is not good!
Bad child 26423, too bad!
Process 26424 is not good!
Bad child 26424, too bad!
Process 26425 is not good!
Bad child 26425, too bad!
Process 26426 is not good!
Bad child 26426, too bad!
Process 26427 is good!
Good child 26427 found, will go into an infinite loop!
Parent 26418 found a good child 26427, will go into an inifnite loop!
```

## Submission

Please write your code in `problem3.c` and describe your design in
`problem3.txt`. If you make any assumptions, please make sure to mention those
in your design.

