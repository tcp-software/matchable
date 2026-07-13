#pragma once

/*
Copyright (c) 2019-, shtroizel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>



namespace matchable
{
    // Associative array for matchables
    template<typename M, typename T>
    class MatchBox
    {
    public:
        using match_t = M;
        using target_t = T;

        MatchBox() : MatchBox(T{}) {}
        explicit MatchBox(T const & fill);
        MatchBox(std::initializer_list<std::pair<M, T>> init_list);
        T const & at(M const &) const;
        T & mut_at(M const &);
        void set(M const &, T const &);
        void unset(M const &);
        bool is_set(M const &) const;
        std::vector<M> currently_set() const;

    private:
        T nil_target;
        bool nil_target_init_flag;
        T default_t;
        std::vector<T> targets;
        std::vector<bool> init_flags;
        using as_index_func_type = int (M::*)() const;
#ifdef MATCHABLE_OMIT_BY_INDEX
        as_index_func_type as_index_func = &M::as_by_string_index;
#else
        as_index_func_type as_index_func = &M::as_index;
#endif
    };


    template<typename M, typename T>
    MatchBox<M, T>::MatchBox(T const & fill)
        : nil_target{}
        , nil_target_init_flag{false}
        , default_t{fill}
    {
        targets.reserve(M::variants().size());
        init_flags.reserve(M::variants().size());

        for (size_t i = 0; i < M::variants().size(); ++i)
        {
            targets.push_back(default_t);
            init_flags.push_back(false);
        }
    }


    template<typename M, typename T>
    MatchBox<M, T>::MatchBox(std::initializer_list<std::pair<M, T>> init_list)
        : MatchBox()
    {
        for (auto const & elem : init_list)
            set(elem.first, elem.second);
    }


    template<typename M, typename T>
    T const & MatchBox<M, T>::at(M const & match) const
    {
        return match.is_nil() ? nil_target : targets[static_cast<size_t>((match.*as_index_func)())];
    }


    template<typename M, typename T>
    T & MatchBox<M, T>::mut_at(M const & match)
    {
        return match.is_nil() ? nil_target : targets[static_cast<size_t>((match.*as_index_func)())];
    }


    template<typename M, typename T>
    void MatchBox<M, T>::set(M const & match, T const & target)
    {
        if (match.is_nil())
        {
            nil_target = target;
            nil_target_init_flag = true;
        }
        else
        {
            size_t index = static_cast<size_t>((match.*as_index_func)());
            targets[index] = target;
            init_flags[index] = true;
        }
    }


    template<typename M, typename T>
    void MatchBox<M, T>::unset(M const & match)
    {
        if (match.is_nil())
        {
            nil_target = default_t;
            nil_target_init_flag = false;
        }
        else
        {
            size_t index = static_cast<size_t>((match.*as_index_func)());
            targets[index] = default_t;
            init_flags[index] = false;
        }
    }


    template<typename M, typename T>
    bool MatchBox<M, T>::is_set(const M & match) const
    {
        if (match.is_nil())
            return nil_target_init_flag;

        return init_flags[static_cast<size_t>((match.*as_index_func)())];
    }


    template<typename M, typename T>
    std::vector<M> MatchBox<M, T>::currently_set() const
    {
        std::vector<M> ret;
        for (auto const & m : M::variants())
            if (is_set(m))
                ret.push_back(m);
        return ret;
    }


    // MatchBox stores a bool for each element anyway for tracking explicit setting, so MatchBox<M, bool>
    // would yield 2 bools per element. To avoid the extra bool use this MatchBox<M, void> specialization
    // instead.
    template<typename M>
    class MatchBox<M, void>
    {
    public:
        using match_t = M;

        MatchBox();
        MatchBox(std::initializer_list<M> init_list);
        void clear();
        void set(M const &);
        void unset(M const &);
        void toggle(M const &);
        bool is_set(M const &) const;
        std::vector<M> currently_set() const;
        bool operator==(MatchBox const &) const;
        bool operator!=(MatchBox const &) const;
        friend std::ostream & operator<<(std::ostream & o, MatchBox<M, void> const & m)
        {
            for (auto v : M::variants())
                o << v << ": " << m.is_set(v) << ", ";
            return o << M() << ": " << m.is_set(M());
        }

    private:
        bool nil_init_flag;
        std::vector<bool> init_flags;
        using as_index_func_type = int (M::*)() const;
#ifdef MATCHABLE_OMIT_BY_INDEX
        as_index_func_type as_index_func = &M::as_by_string_index;
#else
        as_index_func_type as_index_func = &M::as_index;
#endif
    };


    template<typename M>
    MatchBox<M, void>::MatchBox() : nil_init_flag{false}
    {
        init_flags.reserve(M::variants().size());
        for (size_t i = 0; i < M::variants().size(); ++i)
            init_flags.push_back(false);
    }


    template<typename M>
    MatchBox<M, void>::MatchBox(std::initializer_list<M> init_list) : MatchBox()
    {
        for (auto const & m : init_list)
            set(m);
    }


    template<typename M>
    void MatchBox<M, void>::clear()
    {
        nil_init_flag = false;
        for (size_t i = 0; i < init_flags.size(); ++i)
            init_flags[i] = false;
    }


    template<typename M>
    void MatchBox<M, void>::set(M const & match)
    {
        if (match.is_nil())
            nil_init_flag = true;
        else
            init_flags[static_cast<size_t>((match.*as_index_func)())] = true;
    }


    template<typename M>
    void MatchBox<M, void>::unset(M const & match)
    {
        if (match.is_nil())
            nil_init_flag = false;
        else
            init_flags[static_cast<size_t>((match.*as_index_func)())] = false;
    }


    template<typename M>
    void MatchBox<M, void>::toggle(M const & match)
    {
        if (match.is_nil())
            nil_init_flag = !nil_init_flag;
        else
            init_flags[static_cast<size_t>((match.*as_index_func)())] =
                    !init_flags[static_cast<size_t>((match.*as_index_func)())];
    }


    template<typename M>
    bool MatchBox<M, void>::is_set(M const & match) const
    {
        if (match.is_nil())
            return nil_init_flag;

        return init_flags[static_cast<size_t>((match.*as_index_func)())];
    }


    template<typename M>
    std::vector<M> MatchBox<M, void>::currently_set() const
    {
        std::vector<M> ret;
        for (auto const & m : M::variants())
            if (is_set(m))
                ret.push_back(m);
        return ret;
    }


    template<typename M>
    bool MatchBox<M, void>::operator==(MatchBox<M, void> const & other) const
    {
        return init_flags == other.init_flags;
    }


    template<typename M>
    bool MatchBox<M, void>::operator!=(MatchBox<M, void> const & other) const
    {
        return !operator==(other);
    }


    class FlowControl
    {
    public:
        void cont() { c = true; }
        void brk() { b = true; }
        bool cont_requested() const { return c; }
        bool brk_requested() const { return b; }
    private:
        bool c{false};
        bool b{false};
    };


    template<typename M>
    class Unmatchable
    {
    public:
        Unmatchable(std::initializer_list<M> um)
        {
            {
                prev_variants_by_string = M::interface_type::variants_by_string();
                auto i = std::remove_if(
                             M::interface_type::by_string().begin(),
                             M::interface_type::by_string().end(),
                             [&](M m){ return std::find(um.begin(), um.end(), m) != um.end(); }
                         );
                M::interface_type::by_string().erase(i, M::interface_type::by_string().end());
            }

#ifndef MATCHABLE_OMIT_BY_INDEX
            {
                prev_variants_by_index = M::interface_type::variants_by_index();
                M::interface_type::all_by_index() = &prev_variants_by_index;
                auto i = std::remove_if(
                             M::interface_type::by_index().begin(),
                             M::interface_type::by_index().end(),
                             [&](M m){ return std::find(um.begin(), um.end(), m) != um.end(); }
                         );
                M::interface_type::by_index().erase(i, M::interface_type::by_index().end());
            }
#endif
        }
        ~Unmatchable()
        {
            M::interface_type::by_string() = prev_variants_by_string;
#ifndef MATCHABLE_OMIT_BY_INDEX
            M::interface_type::by_index() = prev_variants_by_index;
            M::interface_type::all_by_index() = &M::interface_type::by_index();
#endif
        }
    private:
        std::vector<M> prev_variants_by_string;
#ifndef MATCHABLE_OMIT_BY_INDEX
        std::vector<M> prev_variants_by_index;
#endif
    };


    inline bool str_lt_str(std::string const & l, std::string const & r)
    {
        size_t const min_len = std::min(l.length(), r.length());
        for (size_t i = 0; i < min_len; ++i)
        {
            bool l_alpha = (l[i] >= 'A' && l[i] <= 'Z') || (l[i] >= 'a' && l[i] <= 'z');
            bool r_alpha = (r[i] >= 'A' && r[i] <= 'Z') || (r[i] >= 'a' && r[i] <= 'z');
            if (l_alpha ^ r_alpha)
            {
                return r_alpha;
            }
            else
            {
                if (l[i] < r[i])
                    return true;
                else if (r[i] < l[i])
                    return false;
            }
        }
        return l.length() < r.length();
    }

} // namespace matchable


#define matchable_declare_begin(t)                                                                         \
    namespace t                                                                                            \
    {                                                                                                      \
        using Type = MatchableType<class I##t>;                                                            \
        class I##t                                                                                         \
        {                                                                                                  \
            friend class MatchableType<I##t>;                                                              \
            friend class ::matchable::Unmatchable<MatchableType<I##t>>;                                    \
        public:                                                                                            \
            I##t() = default;                                                                              \
            virtual ~I##t() = default;                                                                     \
            virtual int as_by_string_index() const = 0;                                                    \
            virtual std::string const & as_string() const = 0;                                             \
            virtual std::string const & as_identifier_string() const = 0;                                  \
            static std::vector<Type> const & variants_by_string() { return by_string(); }                  \
            static bool register_variant(Type const & variant, int * i);                                   \
        private:                                                                                           \
            virtual void set_by_string_index(int index) = 0;                                               \
            virtual std::shared_ptr<I##t> clone() const = 0;                                               \
            static std::vector<Type> & by_string() { static std::vector<Type> v; return v; }


#ifdef MATCHABLE_OMIT_BY_INDEX
#define matchable_declare_end(t)                                                                           \
        public:                                                                                            \
            static std::vector<Type> const & variants() { return variants_by_string(); }                   \
        };                                                                                                 \
    }
#else
#define matchable_declare_end(t)                                                                           \
        public:                                                                                            \
            virtual int as_index() const = 0;                                                              \
            static std::vector<Type> const & variants() { return variants_by_index(); }                    \
            static std::vector<Type> const & variants_by_index() { return by_index(); }                    \
            static std::vector<Type> const * all_variants_by_index() { return all_by_index(); }            \
        private:                                                                                           \
            static std::vector<Type> & by_index() { static std::vector<Type> v; return v; }                \
            static std::vector<Type> *& all_by_index()                                                     \
                    { static std::vector<Type> * v = &by_index(); return v; }                              \
        };                                                                                                 \
    }
#endif



#define matchable_create_type_begin(t)                                                                     \
    namespace t                                                                                            \
    {                                                                                                      \
        template<class T>                                                                                  \
        class MatchableType                                                                                \
        {                                                                                                  \
        public:                                                                                            \
            using interface_type = T;                                                                      \
            using MatchParam = ::matchable::MatchBox<MatchableType, std::function<void()>>;                \
            using MatchParamWithFlowControl =                                                              \
                    ::matchable::MatchBox<MatchableType, std::function<void(matchable::FlowControl &)>>;   \
            MatchableType() = default;                                                                     \
            ~MatchableType() = default;                                                                    \
            MatchableType(std::shared_ptr<T> m) : t{m} {}                                                  \
            MatchableType(MatchableType const & o) : t{nullptr == o.t ? nullptr : o.t->clone()} {}         \
            MatchableType(MatchableType &&) = default;                                                     \
            MatchableType & operator=(MatchableType const & other)                                         \
            {                                                                                              \
                if (this != &other)                                                                        \
                    t = nullptr == other.t ? nullptr : other.t->clone();                                   \
                return *this;                                                                              \
            }                                                                                              \
            MatchableType & operator=(MatchableType &&) = default;                                         \
            std::string const & as_string() const                                                          \
            {                                                                                              \
                static std::string const nil_str{"nil"};                                                   \
                return nullptr == t ? nil_str : t->as_string();                                            \
            }                                                                                              \
            std::string as_identifier_string() const                                                       \
            {                                                                                              \
                static std::string const nil_str{"nil"};                                                   \
                return nullptr == t ? nil_str : t->as_identifier_string();                                 \
            }                                                                                              \
            int as_by_string_index() const { return nullptr == t ? -1 : t->as_by_string_index(); }         \
            bool is_nil() const { return nullptr == t; }                                                   \
            matchable::FlowControl match(MatchParamWithFlowControl const & mb) const                       \
            {                                                                                              \
                matchable::FlowControl lc;                                                                 \
                if (mb.is_set(*this))                                                                      \
                    mb.at(*this)(lc);                                                                      \
                return lc;                                                                                 \
            }                                                                                              \
            void match(MatchParam const & mb) const                                                        \
            {                                                                                              \
                if (mb.is_set(*this))                                                                      \
                    mb.at(*this)();                                                                        \
            }                                                                                              \
            static std::vector<MatchableType> const & variants() { return T::variants(); }                 \
            static std::vector<MatchableType> const & variants_by_string()                                 \
                { return T::variants_by_string(); }                                                        \
            bool operator==(MatchableType const & m) const { return as_string() == m.as_string(); }        \
            bool operator!=(MatchableType const & m) const { return as_string() != m.as_string(); }        \
            bool lt_by_string(MatchableType const & m) const                                               \
                    { return matchable::str_lt_str(as_string(), m.as_string()); }                          \
            bool lt_by_string(std::string const & str) const                                               \
                    { return matchable::str_lt_str(as_string(), str); }                                    \
            friend std::ostream & operator<<(std::ostream & o, MatchableType const & m)                    \
            {                                                                                              \
                return o << m.as_string();                                                                 \
            }                                                                                              \
            void set_by_string_index(int index) { if (nullptr != t) t->set_by_string_index(index); }       \
        private:                                                                                           \
            std::shared_ptr<T> t;


#ifdef MATCHABLE_OMIT_BY_INDEX
#define matchable_create_type_end(t)                                                                       \
        public:                                                                                            \
            bool operator<(MatchableType const & m) const                                                  \
                { return as_by_string_index() < m.as_by_string_index(); }                                  \
        };                                                                                                 \
    }
#else
#define matchable_create_type_end(t)                                                                       \
        public:                                                                                            \
            int as_index() const { return nullptr == t ? -1 : t->as_index(); }                             \
            bool lt_by_index(MatchableType const & m) const { return as_index() < m.as_index(); }          \
            bool operator<(MatchableType const & m) const { return as_index() < m.as_index(); }            \
        };                                                                                                 \
    }
#endif


#define matchable_define_common(t)                                                                         \
    namespace t                                                                                            \
    {                                                                                                      \
        using Flags = matchable::MatchBox<Type, void>;                                                     \
        inline std::vector<Type> const & variants() { return I##t::variants(); }                           \
        inline std::vector<Type> const & variants_by_string() { return I##t::variants_by_string(); }       \
        [[maybe_unused]] static std::string const name{#t};                                                \
        static Type nil{};                                                                                 \
        inline Type from_by_string_index(int index)                                                        \
        {                                                                                                  \
            if (index < 0 || index >= (int) I##t::variants_by_string().size())                             \
                return nil;                                                                                \
            return I##t::variants_by_string().at(index);                                                   \
        }                                                                                                  \
        inline int variants_by_string_index_of(std::string const & str, bool * found)                      \
        {                                                                                                  \
            auto it = std::lower_bound(                                                                    \
                I##t::variants_by_string().begin(),                                                        \
                I##t::variants_by_string().end(),                                                          \
                str,                                                                                       \
                [](t::Type const & v, std::string const & s){ return v.lt_by_string(s); }                  \
            );                                                                                             \
            if (it == I##t::variants_by_string().end())                                                    \
            {                                                                                              \
                if (nullptr != found)                                                                      \
                    *found = false;                                                                        \
                return I##t::variants_by_string().size();                                                  \
            }                                                                                              \
            if (nullptr != found)                                                                          \
                *found = str == it->as_string();                                                           \
            return it->as_by_string_index();                                                               \
        }                                                                                                  \
        inline Type from_string(std::string const & str)                                                   \
        {                                                                                                  \
            auto it = std::lower_bound(                                                                    \
                I##t::variants_by_string().begin(),                                                        \
                I##t::variants_by_string().end(),                                                          \
                str,                                                                                       \
                [](t::Type const & v, std::string const & s){ return v.lt_by_string(s); }                  \
            );                                                                                             \
            if (it != I##t::variants_by_string().end() && str == it->as_string())                          \
                return *it;                                                                                \
            return nil;                                                                                    \
        }                                                                                                  \
        inline Type from_identifier_string(std::string const & str)                                        \
        {                                                                                                  \
            for (auto v : variants())                                                                      \
                if (v.as_identifier_string() == str)                                                       \
                    return v;                                                                              \
            return Type{};                                                                                 \
        }                                                                                                  \
        [[maybe_unused]] static bool const register_##t = I##t::register_variant(nil, nullptr);            \
    }


#define matchable_define_register_variant_common                                                           \
            if (variant.is_nil())                                                                          \
            {                                                                                              \
                by_string().clear();                                                                       \
            }                                                                                              \
            else                                                                                           \
            {                                                                                              \
                if (nullptr != index)                                                                      \
                    *index = static_cast<int>(variants().size());                                          \
                static auto pred = [](Type const & a, Type const & b) { return a.lt_by_string(b); };       \
                by_string().insert(                                                                        \
                    std::upper_bound(by_string().begin(), by_string().end(), variant, pred),               \
                    variant                                                                                \
                );                                                                                         \
                for (int i = 0; i < (int) by_string().size(); ++i)                                         \
                    by_string()[i].set_by_string_index(i);                                                 \
            }


#ifdef MATCHABLE_OMIT_BY_INDEX
#define matchable_define(t)                                                                                \
    matchable_define_common(t)                                                                             \
    namespace t                                                                                            \
    {                                                                                                      \
        inline bool I##t::register_variant(Type const & variant, int * index)                              \
        {                                                                                                  \
            matchable_define_register_variant_common                                                       \
            return true;                                                                                   \
        }                                                                                                  \
    }
#else
#define matchable_define(t)                                                                                \
    matchable_define_common(t)                                                                             \
    namespace t                                                                                            \
    {                                                                                                      \
        inline bool I##t::register_variant(Type const & variant, int * index)                              \
        {                                                                                                  \
            matchable_define_register_variant_common                                                       \
            variant.is_nil() ? by_index().clear() : by_index().push_back(variant);                         \
            return true;                                                                                   \
        }                                                                                                  \
        inline std::vector<Type> const & variants_by_index() { return I##t::variants_by_index(); }         \
        inline Type from_index(int index)                                                                  \
        {                                                                                                  \
            if (index < 0 || index >= (int) I##t::all_variants_by_index()->size())                         \
                return nil;                                                                                \
            if (I##t::all_variants_by_index() != &I##t::variants_by_index())                               \
                return from_string(I##t::all_variants_by_index()->at(index).as_string());                  \
            return I##t::all_variants_by_index()->at(index);                                               \
        }                                                                                                  \
    }
#endif


#define matchable_create_variant_begin(t, v)                                                               \
    namespace t                                                                                            \
    {                                                                                                      \
        class v : public I##t                                                                              \
        {                                                                                                  \
        public:                                                                                            \
            v() = default;                                                                                 \
            int as_by_string_index() const override { return *m_by_string_index(); }                       \
            std::string const & as_string() const override                                                 \
            {                                                                                              \
                static std::string const s =                                                               \
                    [&]()                                                                                  \
                    {                                                                                      \
                        std::string s{#v};                                                                 \
                        if (t::name == "escapable")                                                        \
                            return s;                                                                      \
                        return matchable::escapable::unescape_all(s);                                      \
                    }();                                                                                   \
                return s;                                                                                  \
            }                                                                                              \
            std::string const & as_identifier_string() const override                                      \
                { static std::string const s{#v}; return s; }                                              \
            void set_by_string_index(int index) override { *m_by_string_index() = index; }                 \
            static Type grab() { return Type(create()); }                                                  \
            static int * m_by_string_index() { static int i{-1}; return &i; }                              \
        private:                                                                                           \
            std::shared_ptr<I##t> clone() const  override { return create(); }                             \
            static std::shared_ptr<v> create() { return std::make_shared<v>(); }


#ifdef MATCHABLE_OMIT_BY_INDEX
#define matchable_create_variant_end(t, v)                                                                 \
        };                                                                                                 \
        [[maybe_unused]] static bool const register_me_##t##v =                                            \
            I##t::register_variant(v::grab(), nullptr);                                                    \
    }
#else
#define matchable_create_variant_end(t, v)                                                                 \
        public:                                                                                            \
            int as_index() const override { return *m_index(); }                                           \
            static int * m_index() { static int i{-1}; return &i; }                                        \
        };                                                                                                 \
        [[maybe_unused]] static bool const register_me_##t##v =                                            \
            I##t::register_variant(v::grab(), v::m_index());                                               \
    }
#endif


#define matchable_create_variant(t, v)                                                                     \
    matchable_create_variant_begin(t, v)                                                                   \
    matchable_create_variant_end(t, v)


#define matchable_concat_variant(t, v) t::v::grab(),


// these "foreach" macros call a macro expecting a type and variant, for each variant given
//
// m is the macro to call (matchable_create_variant or matchable_concat_variant)
// t is the type
// v is the variant
// ... for more variants
#define   mcv_0(m, t, ...)
#define   mcv_1(m, t, v)      m(t, v)
#define   mcv_2(m, t, v, ...) m(t, v)   mcv_1(m, t, __VA_ARGS__)
#define   mcv_3(m, t, v, ...) m(t, v)   mcv_2(m, t, __VA_ARGS__)
#define   mcv_4(m, t, v, ...) m(t, v)   mcv_3(m, t, __VA_ARGS__)
#define   mcv_5(m, t, v, ...) m(t, v)   mcv_4(m, t, __VA_ARGS__)
#define   mcv_6(m, t, v, ...) m(t, v)   mcv_5(m, t, __VA_ARGS__)
#define   mcv_7(m, t, v, ...) m(t, v)   mcv_6(m, t, __VA_ARGS__)
#define   mcv_8(m, t, v, ...) m(t, v)   mcv_7(m, t, __VA_ARGS__)
#define   mcv_9(m, t, v, ...) m(t, v)   mcv_8(m, t, __VA_ARGS__)
#define  mcv_10(m, t, v, ...) m(t, v)   mcv_9(m, t, __VA_ARGS__)
#define  mcv_11(m, t, v, ...) m(t, v)  mcv_10(m, t, __VA_ARGS__)
#define  mcv_12(m, t, v, ...) m(t, v)  mcv_11(m, t, __VA_ARGS__)
#define  mcv_13(m, t, v, ...) m(t, v)  mcv_12(m, t, __VA_ARGS__)
#define  mcv_14(m, t, v, ...) m(t, v)  mcv_13(m, t, __VA_ARGS__)
#define  mcv_15(m, t, v, ...) m(t, v)  mcv_14(m, t, __VA_ARGS__)
#define  mcv_16(m, t, v, ...) m(t, v)  mcv_15(m, t, __VA_ARGS__)
#define  mcv_17(m, t, v, ...) m(t, v)  mcv_16(m, t, __VA_ARGS__)
#define  mcv_18(m, t, v, ...) m(t, v)  mcv_17(m, t, __VA_ARGS__)
#define  mcv_19(m, t, v, ...) m(t, v)  mcv_18(m, t, __VA_ARGS__)
#define  mcv_20(m, t, v, ...) m(t, v)  mcv_19(m, t, __VA_ARGS__)
#define  mcv_21(m, t, v, ...) m(t, v)  mcv_20(m, t, __VA_ARGS__)
#define  mcv_22(m, t, v, ...) m(t, v)  mcv_21(m, t, __VA_ARGS__)
#define  mcv_23(m, t, v, ...) m(t, v)  mcv_22(m, t, __VA_ARGS__)
#define  mcv_24(m, t, v, ...) m(t, v)  mcv_23(m, t, __VA_ARGS__)
#define  mcv_25(m, t, v, ...) m(t, v)  mcv_24(m, t, __VA_ARGS__)
#define  mcv_26(m, t, v, ...) m(t, v)  mcv_25(m, t, __VA_ARGS__)
#define  mcv_27(m, t, v, ...) m(t, v)  mcv_26(m, t, __VA_ARGS__)
#define  mcv_28(m, t, v, ...) m(t, v)  mcv_27(m, t, __VA_ARGS__)
#define  mcv_29(m, t, v, ...) m(t, v)  mcv_28(m, t, __VA_ARGS__)
#define  mcv_30(m, t, v, ...) m(t, v)  mcv_29(m, t, __VA_ARGS__)
#define  mcv_31(m, t, v, ...) m(t, v)  mcv_30(m, t, __VA_ARGS__)
#define  mcv_32(m, t, v, ...) m(t, v)  mcv_31(m, t, __VA_ARGS__)
#define  mcv_33(m, t, v, ...) m(t, v)  mcv_32(m, t, __VA_ARGS__)
#define  mcv_34(m, t, v, ...) m(t, v)  mcv_33(m, t, __VA_ARGS__)
#define  mcv_35(m, t, v, ...) m(t, v)  mcv_34(m, t, __VA_ARGS__)
#define  mcv_36(m, t, v, ...) m(t, v)  mcv_35(m, t, __VA_ARGS__)
#define  mcv_37(m, t, v, ...) m(t, v)  mcv_36(m, t, __VA_ARGS__)
#define  mcv_38(m, t, v, ...) m(t, v)  mcv_37(m, t, __VA_ARGS__)
#define  mcv_39(m, t, v, ...) m(t, v)  mcv_38(m, t, __VA_ARGS__)
#define  mcv_40(m, t, v, ...) m(t, v)  mcv_39(m, t, __VA_ARGS__)
#define  mcv_41(m, t, v, ...) m(t, v)  mcv_40(m, t, __VA_ARGS__)
#define  mcv_42(m, t, v, ...) m(t, v)  mcv_41(m, t, __VA_ARGS__)
#define  mcv_43(m, t, v, ...) m(t, v)  mcv_42(m, t, __VA_ARGS__)
#define  mcv_44(m, t, v, ...) m(t, v)  mcv_43(m, t, __VA_ARGS__)
#define  mcv_45(m, t, v, ...) m(t, v)  mcv_44(m, t, __VA_ARGS__)
#define  mcv_46(m, t, v, ...) m(t, v)  mcv_45(m, t, __VA_ARGS__)
#define  mcv_47(m, t, v, ...) m(t, v)  mcv_46(m, t, __VA_ARGS__)
#define  mcv_48(m, t, v, ...) m(t, v)  mcv_47(m, t, __VA_ARGS__)
#define  mcv_49(m, t, v, ...) m(t, v)  mcv_48(m, t, __VA_ARGS__)
#define  mcv_50(m, t, v, ...) m(t, v)  mcv_49(m, t, __VA_ARGS__)
#define  mcv_51(m, t, v, ...) m(t, v)  mcv_50(m, t, __VA_ARGS__)
#define  mcv_52(m, t, v, ...) m(t, v)  mcv_51(m, t, __VA_ARGS__)
#define  mcv_53(m, t, v, ...) m(t, v)  mcv_52(m, t, __VA_ARGS__)
#define  mcv_54(m, t, v, ...) m(t, v)  mcv_53(m, t, __VA_ARGS__)
#define  mcv_55(m, t, v, ...) m(t, v)  mcv_54(m, t, __VA_ARGS__)
#define  mcv_56(m, t, v, ...) m(t, v)  mcv_55(m, t, __VA_ARGS__)
#define  mcv_57(m, t, v, ...) m(t, v)  mcv_56(m, t, __VA_ARGS__)
#define  mcv_58(m, t, v, ...) m(t, v)  mcv_57(m, t, __VA_ARGS__)
#define  mcv_59(m, t, v, ...) m(t, v)  mcv_58(m, t, __VA_ARGS__)
#define  mcv_60(m, t, v, ...) m(t, v)  mcv_59(m, t, __VA_ARGS__)
#define  mcv_61(m, t, v, ...) m(t, v)  mcv_60(m, t, __VA_ARGS__)
#define  mcv_62(m, t, v, ...) m(t, v)  mcv_61(m, t, __VA_ARGS__)
#define  mcv_63(m, t, v, ...) m(t, v)  mcv_62(m, t, __VA_ARGS__)
#define  mcv_64(m, t, v, ...) m(t, v)  mcv_63(m, t, __VA_ARGS__)
#define  mcv_65(m, t, v, ...) m(t, v)  mcv_64(m, t, __VA_ARGS__)
#define  mcv_66(m, t, v, ...) m(t, v)  mcv_65(m, t, __VA_ARGS__)
#define  mcv_67(m, t, v, ...) m(t, v)  mcv_66(m, t, __VA_ARGS__)
#define  mcv_68(m, t, v, ...) m(t, v)  mcv_67(m, t, __VA_ARGS__)
#define  mcv_69(m, t, v, ...) m(t, v)  mcv_68(m, t, __VA_ARGS__)
#define  mcv_70(m, t, v, ...) m(t, v)  mcv_69(m, t, __VA_ARGS__)
#define  mcv_71(m, t, v, ...) m(t, v)  mcv_70(m, t, __VA_ARGS__)
#define  mcv_72(m, t, v, ...) m(t, v)  mcv_71(m, t, __VA_ARGS__)
#define  mcv_73(m, t, v, ...) m(t, v)  mcv_72(m, t, __VA_ARGS__)
#define  mcv_74(m, t, v, ...) m(t, v)  mcv_73(m, t, __VA_ARGS__)
#define  mcv_75(m, t, v, ...) m(t, v)  mcv_74(m, t, __VA_ARGS__)
#define  mcv_76(m, t, v, ...) m(t, v)  mcv_75(m, t, __VA_ARGS__)
#define  mcv_77(m, t, v, ...) m(t, v)  mcv_76(m, t, __VA_ARGS__)
#define  mcv_78(m, t, v, ...) m(t, v)  mcv_77(m, t, __VA_ARGS__)
#define  mcv_79(m, t, v, ...) m(t, v)  mcv_78(m, t, __VA_ARGS__)
#define  mcv_80(m, t, v, ...) m(t, v)  mcv_79(m, t, __VA_ARGS__)
#define  mcv_81(m, t, v, ...) m(t, v)  mcv_80(m, t, __VA_ARGS__)
#define  mcv_82(m, t, v, ...) m(t, v)  mcv_81(m, t, __VA_ARGS__)
#define  mcv_83(m, t, v, ...) m(t, v)  mcv_82(m, t, __VA_ARGS__)
#define  mcv_84(m, t, v, ...) m(t, v)  mcv_83(m, t, __VA_ARGS__)
#define  mcv_85(m, t, v, ...) m(t, v)  mcv_84(m, t, __VA_ARGS__)
#define  mcv_86(m, t, v, ...) m(t, v)  mcv_85(m, t, __VA_ARGS__)
#define  mcv_87(m, t, v, ...) m(t, v)  mcv_86(m, t, __VA_ARGS__)
#define  mcv_88(m, t, v, ...) m(t, v)  mcv_87(m, t, __VA_ARGS__)
#define  mcv_89(m, t, v, ...) m(t, v)  mcv_88(m, t, __VA_ARGS__)
#define  mcv_90(m, t, v, ...) m(t, v)  mcv_89(m, t, __VA_ARGS__)
#define  mcv_91(m, t, v, ...) m(t, v)  mcv_90(m, t, __VA_ARGS__)
#define  mcv_92(m, t, v, ...) m(t, v)  mcv_91(m, t, __VA_ARGS__)
#define  mcv_93(m, t, v, ...) m(t, v)  mcv_92(m, t, __VA_ARGS__)
#define  mcv_94(m, t, v, ...) m(t, v)  mcv_93(m, t, __VA_ARGS__)
#define  mcv_95(m, t, v, ...) m(t, v)  mcv_94(m, t, __VA_ARGS__)
#define  mcv_96(m, t, v, ...) m(t, v)  mcv_95(m, t, __VA_ARGS__)
#define  mcv_97(m, t, v, ...) m(t, v)  mcv_96(m, t, __VA_ARGS__)
#define  mcv_98(m, t, v, ...) m(t, v)  mcv_97(m, t, __VA_ARGS__)
#define  mcv_99(m, t, v, ...) m(t, v)  mcv_98(m, t, __VA_ARGS__)
#define mcv_100(m, t, v, ...) m(t, v)  mcv_99(m, t, __VA_ARGS__)
#define mcv_101(m, t, v, ...) m(t, v) mcv_100(m, t, __VA_ARGS__)
#define mcv_102(m, t, v, ...) m(t, v) mcv_101(m, t, __VA_ARGS__)
#define mcv_103(m, t, v, ...) m(t, v) mcv_102(m, t, __VA_ARGS__)
#define mcv_104(m, t, v, ...) m(t, v) mcv_103(m, t, __VA_ARGS__)
#define mcv_105(m, t, v, ...) m(t, v) mcv_104(m, t, __VA_ARGS__)
#define mcv_106(m, t, v, ...) m(t, v) mcv_105(m, t, __VA_ARGS__)
#define mcv_107(m, t, v, ...) m(t, v) mcv_106(m, t, __VA_ARGS__)
#define mcv_108(m, t, v, ...) m(t, v) mcv_107(m, t, __VA_ARGS__)


#define nth(                                                                                               \
      n_0,   n_1,   n_2,   n_3,   n_4,   n_5,   n_6,   n_7,   n_8,  n_9,                                   \
     n_10,  n_11,  n_12,  n_13,  n_14,  n_15,  n_16,  n_17,  n_18, n_19,                                   \
     n_20,  n_21,  n_22,  n_23,  n_24,  n_25,  n_26,  n_27,  n_28, n_29,                                   \
     n_30,  n_31,  n_32,  n_33,  n_34,  n_35,  n_36,  n_37,  n_38, n_39,                                   \
     n_40,  n_41,  n_42,  n_43,  n_44,  n_45,  n_46,  n_47,  n_48, n_49,                                   \
     n_50,  n_51,  n_52,  n_53,  n_54,  n_55,  n_56,  n_57,  n_58, n_59,                                   \
     n_60,  n_61,  n_62,  n_63,  n_64,  n_65,  n_66,  n_67,  n_68, n_69,                                   \
     n_70,  n_71,  n_72,  n_73,  n_74,  n_75,  n_76,  n_77,  n_78, n_79,                                   \
     n_80,  n_81,  n_82,  n_83,  n_84,  n_85,  n_86,  n_87,  n_88, n_89,                                   \
     n_90,  n_91,  n_92,  n_93,  n_94,  n_95,  n_96,  n_97,  n_98, n_99,                                   \
    n_100, n_101, n_102, n_103, n_104, n_105, n_106, n_107, n_108,    N, ...                               \
) N


#define mcv(_macro, t, ...)                                                                                \
    nth(                                                                                                   \
        "shtroizel",                                                                                       \
        ##__VA_ARGS__,                                                                                     \
                 mcv_108,mcv_107,mcv_106,mcv_105,mcv_104,mcv_103,mcv_102,mcv_101,mcv_100,                  \
         mcv_99,  mcv_98, mcv_97, mcv_96, mcv_95, mcv_94, mcv_93, mcv_92, mcv_91, mcv_90,                  \
         mcv_89,  mcv_88, mcv_87, mcv_86, mcv_85, mcv_84, mcv_83, mcv_82, mcv_81, mcv_80,                  \
         mcv_79,  mcv_78, mcv_77, mcv_76, mcv_75, mcv_74, mcv_73, mcv_72, mcv_71, mcv_70,                  \
         mcv_69,  mcv_68, mcv_67, mcv_66, mcv_65, mcv_64, mcv_63, mcv_62, mcv_61, mcv_60,                  \
         mcv_59,  mcv_58, mcv_57, mcv_56, mcv_55, mcv_54, mcv_53, mcv_52, mcv_51, mcv_50,                  \
         mcv_49,  mcv_48, mcv_47, mcv_46, mcv_45, mcv_44, mcv_43, mcv_42, mcv_41, mcv_40,                  \
         mcv_39,  mcv_38, mcv_37, mcv_36, mcv_35, mcv_34, mcv_33, mcv_32, mcv_31, mcv_30,                  \
         mcv_29,  mcv_28, mcv_27, mcv_26, mcv_25, mcv_24, mcv_23, mcv_22, mcv_21, mcv_20,                  \
         mcv_19,  mcv_18, mcv_17, mcv_16, mcv_15, mcv_14, mcv_13, mcv_12, mcv_11, mcv_10,                  \
          mcv_9,   mcv_8,  mcv_7,  mcv_6,  mcv_5,  mcv_4,  mcv_3,  mcv_2,  mcv_1,  mcv_0                   \
    )(_macro, t, ##__VA_ARGS__)


#define MATCHABLE(t, ...)                                                                                  \
    matchable_create_type_begin(t)                                                                         \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


// pt: property type
//  p: property
//  t: matchable type to amend
#define matchable_type_property_amendment(pt, p, t)                                                        \
    public:                                                                                                \
        /* vector property */                                                                              \
        std::vector<pt> const & as_##p##_vect() const                                                      \
            { return nullptr == t ? T::nil_##p##_vect() : t->as_##p##_vect(); }                            \
        std::vector<pt> & as_mutable_##p##_vect()                                                          \
            { return nullptr == t ? T::nil_##p##_vect() : t->as_mutable_##p##_vect(); }                    \
        void set_##p##_vect(std::vector<pt> const & v)                                                     \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
            {                                                                                              \
                T::nil_##p##_vect() = v;                                                                   \
                for (auto const & o : T::nil_##p##_vect_obs())                                             \
                    o.second();                                                                            \
            }                                                                                              \
            else { t->set_##p##_vect(v); }                                                                 \
        }                                                                                                  \
                                                                                                           \
        /* single property */                                                                              \
        pt const & as_##p() const { return nullptr == t ? T::nil_##p() : t->as_##p(); }                    \
        pt & as_mutable_##p() { return nullptr == t ? T::nil_##p() : t->as_mutable_##p(); }                \
        void set_##p(pt const & s)                                                                         \
            { if (nullptr == t) { T::nil_##p() = s; for (auto const & o : T::nil_##p##_obs()) o.second(); }\
              else t->set_##p(s); }                                                                        \
                                                                                                           \
        /* observers */                                                                                    \
        void add_##p##_vect_observer(std::string const & id, std::function<void ()> o)                     \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
                T::nil_##p##_vect_obs()[id] = o;                                                           \
            else                                                                                           \
                t->add_##p##_vect_observer(id, o);                                                         \
        }                                                                                                  \
        void add_##p##_observer(std::string const & id, std::function<void ()> o)                          \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
                T::nil_##p##_obs()[id] = o;                                                                \
            else                                                                                           \
                t->add_##p##_observer(id, o);                                                              \
        }                                                                                                  \
        void del_##p##_vect_observer(std::string const & id)                                               \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
            {                                                                                              \
                auto it = T::nil_##p##_vect_obs().find(id);                                                \
                if (it != T::nil_##p##_vect_obs().end())                                                   \
                    T::nil_##p##_vect_obs().erase(id);                                                     \
            }                                                                                              \
            else                                                                                           \
            {                                                                                              \
                t->del_##p##_vect_observer(id);                                                            \
            }                                                                                              \
        }                                                                                                  \
        void del_##p##_observer(std::string id)                                                            \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
            {                                                                                              \
                auto it = T::nil_##p##_obs().find(id);                                                     \
                if (it != T::nil_##p##_obs().end())                                                        \
                    T::nil_##p##_obs().erase(id);                                                          \
            }                                                                                              \
            else                                                                                           \
            {                                                                                              \
                t->del_##p##_observer(id);                                                                 \
            }                                                                                              \
        }                                                                                                  \
        void call_##p##_vect_observers()                                                                   \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
            {                                                                                              \
                for (auto const & o : T::nil_##p##_vect_obs())                                             \
                    o.second();                                                                            \
            }                                                                                              \
            else { t->call_##p##_vect_observers(); }                                                       \
        }                                                                                                  \
        void call_##p##_observers()                                                                        \
        {                                                                                                  \
            if (nullptr == t)                                                                              \
            {                                                                                              \
                for (auto const & o : T::nil_##p##_obs())                                                  \
                    o.second();                                                                            \
            }                                                                                              \
            else { t->call_##p##_observers(); }                                                            \
        }


#define matchable_declaration_property_amendment(pt, p, t)                                                 \
    public:                                                                                                \
        /* vector property */                                                                              \
        std::vector<pt> const & as_##p##_vect() const { return p##_vect_mb().at(Type(clone())); }          \
        std::vector<pt> & as_mutable_##p##_vect() { return p##_vect_mb().mut_at(Type(clone())); }          \
        void set_##p##_vect(std::vector<pt> const & v)                                                     \
            { auto c = Type(clone()); p##_vect_mb().set(c, v);                                             \
              for (auto o : p##_vect_obs_mb().at(c)) o.second(); }                                         \
                                                                                                           \
        /* single property */                                                                              \
        pt const & as_##p() const { return p##_mb().at(Type(clone())); }                                   \
        pt & as_mutable_##p() { return p##_mb().mut_at(Type(clone())); }                                   \
        void set_##p(pt const & s)                                                                         \
            { auto c = Type(clone()); p##_mb().set(c, s); for (auto o : p##_obs_mb().at(c)) o.second(); }  \
                                                                                                           \
        /* observers */                                                                                    \
        void add_##p##_vect_observer(std::string const & id, std::function<void ()> f)                     \
            { p##_vect_obs_mb().mut_at(Type(clone()))[id] = f; }                                           \
        void add_##p##_observer(std::string const & id, std::function<void ()> f)                          \
            { p##_obs_mb().mut_at(Type(clone()))[id] = f; }                                                \
        void del_##p##_vect_observer(std::string const & id)                                               \
        {                                                                                                  \
            auto c = Type(clone());                                                                        \
            auto it = p##_vect_obs_mb().mut_at(c).find(id);                                                \
            if (it != p##_vect_obs_mb().mut_at(c).end())                                                   \
                p##_vect_obs_mb().mut_at(Type(clone())).erase(it);                                         \
        }                                                                                                  \
        void del_##p##_observer(std::string const & id)                                                    \
        {                                                                                                  \
            auto c = Type(clone());                                                                        \
            auto it = p##_obs_mb().mut_at(c).find(id);                                                     \
            if (it != p##_obs_mb().mut_at(c).end())                                                        \
                p##_obs_mb().mut_at(c).erase(it);                                                          \
        }                                                                                                  \
        void call_##p##_vect_observers()                                                                   \
        {                                                                                                  \
            auto c = Type(clone());                                                                        \
            for (auto o : p##_vect_obs_mb().at(c)) o.second();                                             \
        }                                                                                                  \
        void call_##p##_observers()                                                                        \
        {                                                                                                  \
            auto c = Type(clone());                                                                        \
            for (auto o : p##_obs_mb().at(c)) o.second();                                                  \
        }                                                                                                  \
                                                                                                           \
    private:                                                                                               \
        /* vector property for non-nil variants */                                                         \
        static matchable::MatchBox<t::Type, std::vector<pt>> & p##_vect_mb()                               \
            { static matchable::MatchBox<t::Type, std::vector<pt>> mb; return mb; }                        \
        /* vector property for nil variant */                                                              \
        static std::vector<pt> & nil_##p##_vect() { static std::vector<pt> v; return v; }                  \
        /* observers of vector property for non-nil variants */                                            \
        static matchable::MatchBox<t::Type, std::map<std::string,                                          \
                                                     std::function<void ()>>> & p##_vect_obs_mb()          \
        {                                                                                                  \
            static matchable::MatchBox<t::Type, std::map<std::string, std::function<void ()>>> mb;         \
            return mb;                                                                                     \
        }                                                                                                  \
        /* observers of vector property for nil variant */                                                 \
        static std::map<std::string, std::function<void ()>> & nil_##p##_vect_obs()                        \
            { static std::map<std::string, std::function<void ()>> m; return m; }                          \
                                                                                                           \
        /* single property */                                                                              \
        static matchable::MatchBox<t::Type, pt> & p##_mb()                                                 \
            { static matchable::MatchBox<t::Type, pt> mb; return mb; }                                     \
        /* single property for nil */                                                                      \
        static pt & nil_##p() { static pt s; return s; }                                                   \
        /* observers of single property for non-nil variants */                                            \
        static matchable::MatchBox<t::Type, std::map<std::string, std::function<void ()>>> & p##_obs_mb()  \
        {                                                                                                  \
            static matchable::MatchBox<t::Type, std::map<std::string, std::function<void ()>>> mb;         \
            return mb;                                                                                     \
        }                                                                                                  \
        /* observers of single property for nil variant */                                                 \
        static std::map<std::string, std::function<void ()>> & nil_##p##_obs()                             \
            { static std::map<std::string, std::function<void ()>> v; return v; }


#define PROPERTYx1_MATCHABLE(pt0, p0, t, ...)                                                              \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx2_MATCHABLE(pt0, p0, pt1, p1, t, ...)                                                     \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx3_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, t, ...)                                            \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx4_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, t, ...)                                   \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx5_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, t, ...)                          \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx6_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, t, ...)                 \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx7_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, t, ...)        \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx8_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, t,    \
                             ...)                                                                          \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx9_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8,  \
                             p8, t, ...)                                                                   \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx10_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, t, ...)                                                         \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx11_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, t, ...)                                              \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx12_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, t, ...)                                   \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx13_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, t, ...)                        \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx14_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, t, ...)             \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx15_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, t, ...)  \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx16_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, t, ...)                                                                 \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx17_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, pt16, p16, t, ...)                                                      \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_type_property_amendment(pt16, p16, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declaration_property_amendment(pt16, p16, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx18_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, pt16, p16, pt17, p17, t, ...)                                           \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_type_property_amendment(pt16, p16, t)                                                        \
    matchable_type_property_amendment(pt17, p17, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declaration_property_amendment(pt16, p16, t)                                                 \
    matchable_declaration_property_amendment(pt17, p17, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx19_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, pt16, p16, pt17, p17, pt18, p18, t, ...)                                \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_type_property_amendment(pt16, p16, t)                                                        \
    matchable_type_property_amendment(pt17, p17, t)                                                        \
    matchable_type_property_amendment(pt18, p18, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declaration_property_amendment(pt16, p16, t)                                                 \
    matchable_declaration_property_amendment(pt17, p17, t)                                                 \
    matchable_declaration_property_amendment(pt18, p18, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx20_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, pt16, p16, pt17, p17, pt18, p18, pt19, p19, t, ...)                     \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_type_property_amendment(pt16, p16, t)                                                        \
    matchable_type_property_amendment(pt17, p17, t)                                                        \
    matchable_type_property_amendment(pt18, p18, t)                                                        \
    matchable_type_property_amendment(pt19, p19, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declaration_property_amendment(pt16, p16, t)                                                 \
    matchable_declaration_property_amendment(pt17, p17, t)                                                 \
    matchable_declaration_property_amendment(pt18, p18, t)                                                 \
    matchable_declaration_property_amendment(pt19, p19, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define PROPERTYx21_MATCHABLE(pt0, p0, pt1, p1, pt2, p2, pt3, p3, pt4, p4, pt5, p5, pt6, p6, pt7, p7, pt8, \
                              p8, pt9, p9, pt10, p10, pt11, p11, pt12, p12, pt13, p13, pt14, p14, pt15,    \
                              p15, pt16, p16, pt17, p17, pt18, p18, pt19, p19, pt20, p20, t, ...)          \
    matchable_create_type_begin(t)                                                                         \
    matchable_type_property_amendment(pt0, p0, t)                                                          \
    matchable_type_property_amendment(pt1, p1, t)                                                          \
    matchable_type_property_amendment(pt2, p2, t)                                                          \
    matchable_type_property_amendment(pt3, p3, t)                                                          \
    matchable_type_property_amendment(pt4, p4, t)                                                          \
    matchable_type_property_amendment(pt5, p5, t)                                                          \
    matchable_type_property_amendment(pt6, p6, t)                                                          \
    matchable_type_property_amendment(pt7, p7, t)                                                          \
    matchable_type_property_amendment(pt8, p8, t)                                                          \
    matchable_type_property_amendment(pt9, p9, t)                                                          \
    matchable_type_property_amendment(pt10, p10, t)                                                        \
    matchable_type_property_amendment(pt11, p11, t)                                                        \
    matchable_type_property_amendment(pt12, p12, t)                                                        \
    matchable_type_property_amendment(pt13, p13, t)                                                        \
    matchable_type_property_amendment(pt14, p14, t)                                                        \
    matchable_type_property_amendment(pt15, p15, t)                                                        \
    matchable_type_property_amendment(pt16, p16, t)                                                        \
    matchable_type_property_amendment(pt17, p17, t)                                                        \
    matchable_type_property_amendment(pt18, p18, t)                                                        \
    matchable_type_property_amendment(pt19, p19, t)                                                        \
    matchable_type_property_amendment(pt20, p20, t)                                                        \
    matchable_create_type_end(t)                                                                           \
    matchable_declare_begin(t)                                                                             \
    matchable_declaration_property_amendment(pt0, p0, t)                                                   \
    matchable_declaration_property_amendment(pt1, p1, t)                                                   \
    matchable_declaration_property_amendment(pt2, p2, t)                                                   \
    matchable_declaration_property_amendment(pt3, p3, t)                                                   \
    matchable_declaration_property_amendment(pt4, p4, t)                                                   \
    matchable_declaration_property_amendment(pt5, p5, t)                                                   \
    matchable_declaration_property_amendment(pt6, p6, t)                                                   \
    matchable_declaration_property_amendment(pt7, p7, t)                                                   \
    matchable_declaration_property_amendment(pt8, p8, t)                                                   \
    matchable_declaration_property_amendment(pt9, p9, t)                                                   \
    matchable_declaration_property_amendment(pt10, p10, t)                                                 \
    matchable_declaration_property_amendment(pt11, p11, t)                                                 \
    matchable_declaration_property_amendment(pt12, p12, t)                                                 \
    matchable_declaration_property_amendment(pt13, p13, t)                                                 \
    matchable_declaration_property_amendment(pt14, p14, t)                                                 \
    matchable_declaration_property_amendment(pt15, p15, t)                                                 \
    matchable_declaration_property_amendment(pt16, p16, t)                                                 \
    matchable_declaration_property_amendment(pt17, p17, t)                                                 \
    matchable_declaration_property_amendment(pt18, p18, t)                                                 \
    matchable_declaration_property_amendment(pt19, p19, t)                                                 \
    matchable_declaration_property_amendment(pt20, p20, t)                                                 \
    matchable_declare_end(t)                                                                               \
    matchable_define(t)                                                                                    \
    mcv(matchable_create_variant, t, ##__VA_ARGS__)


#define MATCHABLE_VARIANT_PROPERTY_VALUE(t, v, p, pv)                                                      \
    [[maybe_unused]] static bool const MATCHABLE_VARIANT_PROPERTY_VALUE_init_##t##_##v##_##p =             \
        [](){t::v::grab().set_##p(pv); return true;}();

#define MATCHABLE_VARIANT_PROPERTY_VALUES(t, v, p, ...)                                                    \
    [[maybe_unused]] static bool const MATCHABLE_VARIANT_PROPERTY_VALUES_init_##t##_##v##_##p =            \
        [](decltype(t::v::grab().as_##p##_vect()) sv)                                                      \
            { t::v::grab().set_##p##_##vect(sv); return true; }({__VA_ARGS__});


#define MATCHABLE_NIL_PROPERTY_VALUE(t, p, pv)                                                             \
    [[maybe_unused]] static bool const SET_PROPERTY_init_##t##_nil_##p =                                   \
        [](){t::nil.set_##p(pv); return true;}();

#define MATCHABLE_NIL_PROPERTY_VALUES(t, p, ...)                                                           \
    [[maybe_unused]] static bool const SET_PROPERTY_VECT_init_##t##_nil_##p =                              \
        [](decltype(t::nil.as_##p##_vect()) sv)                                                            \
            { t::nil.set_##p##_##vect(sv); return true; }({__VA_ARGS__});


// Remove variants for the current scope (when the scope exits the removed variants are restored).
#define UNMATCHABLE(t, ...)                                                                                \
    matchable::Unmatchable<t::Type> unm_##t{{mcv(matchable_concat_variant, t, ##__VA_ARGS__)}}


#define NAMESPACEx1_UNMATCHABLE(n0, t, ...)                                                                \
    matchable::Unmatchable<n0::t::Type> unm_##n0##_##t                                                     \
        {{mcv(matchable_concat_variant, n0::t, ##__VA_ARGS__)}}


#define NAMESPACEx2_UNMATCHABLE(n0, n1, t, ...)                                                            \
    matchable::Unmatchable<n0::n1::t::Type> unm_##n0##_##n1##_##t                                          \
        {{mcv(matchable_concat_variant, n0::n1::t, ##__VA_ARGS__)}}


#define NAMESPACEx3_UNMATCHABLE(n0, n1, n2, t, ...)                                                        \
    matchable::Unmatchable<n0::n1::n2::t::Type> unm_##n0##_##n1##_##n2##_##t                               \
        {{mcv(matchable_concat_variant, n0::n1::n2::t, ##__VA_ARGS__)}}


#define NAMESPACEx4_UNMATCHABLE(n0, n1, n2, n3, t, ...)                                                    \
    matchable::Unmatchable<n0::n1::n2::n3::t::Type> unm_##n0##_##n1##_##n2##_##n3##_##t                    \
        {{mcv(matchable_concat_variant, n0::n1::n2::n3::t, ##__VA_ARGS__)}}


#define NAMESPACEx5_UNMATCHABLE(n0, n1, n2, n3, n4, t, ...)                                                \
    matchable::Unmatchable<n0::n1::n2::n3::n4::t::Type>                                                    \
        unm_##n0##_##n1##_##n2##_##n3##_##n4##_##t                                                         \
            {{mcv(matchable_concat_variant, n0::n1::n2::n3::n4::t, ##__VA_ARGS__)}}


// Test if a given matchable instance is contained within a given list of variants
#define MATCHABLE_INSTANCE_IN(t, _i, ...)                                                                  \
    [](t::Type const & t, std::vector<t::Type> const & v)                                                  \
        { return std::find(v.begin(), v.end(), t) != v.end(); }                                            \
            (_i, {mcv(matchable_concat_variant, t, ##__VA_ARGS__)})


// Add variants to existing matchable
#define GROW_MATCHABLE(t, ...) mcv(matchable_create_variant, t, ##__VA_ARGS__)


// FLOW CONTROL MACROS
#define MATCH_WITH_FLOW_CONTROL { matchable::FlowControl fc =
#define EVAL_FLOW_CONTROL if (fc.brk_requested()) break; if (fc.cont_requested()) continue; }
#define EVAL_BREAK_ONLY if (fc.brk_requested()) break; }


namespace matchable
{
    namespace escapable
    {
        inline std::string unescape_all(std::string const & input);
        inline std::string escape_all(std::string const & input);

        inline std::vector<std::pair<std::string, std::string>> const & code_symbol_pairs()
        {
            static std::vector<std::pair<std::string, std::string>> const csp =
                [](){
                    std::vector<std::pair<std::string, std::string>> csp_init;
                    csp_init.push_back(std::make_pair("_spc_", " "));
                    csp_init.push_back(std::make_pair("_bng_", "!"));
                    csp_init.push_back(std::make_pair("_quot_", "\""));
                    csp_init.push_back(std::make_pair("_hsh_", "#"));
                    csp_init.push_back(std::make_pair("_dol_", "$"));
                    csp_init.push_back(std::make_pair("_pct_", "%"));
                    csp_init.push_back(std::make_pair("_und_", "&"));
                    csp_init.push_back(std::make_pair("_sqt_", "'"));
                    csp_init.push_back(std::make_pair("_parl_", "("));
                    csp_init.push_back(std::make_pair("_parr_", ")"));
                    csp_init.push_back(std::make_pair("_ast_", "*"));
                    csp_init.push_back(std::make_pair("_plus_", "+"));
                    csp_init.push_back(std::make_pair("_cma_", ","));
                    csp_init.push_back(std::make_pair("_mns_", "-"));
                    csp_init.push_back(std::make_pair("_dot_", "."));
                    csp_init.push_back(std::make_pair("_slsh_", "/"));
                    csp_init.push_back(std::make_pair("_cln_", ":"));
                    csp_init.push_back(std::make_pair("_scln_", ";"));
                    csp_init.push_back(std::make_pair("_lt_", "<"));
                    csp_init.push_back(std::make_pair("_eq_", "="));
                    csp_init.push_back(std::make_pair("_gt_", ">"));
                    csp_init.push_back(std::make_pair("_qstn_", "?"));
                    csp_init.push_back(std::make_pair("_atsym_", "@"));
                    csp_init.push_back(std::make_pair("_sbl_", "["));
                    csp_init.push_back(std::make_pair("_bslsh_", "\\"));
                    csp_init.push_back(std::make_pair("_sbr_", "]"));
                    csp_init.push_back(std::make_pair("_dach_", "^"));
                    csp_init.push_back(std::make_pair("_bqt_", "`"));
                    csp_init.push_back(std::make_pair("_cbl_", "{"));
                    csp_init.push_back(std::make_pair("_pip_", "|"));
                    csp_init.push_back(std::make_pair("_cbr_", "}"));
                    csp_init.push_back(std::make_pair("_tld_", "~"));
                    return csp_init;
                }();
            return csp;
        }

        inline std::string unescape(std::string const & esc)
        {
            if (esc.size() == 0)
                return "";

            for (size_t i = 0; i < code_symbol_pairs().size(); ++i)
                if (code_symbol_pairs()[i].first == esc)
                    return code_symbol_pairs()[i].second;

            return esc;
        }


        inline std::string unescape_all(std::string const & input)
        {
            std::string unescaped{input};
            if (unescaped.substr(0, 4) == "esc_")
                unescaped.erase(0, 4);

            size_t index = 0;
            for (size_t i = 0; i < code_symbol_pairs().size(); ++i)
            {
                std::string const & code = code_symbol_pairs()[i].first;
                std::string const & symbol = code_symbol_pairs()[i].second;
                index = 0;
                while (true)
                {
                    index = unescaped.find(code, index);
                    if (index == std::string::npos)
                        break;

                    auto replacement = symbol;
                    assert(replacement != "");
                    unescaped.erase(index, code.size());
                    unescaped.insert(index, replacement);
                    index += replacement.size();
                }
            }

            return unescaped;
        }


        inline std::string escape(std::string const & str)
        {
            if (str.size() != 1)
                return str;

            static int const offset{32};
            static std::array<std::string, 128 - offset> const escapables =
                [&](){
                    std::array<std::string, 128 - offset> e;

                    for (int i = 0; i < (int) e.size(); ++i)
                        e[i] = std::string(1, (char) (i + offset));

                    e[(int) ' ' - offset] = "_spc_";
                    e[(int) '!' - offset] = "_bng_";
                    e[(int) '"' - offset] = "_quot_";
                    e[(int) '#' - offset] = "_hsh_";
                    e[(int) '$' - offset] = "_dol_";
                    e[(int) '%' - offset] = "_pct_";
                    e[(int) '&' - offset] = "_und_";
                    e[(int) '\'' - offset] = "_sqt_";
                    e[(int) '(' - offset] = "_parl_";
                    e[(int) ')' - offset] = "_parr_";
                    e[(int) '*' - offset] = "_ast_";
                    e[(int) '+' - offset] = "_plus_";
                    e[(int) ',' - offset] = "_cma_";
                    e[(int) '-' - offset] = "_mns_";
                    e[(int) '.' - offset] = "_dot_";
                    e[(int) '/' - offset] = "_slsh_";
                    e[(int) ':' - offset] = "_cln_";
                    e[(int) ';' - offset] = "_scln_";
                    e[(int) '<' - offset] = "_lt_";
                    e[(int) '=' - offset] = "_eq_";
                    e[(int) '>' - offset] = "_gt_";
                    e[(int) '?' - offset] = "_qstn_";
                    e[(int) '@' - offset] = "_atsym_";
                    e[(int) '[' - offset] = "_sbl_";
                    e[(int) '\\' - offset] = "_bslsh_";
                    e[(int) ']' - offset] = "_sbr_";
                    e[(int) '^' - offset] = "_dach_";
                    e[(int) '`' - offset] = "_bqt_";
                    e[(int) '{' - offset] = "_cbl_";
                    e[(int) '|' - offset] = "_pip_";
                    e[(int) '}' - offset] = "_cbr_";
                    e[(int) '~' - offset] = "_tld_";
                    return e;
                }();

            int const & ch = str[0];
            if (ch >= offset && ch < 127)
                return escapables[ch - offset];

            return str;
        }


        inline std::string escape_all(std::string const & input)
        {
            std::string escaped;
            std::string escapable;
            std::string char_as_str;
            for (size_t i = 0; i < input.size(); ++i)
            {
                char_as_str = std::string(1, input[i]);
                escaped += escapable::escape(char_as_str);
            }
            return escaped;
        }
    }
}
