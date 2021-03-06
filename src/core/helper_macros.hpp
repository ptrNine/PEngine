#pragma once

#define SINGLETON_IMPL(CLASS) \
public: \
static CLASS& instance() { \
    static CLASS inst; \
    return inst; \
} \
CLASS(const CLASS&) = delete; \
CLASS& operator=(const CLASS&) = delete; \
CLASS(CLASS&&) noexcept = delete; \
CLASS& operator=(CLASS&&) noexcept = delete

#define DECLARE_GET(FIELD) \
[[nodiscard]] \
auto& FIELD() const { return _##FIELD; }

#define DECLARE_VAL_GET(FIELD) \
[[nodiscard]] \
auto FIELD() const { return _##FIELD; }

#define DECLARE_NON_CONST_GET(FIELD) \
[[nodiscard]] \
auto& FIELD() { return _##FIELD; }

#define DECLARE_SET(FIELD) \
void set_##FIELD(const decltype(_##FIELD)& value) { _##FIELD = value; }

#define DECLARE_VAL_SET(FIELD) \
void set_##FIELD(decltype(_##FIELD) value) { _##FIELD = value; }

#define DECLARE_GET_SET(FIELD) \
DECLARE_GET(FIELD) \
DECLARE_SET(FIELD)

#define DECLARE_VAL_GET_SET(FIELD) \
DECLARE_VAL_GET(FIELD) \
DECLARE_VAL_SET(FIELD)

#define DECLARE_SMART_POINTERS_T(CLASS) \
using SharedPtr = std::shared_ptr<CLASS>; \
using UniquePtr = std::unique_ptr<CLASS>; \
using WeakPtr   = std::weak_ptr<CLASS>

#define DECLARE_SELF_FABRICS(CLASS)  \
template<typename... Args>          \
static auto createUnique(Args&&... args) { \
    return std::make_unique<CLASS>(args...); \
} \
template<typename... Args>          \
static auto createShared(Args&&... args) { \
    return std::make_shared<CLASS>(args...); \
}                                          \
DECLARE_SMART_POINTERS_T(CLASS)

#define TO_TUPLE_IMPL(...) \
auto to_tuple() const { \
    return std::make_tuple(__VA_ARGS__); \
}

#define TUPLE_SERIALIZE(...) \
auto to_tuple() const { \
    return std::make_tuple(__VA_ARGS__); \
} \
void set_from_tuple(decltype(std::make_tuple(__VA_ARGS__)) tuple_data) { \
    std::tie(__VA_ARGS__) = std::move(tuple_data); \
}

#define DEBUG_VAL_LMAO(value) \
std::cout << #value ": " << (value) << std::endl

/**
 * Equivalent to ([](const auto& ARGUMENT_NAME) { return ...; })
 */
#define xlambda(ARGUMENT_NAME, ...) \
    ([](const auto& ARGUMENT_NAME) { return __VA_ARGS__; })

/**
 * Equivalent to ([](const auto& ARGUMENT_NAME1, const auto& ARGUMENT_NAME2) { return ...; })
 */
#define xlambda2(ARGUMENT_NAME1, ARGUMENT_NAME2, ...) \
    ([](const auto& ARGUMENT_NAME1, const auto& ARGUMENT_NAME2) { return __VA_ARGS__; })

#include "macro_argname.hpp"
