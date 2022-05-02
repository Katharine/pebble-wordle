# pebble-wordle

Imagine wordle, but on your Pebble. That's pretty much it.

Supports playing the game, stat tracking, and sharing your score. Runs
100% on-watch for the best experience.

Works best on Pebble Time and Pebble Time Steel (basalt). Works on Pebble 2 (diorite),
but the colour scheme is confusing. Basically works on the original Pebble and Pebble
Steel, but the colour scheme is confusing and the sharing QR codes aren't available.

![](https://assets2.rebble.io/exact/144x168/623146422a976b000a6e73e8)
![](https://assets2.rebble.io/exact/144x168/6231463ab442c1000a1ff260)
![](https://assets2.rebble.io/exact/144x168/62314647b442c1000a1ff261)
![](https://assets2.rebble.io/exact/144x168/6231464c2a976b000a6e73e9)
![](https://assets2.rebble.io/exact/144x168/62314650b442c1000a1ff262)

## Word lists

The word lists are stored in `resources/acceptable.dat` and `resources/solutions.dat`.
`acceptable.dat` contains all possible words, including the ones in `solutions.dat`.
`acceptable.dat` is in alphabetical order for efficient searching; breaking that
ordering will break the valid word check.

`solutions.dat` is the list of every solution word, in the order they will appear.
