// vagabond
// Copyright (C) 2022 Helen Ginn
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Please email: vagabond @ hginn.co.uk for more details.

#pragma once

#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

template <typename T>
concept HasToString = requires(T obj) {
  { obj.toString() } -> std::convertible_to<std::string>;
};

template <typename T> struct Ok {
  T value;
};
template <typename T> Ok(T) -> Ok<T>;

template <typename E> struct Err {
  E value;
};

template <> struct Ok<void> {};
Ok() -> Ok<void>;

template <typename T, typename E> class [[nodiscard]] Result {
  std::variant<T, E> data_;

public:
  template <typename U = T>
    requires std::convertible_to<U, T>
  constexpr Result(Ok<U> ok)
      : data_(std::in_place_index<0>, std::move(ok.value)) {}

  template <typename U = E>
    requires std::convertible_to<U, E>
  constexpr Result(Err<U> err)
      : data_(std::in_place_index<1>, std::move(err.value)) {}

  constexpr bool operator==(const Result &) const = default;

  [[nodiscard]] constexpr bool is_ok() const { return data_.index() == 0; }
  [[nodiscard]] constexpr bool is_err() const { return data_.index() == 1; }

  constexpr T &unwrap() & {
    if (is_err())
      throw std::runtime_error("Called unwrap on an Err value!");
    return std::get<0>(data_);
  }

  constexpr T unwrap() && {
    if (is_err())
      throw std::runtime_error("Called unwrap on an Err value!");
    return std::get<0>(std::move(data_));
  }

  constexpr const T &unwrap() const & {
    if (is_err())
      throw std::runtime_error("Called unwrap on an Err value!");
    return std::get<0>(data_);
  }

  constexpr E &unwrap_err() & {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(data_);
  }

  constexpr E unwrap_err() && {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(std::move(data_));
  }

  constexpr const E &unwrap_err() const & {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(data_);
  }

  constexpr T unwrap_or(T fallback) && {
    if (is_ok()) {
      return std::get<0>(std::move(data_));
    }
    return fallback;
  }

  constexpr T unwrap_or_default() && {
    if (is_ok()) {
      return std::get<0>(std::move(data_));
    }
    return T{};
  }

  template <typename F>
    requires std::convertible_to<std::invoke_result_t<F, E>, T>
  constexpr T unwrap_or_else(F &&func) && {
    if (is_ok()) {
      return std::get<0>(std::move(data_));
    }
    return func(std::get<1>(std::move(data_)));
  }

  T expect(const std::string &msg) &&
    requires HasToString<E>
  {
    if (is_err()) {
      throw std::runtime_error(msg + ": " + error().toString());
    }
    return std::get<0>(std::move(data_));
  }

  T expect(const std::string &msg) &&
    requires(!HasToString<E>)
  {
    if (is_err()) {
      throw std::runtime_error(msg);
    }
    return std::get<0>(std::move(data_));
  }

  E expect_err(const std::string &msg) &&
    requires HasToString<T>
  {
    if (is_ok()) {
      throw std::runtime_error(msg + ": " + std::get<0>(data_).toString());
    }
    return std::get<1>(std::move(data_));
  }

  E expect_err(const std::string &msg) &&
    requires(!HasToString<T>)
  {
    if (is_ok()) {
      throw std::runtime_error(msg);
    }
    return std::get<1>(std::move(data_));
  }

  constexpr const E &error() const { return std::get<1>(data_); }

  template <typename F> constexpr auto map(F &&func) && {
    using U = std::invoke_result_t<F, T>;
    if constexpr (std::is_void_v<U>) {
      if (is_ok()) {
        func(std::get<0>(std::move(data_)));
        return Result<void, E>(Ok<void>{});
      }
      return Result<void, E>(Err(std::get<1>(std::move(data_))));
    } else {
      if (is_ok()) {
        return Result<U, E>(Ok(func(std::get<0>(std::move(data_)))));
      }
      return Result<U, E>(Err(std::get<1>(std::move(data_))));
    }
  }

  template <typename F> constexpr auto map_err(F &&func) && {
    using U = std::invoke_result_t<F, E>;
    if (is_err()) {
      return Result<T, U>(Err(func(std::get<1>(std::move(data_)))));
    }
    return Result<T, U>(Ok(std::get<0>(std::move(data_))));
  }

  template <typename F> constexpr auto and_then(F &&func) && {
    using CallResult = std::invoke_result_t<F, T>;
    if (is_ok()) {
      return func(std::get<0>(std::move(data_)));
    }
    return CallResult(Err(std::get<1>(std::move(data_))));
  }

  template <typename F> constexpr auto or_else(F &&func) && {
    using CallResult = std::invoke_result_t<F, E>;
    if (is_err()) {
      return func(std::get<1>(std::move(data_)));
    }
    return CallResult(Ok(std::get<0>(std::move(data_))));
  }

  [[nodiscard]] constexpr std::optional<T> ok() const & {
    if (is_ok()) {
      return std::get<0>(data_);
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<T> ok() && {
    if (is_ok()) {
      return std::get<0>(std::move(data_));
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<E> err() const & {
    if (is_err()) {
      return std::get<1>(data_);
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<E> err() && {
    if (is_err()) {
      return std::get<1>(std::move(data_));
    }
    return std::nullopt;
  }
};

template <typename E> class [[nodiscard]] Result<void, E> {
  std::variant<std::monostate, E> data_;

public:
  constexpr Result(Ok<void>) : data_(std::in_place_index<0>) {}

  template <typename U = E>
    requires std::convertible_to<U, E>
  constexpr Result(Err<U> err)
      : data_(std::in_place_index<1>, std::move(err.value)) {}

  constexpr bool operator==(const Result &) const = default;

  [[nodiscard]] constexpr bool is_ok() const { return data_.index() == 0; }
  [[nodiscard]] constexpr bool is_err() const { return data_.index() == 1; }

  constexpr void unwrap() const {
    if (is_err())
      throw std::runtime_error("Called unwrap on an Err value!");
  }

  constexpr E &unwrap_err() & {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(data_);
  }

  constexpr E unwrap_err() && {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(std::move(data_));
  }

  constexpr const E &unwrap_err() const & {
    if (is_ok())
      throw std::runtime_error("Called unwrap_err on an Ok value!");
    return std::get<1>(data_);
  }

  void expect(const std::string &msg) &&
    requires HasToString<E>
  {
    if (is_err()) {
      throw std::runtime_error(msg + ": " + error().toString());
    }
  }

  void expect(const std::string &msg) &&
    requires(!HasToString<E>)
  {
    if (is_err()) {
      throw std::runtime_error(msg);
    }
  }

  E expect_err(const std::string &msg) && {
    if (is_ok()) {
      throw std::runtime_error(msg);
    }
    return std::get<1>(std::move(data_));
  }

  constexpr const E &error() const { return std::get<1>(data_); }

  template <typename F> constexpr auto map(F &&func) && {
    using U = std::invoke_result_t<F>;

    if (is_ok()) {
      return Result<U, E>(Ok(func()));
    }

    return Result<U, E>(Err(std::get<1>(std::move(data_))));
  }

  template <typename F> constexpr auto map_err(F &&func) && {
    using U = std::invoke_result_t<F, E>;
    if (is_err()) {
      return Result<void, U>(Err(func(std::get<1>(std::move(data_)))));
    }
    return Result<void, U>(Ok<void>{});
  }

  template <typename F> constexpr auto and_then(F &&func) && {
    using CallResult = std::invoke_result_t<F>;
    if (is_ok()) {
      return func();
    }
    return CallResult(Err(std::get<1>(std::move(data_))));
  }

  template <typename F> constexpr auto or_else(F &&func) && {
    using CallResult = std::invoke_result_t<F, E>;
    if (is_err()) {
      return func(std::get<1>(std::move(data_)));
    }
    return CallResult(Ok<void>{});
  }

  [[nodiscard]] constexpr std::optional<E> err() const & {
    if (is_err()) {
      return std::get<1>(data_);
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<E> err() && {
    if (is_err()) {
      return std::get<1>(std::move(data_));
    }
    return std::nullopt;
  }
};
