#pragma once

namespace sy
{
namespace chrono = std::chrono;
namespace fs     = std::filesystem;

template <typename T>
using Ref = std::reference_wrapper<T>;
template <typename T>
using CRef = Ref<const T>;
template <typename T>
using RefVec = std::vector<Ref<T>>;
template <typename T>
using CRefVec = std::vector<CRef<T>>;
template <typename T>
using RefSpan = std::span<Ref<T>>;
template <typename T>
using CRefSpan = std::span<CRef<T>>;
template <typename T, size_t C>
using RefArray = std::array<Ref<T>, C>;
template <typename T, size_t C>
using CRefArray = std::array<CRef<T>, C>;
template <typename T>
using RefOptional = std::optional<Ref<T>>;
template <typename T>
using CRefOptional = std::optional<CRef<T>>;

template <typename T>
T& Unwrap(const RefOptional<T>& optional)
{
    return optional.value().get();
}

template <typename T>
const T& Unwrap(const CRefOptional<T>& optional)
{
    return optional.value().get();
}

using RWLock       = std::unique_lock<std::shared_mutex>;
using ReadOnlyLock = std::shared_lock<std::shared_mutex>;
} // namespace sy

#include <Core/NonCopyable.h>
#include <Core/NamedType.h>
#include <Core/Range.h>
#include <Core/Serializable.h>