# `bit_slice`

```cpp
class bit_slice {
public:
  bit_slice() = default;
  constexpr bit_slice(size_t start, size_t size, size_t stride) noexcept;
  explicit bit_slice(const slice& s) noexcept;

  constexpr auto start() const noexcept -> size_t;
  constexpr auto size() const noexcept -> size_t;
  constexpr auto stride() const noexcept -> size_t;

  friend auto operator==(const bit_slice&, const bit_slice&) -> bool = default;
};
```

- As [[class.slice]](https://eel.is/c++draft/class.slice), usable in a constant expression, where `slice`'s
  constructors are not, so `v[bit_slice(start, size, stride)]` runs in one.
- It selects what the `slice` of the same three numbers selects.
