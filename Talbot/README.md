# Cross Hyperthread Experiment

In this part of this repo, we implement an the automated tool summarizing all of our experiments.

## Usage

To compile and run it you can simple run

```
make test
```

It wraps the following make commands:

```
make trigger
make trcrt
make destroy
make ready
make run
```

Please refer to the Makefile for further information.

Once `make ready` was successfully run, you can just use `make run` or `sudo ./trigger` to run the tool.

## Configuration

We have not implemented support for command line arguments due to time constraints. Instead this tool can be configured by adapting values in `shared/include/definitions.h`.

In PINNEDCORE we can configure the core this tool should run on.

Additionally, you can configure the amount of mapped pages per page table level by modifying the coresponding values in the PAGE_TABLE_ENTRIES array. By default, the first page table level maps 512\*32 entries on the second-, 512, entries on the third-, and 32 entries on the fourth level translation cache. In this case the array would look like this:

```c
static const int PAGE_TABLE_ENTRIES[] = {-1, -1, 512*32, 512, 32};
```

We can also configure the amount of huge pages to be mapped with AMOUNT_2M_PAGES and AMOUNT_1G_PAGES.

Furthermore we can configure whether the tool should output minimal eviction sets, using the switch PRINT_MINIMAL_EVICTION_SET.

With ITERATIONS we can define the amount of iterations each experiment should be run. THRESHOLD configure the success threshold of the experiments.

We do not recommend changing other values than the aformentioned ones.