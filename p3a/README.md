# CS537-Project-1


This is a solution to the [original problem](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/concurrency-pzip):

### Overview:
1. The program is designed to follow `batch framework`, where we have 3 components, i.e. reader, processor, writer
2. We have single threaded `reader`, multi-threaded `processor` and single threaded `writer`.
3. `reader` reads from one file at a time, page by page, and enqueue items for `processor`.
4. `processor` dequeue items and perform RLE compression. It later stores items into the dedicated multi-dimensional array for `writer`.
5. `writer` waits for `reader` and `processor` to finish their respective tasks and then writes the output. While writing, it also performs merge where last page last character count is merged with next page first character count. This is a big reason why we have to wait for `reader` and `processor` to complete their tasks first.

### Misc:
1. We have used a circular queue with size `N`, that can hold `N-1` items at a time. With this, we are trading off 1 item space with better readability, since we don't need to maintain `size` variable and boundary conditions can be checked easily.
2. For `mmap()`, we are using the larger `length`, because this way we can achieve better concurrency, since most of the time will be spent doing RLE compression.
