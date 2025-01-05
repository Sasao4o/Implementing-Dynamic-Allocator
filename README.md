# Implementing Dynamic Allocator

This project is part of the **Computer Systems: A Programmer's Perspective** Malloc Assignment.

---

## Task Breakdown
The implementation will be divided into the following subtasks:

### First Version Features:
- **Alignment**: Ensuring proper memory alignment.
- **Free Block Organization**: Explicit free list.
- **Placement Policy**: First Fit.
- **Splitting Policy**: No Splitting.
- **Coalescing Policy**: No Coalescing.
- **Free-List Insertion Policy**: LIFO (Last In, First Out).

### Second Version Features:
- **Alignment**: Ensuring proper memory alignment.
- **Free Block Organization**: Explicit free list.
- **Placement Policy**: First Fit.
- **Splitting Policy**: Always Split (i guess i should'nt split when the diff is too small (internal fragmentation vs external fragmentation)).
- **Coalescing Policy**: immediate Coalescing.
- **Free-List Insertion Policy**: LIFO (Last In, First Out).
---

## Questions to Consider
While working on the dynamic allocator, the following questions are essential:

1. **Can Dynamic Allocator Use MMAP?**
2. **What Does a Scalable Allocator Mean?**
3. **Throughput vs Utilization**: Which matters most, and why?
4. **What is Data Alignment, and Why is it Important?**
5. **Why might immediate coalescing be inefficient in some cases?(Hint: Consider how certain request patterns could lead to a situation where a block is repeatedly coalesced and then split again, causing unnecessary overhead and inefficiency.)**
6. **Can we use realloc in place? (Hint:stanford lec 24)**
7. **What is False Fragmentation?**




---

## Resources
Inner Workings of Malloc and Free
https://www.cs.princeton.edu/courses/archive/fall06/cos217/lectures/14Memory-2x2.pdf
Hack the VM
https://blog.holbertonschool.com/hack-the-virtual-memory-drawing-the-vm-diagram/

