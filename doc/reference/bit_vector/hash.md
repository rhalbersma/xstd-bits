# `hash`, `is_range`, `is_tuple_like`

As for `basic_bit_array`, the model being `vector<bool>`, as
[[vector.bool.pspc]](https://eel.is/c++draft/vector.bool.pspc) hashes it: equal values hash equal, whatever capacity
or allocation is behind them.
