//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_OWNER_MEMBERS_HPP
#define XSTD_BITS_DETAIL_OWNER_MEMBERS_HPP

#include <concepts>    // swappable
#include <type_traits> // is_nothrow_swappable_v
#include <utility>     // move

namespace xstd::bits::detail {

// What an owner forwards to its storage, in either reading; a view has the names too, but none it can call.
template<class Members, class Storage, class Owner, bool Owns>
class owner_members : public Members
{
public:
        using Members::Members;

        [[nodiscard]] friend auto operator==(owner_members const&, owner_members const&) -> bool = default;

        // The non-member beside it: ranges::swap finds this and never the member.
        friend constexpr auto swap(Owner& x, Owner& y) noexcept(std::is_nothrow_swappable_v<Storage>)
                -> void
                requires Owns and std::swappable<Storage>
        {
                x.swap(y);
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr auto swap(Owner& other) noexcept(std::is_nothrow_swappable_v<Storage>)
                -> void
                requires Owns and std::swappable<Storage>
        {
                std::ranges::swap(this->m_bits, static_cast<owner_members&>(other).m_bits);
        }

        // Asking the storage, so that an owner over an allocating storage has its allocator to show.
        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires Owns and requires (Storage const& s) { s.get_allocator(); }
        {
                return this->m_bits.get_allocator();
        }

        // flat_set's door onto its representation, at a run-time width: the blocks come in and go out whole.
        constexpr auto replace(Storage::block_container_type&& blocks) noexcept(noexcept(this->m_bits.replace(std::move(blocks))))
                -> void
                requires Owns and requires (Storage& s, Storage::block_container_type&& c) { s.replace(std::move(c)); }
        {
                this->m_bits.replace(std::move(blocks));
        }

        [[nodiscard]] constexpr auto extract() && noexcept(noexcept(std::move(this->m_bits).extract()))
                -> Storage::block_container_type
                requires Owns and requires (Storage&& s) { std::move(s).extract(); }
        {
                return std::move(this->m_bits).extract();
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_OWNER_MEMBERS_HPP
