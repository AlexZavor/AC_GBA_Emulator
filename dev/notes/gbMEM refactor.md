# GB Mem

I don't know why I'm so into documentation suddenly. but here I go.

I want to upgrade the GB mem system that is currently one of the main causes for slow frames in my system. And to be a refresher before I continue on with the GBA emulator.

1. The GB mem system is currently a C++ object, and I would rather have it just be a C file
2. the MEM system currently does really stupid memcpy() functions all the time, when I could just be a bit smarter with pointers.
3. More error checking would be nice and maybe save some data from the unused sections of memory. this will be much more important for the GBA.

### Benchmarking

First we should benchmark to make sure that our improvements actually, you know, improve things. Just in case i don't make any difference or even make things worse if the compiler is smarter than me. Which happens a lot.

For this benchmark I'll run 3 games for 11 seconds, set the frame timer buffer for 600 frames, and record the min, max and average frame times in ms for each, 3 times, for a total of 21 measurements. on my laptop, unplugged, with just vscode and firefox in the background idling.

```bash
GAME      |  MIN 1  |  MIN 2  |  MIN 3  |  MAX 1  |  MAX 2  |  MAX 3  |  AVG 1  |  AVG 2  |  AVG 3  |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
JAYRO.GB  |  2.114  |  1.979  |  1.874  | 10.328  |  9.397  |  9.573  |  6.399  |  6.122  |  6.086  |
DEADEUS.GB|  2.050  |  2.669  |  2.062  | 22.819  | 12.043  | 10.707  |  8.043  |  8.297  |  8.232  |
ZELDA.GB  |  1.870  |  1.637  |  1.559  |  9.717  | 14.404  |  8.773  |  5.336  |  5.308  |  5.381  |
```
(10/20/2025) data

The main hope is to improve the average and max times. possibly at the sacrifice of some of the minimum times due to pointer overhead.

### C refactor

After fixing the code to just be C without changing any of the functionality, it seems like it is... significantly slower. by about 2-4 ms on all fronts. which isn't great. but! we can make optimizations still hopefully. inline functions, simplifications, and getting rid of the costly bank swapping!... hopefully faster anyway. otherwise that was a lot of work to get rid of direct memory accesses.

### pointer changes

I'm still disappointed. fixing it only made it slower. having to do more checks for every read is. really slow. I have a few ideas that could help? but I think honestly just the stack frame is slow enough to make a difference. So I want to change one idea. then i might go back to direct memory access with inline functions.

### Inline Functions

So I moved to inline. and I got a lot of the time back. leaving in the memcpy calls as a sort of cache. the direct memory access should be faster again but, yet, here I am, finding it's still about 2-4 ms slower. I don't fully understand. but. You know. it doesn't need to be the best code here. I guess to improve more I need like, faster memory access. the read is about as fast as I can get other than the memcpy calls. and the write is pretty darn close too. but it still just runs slow. but also I have a trashy laptop. I think I'd like to see how it performs on my desktop or something. It is overall good. and I think I learned some good tricks for the GBA emulator. but I hope I don't keep slowing it down going from C++ to C. That would actually make me. pretty sad.

### Final Benchmark

```bash
GAME      |  MIN 1  |  MIN 2  |  MIN 3  |  MAX 1  |  MAX 2  |  MAX 3  |  AVG 1  |  AVG 2  |  AVG 3  |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
JAYRO.GB  |  3.492  |  3.567  |  2.571  | 13.601  | 11.552  | 11.588  |  9.130  |  9.209  |  8.940  |
DEADEUS.GB|  3.520  |  3.801  |  3.405  | 16.207  | 14.976  | 14.749  | 11.374  | 11.734  | 10.934  |
ZELDA.GB  |  2.290  |  3.117  |  4.240  | 24.122  | 11.564  | 14.673  |  8.015  |  8.044  |  8.108  |
```
(10/25/2025) data

### Thoughts

Well. I'm not sure how I did it. it should be about the same code. I guess I'm doing more checks each time. many of the writes were just done directly rather than checking against addresses. but like. it didn't matter much because many of them were direct. so now all of those are slower by a few cycles to check. but I didn't think it would make that much difference. Maybe in the future I have two inline functions, one with protections, and one without. if it's a known address to write to then it doesn't need to check the area written to. Not a bad idea but hard to implement

### Afterthought

Macros. I know it's not the smartest. but I made the gb_read a macro instead. and it is in fact faster by a measure. bringing it back to about the speed I had before. I guess inline functions are slower, possibly because they still need to make a new scope? still return a value? anyways. I guess I need yet another benchmark. nah. its just pretty close to the original. but now all C instead of C++ and more, stable I think