# `<xstd/bits/bit_span.hpp>`

```cpp
namespace xstd {
  // [views.span], class template span
  template<bit_blocks Blocks, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_span;
  // beside it: the window its first, last and subspan return
  template<bit_blocks Blocks, size_t Extent = dynamic_extent, size_t N = bit_blocks_extent_v<Blocks>>
    class bit_subspan;
}

namespace std::ranges {
  template<class Blocks, size_t N>
    constexpr bool enable_view<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_span<Blocks, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_view<xstd::bit_subspan<Blocks, Extent, N>> = true;
  template<class Blocks, size_t Extent, size_t N>
    constexpr bool enable_borrowed_range<xstd::bit_subspan<Blocks, Extent, N>> = true;
}

namespace boost::container_hash {
  template<class Blocks, size_t N>
    struct is_range<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t N>
    struct is_tuple_like<xstd::bit_span<Blocks, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_range<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
  template<class Blocks, size_t Extent, size_t N>
    struct is_tuple_like<xstd::bit_subspan<Blocks, Extent, N>> : false_type {};
}
```

| Header | Declares |
| :----- | :------- |
| `<xstd/bits/bit_span/bit_span.hpp>` | `bit_span` |
| `<xstd/bits/bit_span/bit_subspan.hpp>` | `bit_subspan` |
